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
int renderDist = 64;
double posx = 1, posy = 1, posh = worldHeight - 1.0, velx = 0, vely = 0;

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


int getTile(int h, int x, int y)
{
    if (h >= 0 && h < worldHeight && x >= 0 && x < mapWidth && y >= 0 && y < mapHeight)
        return tiles[h][x][y].id;
    return -1;
}


void setTile(int h, int x, int y, int id)
{
    if (h >= 0 && h < worldHeight && x >= 0 && x < mapWidth && y >= 0 && y < mapHeight)
        tiles[h][x][y] = id;
}


void generate()
{
    for (int x = 0; x < mapWidth; x++)
        for (int y = 0; y < mapHeight; y++)
        {
            for (int h = 1; h < worldHeight; h++)
                setTile(h, x, y, 0);
            setTile(0, x, y, 1);
            int hgt = noise.unsignedFBM(x * 0.05, y * 0.05, 3, 0.1, 0.1) * worldHeight;
            for (int h = 0; h < hgt; h++)
                setTile(h, x, y, 2);
        }
}


void update()
{
    velx += (double(dk) - double(ak)) * 0.05;
    vely += (double(sk) - double(wk)) * 0.05;

    if (velx >= 0.1) velx = 0.1;
    if (velx <= -0.1) velx = -0.1;
    if (vely >= 0.1) vely = 0.1;
    if (vely <= -0.1) vely = -0.1;

    if (getTile(posh - 1, posx, posy) != 2 && posh > 0)
        posh--;

    // LEFT
    if (velx < 0)
        for (int i = 0; i < std::abs(velx) * 100; i++)
        {
            if (spacek || (getTile(posh, std::floor(posx - 0.4 - 0.01), std::floor(posy - 0.4)) != 2 && getTile(posh, std::floor(posx - 0.4 - 0.01), std::ceil(posy - 0.4)) != 2))
                posx -= 0.01;
            else velx = 0;
        }


    // RIGHT
    if (velx > 0)
        for (int i = 0; i < std::abs(velx) * 100; i++)
        {
            if (spacek || (getTile(posh, std::floor(posx + 0.4 + 0.01), std::floor(posy - 0.4)) != 2 && getTile(posh, std::floor(posx + 0.4 + 0.01), std::ceil(posy - 0.4)) != 2))
                posx += 0.01;
            else
                velx = 0;
        }

    // UP
    if (vely < 0)
        for (int i = 0; i < std::abs(vely) * 100; i++)
        {
            if (spacek || (getTile(posh, std::floor(posx - 0.4), std::floor(posy - 0.4 - 0.01)) != 2 && getTile(posh, std::ceil(posx - 0.4), std::floor(posy - 0.4 - 0.01)) != 2))
                posy -= 0.01;
            else vely = 0;
        }

    // DOWN
    if (vely > 0)
        for (int i = 0; i < std::abs(vely) * 100; i++)
        {
            if (spacek || (getTile(posh, std::floor(posx - 0.4), std::ceil(posy - 0.6 + 0.01)) != 2 && getTile(posh, std::ceil(posx - 0.4), std::ceil(posy - 0.6 + 0.01)) != 2))
                posy += 0.01;
            else vely = 0;
        }

    velx /= 1.05;
    vely /= 1.05;
    if (std::abs(velx) < 0.01) velx = 0;
    if (std::abs(vely) < 0.01) vely = 0;

    if (posx + 0.4 >= mapWidth) posx = mapWidth - 0.4;
    if (posy + 0.4 >= mapHeight) posy = mapHeight - 0.4;
    if (posx - 0.4 < 0) posx = 0.4;
    if (posy - 0.4 < 0) posy = 0.4;

    //std::cout << "POS - L: " << posx - 0.4 << " R: " << posx + 0.4 << "\n";
}


void render()
{
    int hgt, r, g, b, lvl = 128 / (worldHeight - 1);
    window.clear();

    for (int x = -renderDist + posx; x <= renderDist + posx; x++)
        for (int y = -renderDist + posy; y <= renderDist + posy; y++)
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
            rect.setPosition(width / 2 - (-x + posx) * tileSize, height / 2 - (-y + posy) * tileSize);
            rect.setSize(sf::Vector2f(tileSize, tileSize));
            rect.setFillColor(sf::Color(r + lvl * hgt, g + lvl * hgt, b + lvl * hgt));
            window.draw(rect);
            if ((y == std::floor(posy - 0.4 - 0.01) && (x == std::floor(posx - 0.4) || x == std::ceil(posx - 0.4))) || (y == std::ceil(posy - 0.6 + 0.01) && (x == std::floor(posx - 0.4) || x == std::ceil(posx - 0.4))))
            {
                rect.setFillColor(sf::Color(255, 255, 0, 128));
                window.draw(rect);
            }
            if ((x == std::floor(posx - 0.4 - 0.01) && (y == std::floor(posy - 0.4) || y == std::ceil(posy - 0.4))) || (x == std::floor(posx + 0.4 + 0.01) && (y == std::floor(posy - 0.4) || y == std::ceil(posy - 0.4))))
            {
                rect.setFillColor(sf::Color(255, 0, 0, 128));
                window.draw(rect);
            }
        }

    rect.setSize(sf::Vector2f(tileSize * 0.8, tileSize * 0.8));
    rect.setPosition(width / 2.0 - tileSize / 2 + 0.1 * tileSize, height / 2.0 - tileSize / 2 + 0.1 * tileSize);
    rect.setFillColor(sf::Color(128 + lvl * posh, 128 + lvl * posh, 128 + lvl * posh));
    window.draw(rect);

    window.display();
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


int main()
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
        }
        render();
    }

    return 0;
}
