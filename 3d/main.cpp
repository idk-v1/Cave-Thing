#include <SFML/Graphics.hpp>
#include "SimplexNoise.hpp"
#include <vector>
#include <chrono>
#include <iostream>


int width = 1280, height = 720;
bool focus = true;
bool wk = 0, ak = 0, sk = 0, dk = 0, spacek = 0;
long now, last = 0, dt = 0, ticks = 0, fps = 1000 / 60;
int tileSize = 64, mapWidth = 200, mapHeight = 50, worldHeight = 5;
sf::RenderWindow window(sf::VideoMode(width, height), "Tiles");
int renderDist = std::max(window.getSize().x / tileSize / 2, window.getSize().y / tileSize / 2) + 1;
double posx = 1000, posy = 1000, posh = worldHeight - 1.0, velx = 0, vely = 0, playWidth = 0.8, step = 0.1, maxSpeed = 2.0;

std::vector<std::string>ids =
{
    "waterTop",
    "waterSide",
    "grassTop",
    "grassSide"
};
sf::Image missingImg, airImg;
sf::Texture missingTex, airTex;

SimplexNoise noise;

sf::RectangleShape rect;
std::vector<sf::Texture>tex;


struct TileAttr
{
    std::string name = "ERR";
    int id = -1, dropId = -1, damage = 1000, growth = -1, light = 0;
    double fric = 1.0;
};


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
sf::Texture createTex(std::string);


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
                renderDist = std::max(window.getSize().x / tileSize / 2, window.getSize().y / tileSize / 2) + 1;
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


sf::Texture createTex(std::string str)
{
    sf::Texture tmp;
    if (tmp.loadFromFile("textures/" + str + ".png"))
        return tmp;
    else
        return missingTex;
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

    const unsigned int numPixels = 32 * 32;
    sf::Uint8 pixels[4 * numPixels];
    for (int i = 0; i < numPixels; i++)
    {
        int xval = i % 32;
        int yval = i / 32;
        bool val = ((xval % 16 < 8 && yval % 16 < 8) || (xval % 16 >= 8 && yval % 16 >= 8));

        pixels[i * 4 + 0] = val * 255;
        pixels[i * 4 + 1] = 0;
        pixels[i * 4 + 2] = val * 255;
        pixels[i * 4 + 3] = 255;
    }
    
    missingImg.create(32, 32, pixels);
    missingTex.create(32, 32);
    missingTex.update(pixels);


    for (int i = 0; i < numPixels * 4; i++)
        pixels[i] = 255;
    airImg.create(32, 32, pixels);
    airTex.create(32, 32);
    airTex.update(pixels);

    for (int i = 0; i < ids.size(); i++)
        tex.push_back(createTex(ids[i]));

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

    if (isTileMovable(posh - 1, (posx - playWidth / 2 * tileSize) / tileSize, (posy - playWidth / 2 * tileSize) / tileSize) &&
        isTileMovable(posh - 1, (posx + playWidth / 2 * tileSize) / tileSize, (posy - playWidth / 2 * tileSize) / tileSize) &&
        isTileMovable(posh - 1, (posx + playWidth / 2 * tileSize) / tileSize, (posy + playWidth / 2 * tileSize) / tileSize) &&
        isTileMovable(posh - 1, (posx - playWidth / 2 * tileSize) / tileSize, (posy + playWidth / 2 * tileSize) / tileSize) &&
        posh > 0)
        posh--;
    if (isTileSolid(posh, (posx - playWidth / 2 * tileSize) / tileSize, (posy - playWidth / 2 * tileSize) / tileSize) ||
        isTileSolid(posh, (posx + playWidth / 2 * tileSize) / tileSize, (posy - playWidth / 2 * tileSize) / tileSize) ||
        isTileSolid(posh, (posx + playWidth / 2 * tileSize) / tileSize, (posy + playWidth / 2 * tileSize) / tileSize) ||
        isTileSolid(posh, (posx - playWidth / 2 * tileSize) / tileSize, (posy + playWidth / 2 * tileSize) / tileSize))
        posh++;
    if (isTileLiquid(posh, posx / tileSize, posy / tileSize))
    {
        velx /= 1.2;
        vely /= 1.2;
    }

    double oldPosx = posx, oldPosy = posy;


    // RIGHT
    if (velx > 0)
        for (int i = 0; i < std::abs(velx) * 100; i++)
            if (isTileMovable(posh + 1, (posx + playWidth / 2 * tileSize + step) / tileSize, (posy + playWidth / 2 * tileSize) / tileSize) &&
                isTileMovable(posh + 1, (posx + playWidth / 2 * tileSize + step) / tileSize, (posy - playWidth / 2 * tileSize) / tileSize))
                posx += step;
            else
                velx = 0;


    // LEFT
    if (velx < 0)
        for (int i = 0; i < std::abs(velx) * 100; i++)
            if (isTileMovable(posh + 1, (posx - playWidth / 2 * tileSize - step) / tileSize, (posy + playWidth / 2 * tileSize) / tileSize) &&
                isTileMovable(posh + 1, (posx - playWidth / 2 * tileSize - step) / tileSize, (posy - playWidth / 2 * tileSize) / tileSize))
                posx -= step;
            else
                velx = 0;

    // BOTTOM
    if (vely > 0)
        for (int i = 0; i < std::abs(vely) * 100; i++)
            if (isTileMovable(posh + 1, (posx - playWidth / 2 * tileSize) / tileSize, (posy + playWidth / 2 * tileSize + step) / tileSize) &&
                isTileMovable(posh + 1, (posx + playWidth / 2 * tileSize) / tileSize, (posy + playWidth / 2 * tileSize + step) / tileSize))
                posy += step;
            else
                vely = 0;

    // TOP
    if (vely < 0)
        for (int i = 0; i < std::abs(vely) * 100; i++)
            if (isTileMovable(posh + 1, (posx - playWidth / 2 * tileSize) / tileSize, (posy - playWidth / 2 * tileSize - step) / tileSize) &&
                isTileMovable(posh + 1, (posx + playWidth / 2 * tileSize) / tileSize, (posy - playWidth / 2 * tileSize - step) / tileSize))
                posy -= step;
            else
                vely = 0;

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
    int hgt, lvl = 128 / (worldHeight - 1);
    bool draw;
    window.clear();

    for (int x = -renderDist + posx / tileSize; x <= renderDist + posx / tileSize; x++)
        for (int y = -renderDist + posy / tileSize; y <= renderDist + posy / tileSize; y++)
        {
            if (x >= 0 && y >= 0 && x < mapWidth && y < mapHeight)
            {
                hgt = 0;
                for (int h = 0; h < worldHeight; h++)
                {
                    int id = getTile(h, x, y);
                    if (id > 0)
                        hgt = h;
                    if (id == -1)
                        hgt = 0;
                }
                for (int h = 0; h <= hgt; h++)
                {
                    draw = true;
                    double hgtmod = 0;
                    switch (getTile(h, x, y))
                    {
                    case 0:
                        draw = false;
                        break;
                    case 1:
                        hgtmod = -0.125;
                        rect.setTexture(&tex[1.0 - (h == hgt)]);
                        break;
                    case 2:
                        rect.setTexture(&tex[3.0 - (h == hgt)]);
                        break;
                    default:
                        rect.setTexture(&missingTex);
                    }
                    if (draw)
                    {
                        rect.setTextureRect(sf::IntRect(0, 0, 32, 32));
                        rect.setPosition(width / 2.0 + double(x) * tileSize - posx, height / 2.0 + double(y) * tileSize - posy - double(h) * tileSize * 0.125 + posh * tileSize * 0.125 - tileSize * hgtmod);
                        rect.setSize(sf::Vector2f(tileSize, tileSize));
                        rect.setFillColor(sf::Color(128 + lvl * hgt, 128 + lvl * hgt, 128 + lvl * hgt));
                        window.draw(rect);
                    }
                    if (hgtmod < 0)
                    {
                        draw = true;
                        switch (getTile(h, x, y - 1))
                        {
                        case 0:
                            draw = false;
                            break;
                        case 1:
                            hgtmod = -0.125;
                            rect.setTexture(&tex[1.0]);
                            break;
                        case 2:
                            rect.setTexture(&tex[3.0]);
                            break;
                        default:
                            rect.setTexture(&missingTex);
                        }
                        if (draw)
                        {
                            rect.setTextureRect(sf::IntRect(0, 0, 32, 4));
                            rect.setPosition(width / 2.0 + double(x) * tileSize - posx, height / 2.0 + double(y) * tileSize - posy - double(h) * tileSize * 0.125 + posh * tileSize * 0.125);
                            rect.setSize(sf::Vector2f(tileSize, tileSize * 0.125));
                            rect.setFillColor(sf::Color(128 + lvl * hgt, 128 + lvl * hgt, 128 + lvl * hgt));
                            window.draw(rect);
                        }
                    }
                }
            }
        }

    rect.setTexture(&airTex);
    rect.setSize(sf::Vector2f(playWidth * tileSize, playWidth * tileSize));
    rect.setPosition(width / 2.0 - playWidth * tileSize / 2, height / 2.0 - playWidth * tileSize / 2);
    rect.setFillColor(sf::Color(96 + lvl * posh, 96 + lvl * posh, 96 + lvl * posh));
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
