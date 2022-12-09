#include <SFML/Graphics.hpp>
#include "SimplexNoise.hpp"
#include <vector>
#include <chrono>
#include <iostream>


int width = 1280, height = 720;
bool focus = true;
bool wk = 0, ak = 0, sk = 0, dk = 0, spacek = 0;
long now, last = 0, dt = 0, ticks = 0, fps = 1000 / 60;
int tileSize = 64, mapWidth = 50, mapHeight = 50, worldHeight = 5;
int renderDist = 64, jumpCD = 0;
double posx = 1000, posy = 1000, posh = worldHeight - 1.0, velx = 0, vely = 0, playWidth = 0.8, step = 0.1, maxSpeed = 2.0;

SimplexNoise noise;

sf::RenderWindow window(sf::VideoMode(width, height), "Tiles");
sf::RectangleShape rect;

struct Tile
{
    int id = 0;
    Tile(int idp)
    {
        id = idp;
    }
};
std::vector<std::vector<std::vector<Tile>>>tiles;


bool isTileSolid(int, int, int);
bool isTileMovable(int, int, int);
bool isTileLiquid(int, int, int);
int getTile(int, int, int);
void setTile(int, int, int, int);
void generate();
void move();
void render();
void update();
void getKeys(sf::Event, bool);
void init();


int main()
{
#pragma warning (push)
#pragma warning (disable:4244)
    init();

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            switch (event.type)
            {
            case sf::Event::Closed:
                window.close();
                break;
            case sf::Event::GainedFocus:
                focus = true;
                break;
            case sf::Event::LostFocus:
                focus = false;
                wk = 0; ak = 0; sk = 0; dk = 0;
                break;
            case sf::Event::KeyPressed:
                if (focus)
                    getKeys(event, 1);
                break;
            case sf::Event::KeyReleased:
                if (focus)
                    getKeys(event, 0);
                break;
            case sf::Event::Resized:
                sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
                window.setView(sf::View(visibleArea));
                width = event.size.width; height = event.size.height;
                break;
            }
        }
        now = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
        if (last == 0) last = now;
        dt += now - last;
        last = now;
        while (dt >= fps)
        {
            dt -= fps;
            update();
            move();
        }
        render();
    }

    return 0;
#pragma warning (pop)
}


void init()
{
    window.setFramerateLimit(60);

    std::vector<std::vector<Tile>>vvt;
    std::vector<Tile>vt;
    Tile t(0);
    for (int h = 0; h < worldHeight; h++)
    {
        for (int x = 0; x < mapWidth; x++)
        {
            for (int y = 0; y < mapHeight; y++)
                vt.push_back(t);
            vvt.push_back(vt);
        }
        tiles.push_back(vvt);
    }

    generate();
}


bool isTileSolid(int h, int x, int y)
{
    switch (getTile(h, x, y))
    {
    case 2:
        return 1;
    default:
        return 0;
    }
}


bool isTileMovable(int h, int x, int y)
{
    switch (getTile(h, x, y))
    {
    case 0:
    case 1:
        return 1;
    default:
        return 0;
    }
}


bool isTileLiquid(int h, int x, int y)
{
    switch (getTile(h, x, y))
    {
    case 1:
        return 1;
    default:
        return 0;
    }
}


int getTile(int h, int x, int y)
{
    if (h >= 0 && h < worldHeight && x >= 0 && x < mapWidth && y >= 0 && y < mapHeight)
        return tiles[h][x][y].id;
    if (h >= worldHeight)
        return 0;
    return -1;
}


void setTile(int h, int x, int y, int id)
{
    if (h >= 0 && h < worldHeight && x >= 0 && x < mapWidth && y >= 0 && y < mapHeight)
        tiles[h][x][y] = id;
}


void generate()
{
#pragma warning (push)
#pragma warning (disable:4244)
    for (int x = 0; x < mapWidth; x++)
        for (int y = 0; y < mapHeight; y++)
        {
            for (int h = 1; h < worldHeight; h++)
                setTile(h, x, y, 0);
            setTile(0, x, y, 1);
            int hgt = std::floor(noise.unsignedFBM(x * 0.05, y * 0.05, 3, 0.5, 0.1) * worldHeight);
            for (int h = 0; h < hgt; h++)
                setTile(h, x, y, 2);
        }
#pragma warning (pop)
}


void update()
{
}


void move()
{
#pragma warning (push)
#pragma warning (disable:4244)
    velx += (double(dk) - double(ak)) * 0.1;
    vely += (double(sk) - double(wk)) * 0.1;

    if (velx >= maxSpeed) velx = maxSpeed;
    if (velx <= -maxSpeed) velx = -maxSpeed;
    if (vely >= maxSpeed) vely = maxSpeed;
    if (vely <= -maxSpeed) vely = -maxSpeed;

    if (isTileMovable(posh - 1, posx / tileSize, posy / tileSize) && posh > 0)
        posh--;
    if (isTileSolid(posh, posx / tileSize, posy / tileSize))
        posh++;
    if (isTileLiquid(posh, posx / tileSize, posy / tileSize))
    {
        velx /= 1.3;
        vely /= 1.3;
    }

    double oldPosx = posx, oldPosy = posy;


    // RIGHT
    if (velx > 0)
        for (int i = 0; i < std::abs(velx) * 100; i++)
        {
            if (isTileMovable(posh + 1, (posx + playWidth / 2 * tileSize + step) / tileSize, (posy + playWidth / 2 * tileSize) / tileSize) &&
                isTileMovable(posh + 1, (posx + playWidth / 2 * tileSize + step) / tileSize, (posy - playWidth / 2 * tileSize) / tileSize))
                if ((isTileMovable(posh, (posx + playWidth / 2 * tileSize + step) / tileSize, (posy + playWidth / 2 * tileSize) / tileSize) &&
                     isTileMovable(posh, (posx + playWidth / 2 * tileSize + step) / tileSize, (posy - playWidth / 2 * tileSize) / tileSize)) || jumpCD <= 0)
                {
                    posx += step;
                    if (isTileSolid(posh, posx / tileSize, posy / tileSize))
                    {
                        posh++;
                        //jumpCD = 50;
                    }
                }
            else
                velx = 0;
        }


    // LEFT
    if (velx < 0)
        for (int i = 0; i < std::abs(velx) * 100; i++)
        {
            if (isTileMovable(posh + 1, (posx - playWidth / 2 * tileSize - step) / tileSize, (posy + playWidth / 2 * tileSize) / tileSize) &&
                isTileMovable(posh + 1, (posx - playWidth / 2 * tileSize - step) / tileSize, (posy - playWidth / 2 * tileSize) / tileSize))
                if ((isTileMovable(posh, (posx - playWidth / 2 * tileSize - step) / tileSize, (posy + playWidth / 2 * tileSize) / tileSize) &&
                     isTileMovable(posh, (posx - playWidth / 2 * tileSize - step) / tileSize, (posy - playWidth / 2 * tileSize) / tileSize)) || jumpCD <= 0)
                {
                    posx -= step;
                    if (isTileSolid(posh, (posx - playWidth / 2 * tileSize - step) / tileSize, (posy + playWidth / 2 * tileSize) / tileSize) &&
                        isTileSolid(posh, (posx - playWidth / 2 * tileSize - step) / tileSize, (posy - playWidth / 2 * tileSize) / tileSize))
                    {
                        posh++;
                        //jumpCD = 50;
                    }
                }
            else
                velx = 0;
        }

    // BOTTOM
    if (vely > 0)
        for (int i = 0; i < std::abs(vely) * 100; i++)
        {
            if (isTileMovable(posh + 1, (posx - playWidth / 2 * tileSize) / tileSize, (posy + playWidth / 2 * tileSize + step) / tileSize) && 
                isTileMovable(posh + 1, (posx + playWidth / 2 * tileSize) / tileSize, (posy + playWidth / 2 * tileSize + step) / tileSize))
                if ((isTileMovable(posh, (posx - playWidth / 2 * tileSize) / tileSize, (posy + playWidth / 2 * tileSize + step) / tileSize) &&
                     isTileMovable(posh, (posx + playWidth / 2 * tileSize) / tileSize, (posy + playWidth / 2 * tileSize + step) / tileSize)) || jumpCD <= 0)
                {
                    posy += step;
                    if (isTileSolid(posh, posx / tileSize, posy / tileSize))
                    {
                        posh++;
                        //jumpCD = 50;
                    }
                }
            else
                vely = 0;
        }

    // TOP
    if (vely < 0)
        for (int i = 0; i < std::abs(vely) * 100; i++)
        {
            if (isTileMovable(posh + 1, (posx - playWidth / 2 * tileSize) / tileSize, (posy - playWidth / 2 * tileSize - step) / tileSize) && 
                isTileMovable(posh + 1, (posx + playWidth / 2 * tileSize) / tileSize, (posy - playWidth / 2 * tileSize - step) / tileSize))
                if ((isTileMovable(posh, (posx - playWidth / 2 * tileSize) / tileSize, (posy - playWidth / 2 * tileSize - step) / tileSize) &&
                     isTileMovable(posh, (posx + playWidth / 2 * tileSize) / tileSize, (posy - playWidth / 2 * tileSize - step) / tileSize)) || jumpCD <= 0)
                {
                    posy -= step;
                    if (isTileSolid(posh, posx / tileSize, posy / tileSize))
                    {
                        posh++;
                        //jumpCD = 50;
                    }
                }
            else
                vely = 0;
        }

    if (oldPosx == posx && oldPosy == posy)
    {
        jumpCD--;
        if (jumpCD < 0) jumpCD = 0;
        std::cout << "jumpCD-- : " << jumpCD << "\n";
    }
    else
    {
        jumpCD = 50;
    }


    velx /= 1.2;
    vely /= 1.2;
    if (std::abs(velx) < 0.01) velx = 0;
    if (std::abs(vely) < 0.01) vely = 0;

    if (posx + playWidth / 2 * tileSize >= (double)mapWidth * tileSize) posx = (double)mapWidth * tileSize - playWidth / 2 * tileSize - 1;
    if (posy + playWidth / 2 * tileSize >= (double)mapHeight * tileSize) posy = (double)mapHeight * tileSize - playWidth / 2 * tileSize - 1;
    if (posx - playWidth / 2 * tileSize < 0) posx = playWidth / 2 * tileSize;
    if (posy - playWidth / 2 * tileSize < 0) posy = playWidth / 2 * tileSize;
#pragma warning (pop)
}


void render()
{
#pragma warning (push)
#pragma warning (disable:4244)
    int hgt, r, g, b, lvl = 128 / (worldHeight - 1);
    window.clear();

    for (int x = -renderDist + posx / tileSize; x <= renderDist + posx / tileSize; x++)
        for (int y = -renderDist + posy / tileSize; y <= renderDist + posy / tileSize; y++)
        {
            hgt = 0;
            for (int h = 0; h < worldHeight; h++)
                if (getTile(h, x, y) > 0)
                    hgt = h;
            switch (getTile(hgt, x, y))
            {
            case 0:
                r = 0; g = 0; b = 0;
                break;
            case 1:
                r = 0; g = 0; b = 128;
                break;
            case 2:
                r = 0; g = 128; b = 0;
                break;
            case 3:
                r = 128; g = 0; b = 0;
                break;
            default:
                r = 128; g = 0; b = 128;
            }
            rect.setPosition(width / 2 + x * tileSize - posx, height / 2 + y * tileSize - posy);
            rect.setSize(sf::Vector2f(tileSize, tileSize));
            rect.setFillColor(sf::Color(r + lvl * hgt, g + lvl * hgt, b + lvl * hgt));
            window.draw(rect);
        }

    rect.setSize(sf::Vector2f(playWidth * tileSize, playWidth * tileSize));
    rect.setPosition(width / 2.0 - playWidth * tileSize / 2, height / 2.0 - playWidth * tileSize / 2);
    rect.setFillColor(sf::Color(96 + lvl * posh, 96 + lvl * posh, 69 + lvl * posh));
    window.draw(rect);

    window.display();
#pragma warning (pop)
}


void getKeys(sf::Event event, bool down)
{
    switch (event.key.code)
    {
    case 22:
        wk = down;
        break;
    case 00:
        ak = down;
        break;
    case 18:
        sk = down;
        break;
    case 03:
        dk = down;
        break;
    case 57:
        spacek = down;
    }
}