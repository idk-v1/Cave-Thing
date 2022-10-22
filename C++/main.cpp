#include "SFML/Graphics.hpp"
#include "Perlin.h"
#include <chrono>
#include <thread>
#include <windows.h>
#include <iostream>
#include <sstream>

const int
WIDTH = 960,
HEIGHT = 720;

enum ID {
// ID 0000 - Air
// ID 0001 - Stone
// ID 0010 - Water
// ID 0011 - Ore
// ID 0100 - Tree
// ID 0101 - Light
// ID 0110 - Bricks
// ID 0111 - Wood
// ID 1000 - 
// ID 1001 - 
// ID 1010 - 
// ID 1011 - 
// ID 1100 - 
// ID 1101 - 
// ID 1110 - 
// ID 1111 - 

	air,
	stone,
	water,
	ore,
	tree,
	light,
	bricks,
	wood,
};

int
tps = 1000 / 20,
tileSize = 16,
lightLevel = 16,
viewDist = 32,
mWidth = 500,
mHeight = 500;

long
now,
last,
delta = 0,
ticks = 0;

double
noiseLvl = 0.01;

std::stringstream ss;

bool
w = false,
a = false,
s = false,
d = false,
r = false,
ctrl = false,
shift = false,
regenKey = false,
lightKey = false;

char** terrain = new char* [mWidth];
char** lights = new char* [mWidth];


class Vector
{
public:
	int x;
	int y;
	Vector(int xPos, int yPos)
	{
		x = xPos;
		y = yPos;
	}
};


class Item
{
public:
	int count = 0;
	int id = 0;
};


class Player
{
public:
	int
		xVelo = 0,
		yVelo = 0,
		x = int(mWidth / 2),
		y = int(mHeight / 2),
		xStart = int(mWidth / 2),
		yStart = int(mHeight / 2),
		dir = 0,
		width = tileSize * 3,
		destroyTime = 0,
		mineSpeed = 0,
		speed = 1,
		health = 50,
		invNum = 0;

	Item
		inv[256];

	bool
		light = true;
};


Perlin perlin;

Player play;

sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Cave");

sf::Font font;

sf::Text debug;

sf::Texture* tex[16];

sf::RectangleShape
tile,
playTile,
invTile,
invTileItem;


int getLight(int xTile, int yTile)
{
	if (xTile >= 0 && xTile < mWidth && yTile >= 0 && yTile < mHeight)
		return
		((lights[xTile][yTile] >> 0) & 1) +
		((lights[xTile][yTile] >> 1) & 1) * 2 +
		((lights[xTile][yTile] >> 2) & 1) * 4 +
		((lights[xTile][yTile] >> 3) & 1) * 8;
	else return -1;
}


int getID(int xTile, int yTile)
{
	if (xTile >= 0 && xTile < mWidth && yTile >= 0 && yTile < mHeight)
		return
		((terrain[xTile][yTile] >> 0) & 1) +
		((terrain[xTile][yTile] >> 1) & 1) * 2 +
		((terrain[xTile][yTile] >> 2) & 1) * 4 +
		((terrain[xTile][yTile] >> 3) & 1) * 8;
	else return -1;
}


void setLight(int xTile, int yTile, int Level)
{
	if (xTile >= 0 && xTile < mWidth && yTile >= 0 && yTile < mHeight)
	{
		lights[xTile][yTile] &= ~(1 << 3);
		lights[xTile][yTile] &= ~(1 << 2);
		lights[xTile][yTile] &= ~(1 << 1);
		lights[xTile][yTile] &= ~(1 << 0);

		if (Level >= 8) { Level -= 8; lights[xTile][yTile] |= 1 << 3; }
		else lights[xTile][yTile] |= 0 << 3;
		if (Level >= 4) { Level -= 4; lights[xTile][yTile] |= 1 << 2; }
		else lights[xTile][yTile] |= 0 << 2;
		if (Level >= 2) { Level -= 2; lights[xTile][yTile] |= 1 << 1; }
		else lights[xTile][yTile] |= 0 << 1;
		if (Level >= 1) { Level -= 1; lights[xTile][yTile] |= 1 << 0; }
		else lights[xTile][yTile] |= 0 << 0;
	}
}


void resetLight(bool init)
{
	if (init)
	{
		for (int x = 0; x < mWidth; x++)
		{
			for (int y = 0; y < mHeight; y++)
			{
				if (x >= 0 && x < mWidth && y >= 0 && y < mHeight)
					lights[x][y] = (char)0;
			}
		}
	}
	else
	{
		for (int x = play.x - viewDist; x < play.x + viewDist; x++)
		{
			for (int y = play.y - viewDist; y < play.y + viewDist; y++)
			{
				if (x >= 0 && x < mWidth && y >= 0 && y < mHeight)
					lights[x][y] = (char)0;
			}
		}
	}
}


void setID(int xTile, int yTile, int Value)
{
	if (xTile >= 0 && xTile < mWidth && yTile >= 0 && yTile < mHeight)
	{
		terrain[xTile][yTile] &= ~(1 << 3);
		terrain[xTile][yTile] &= ~(1 << 2);
		terrain[xTile][yTile] &= ~(1 << 1);
		terrain[xTile][yTile] &= ~(1 << 0);

		if (Value >= 8) {Value -= 8; terrain[xTile][yTile] |= 1 << 3;} else terrain[xTile][yTile] |= 0 << 3;
		if (Value >= 4) {Value -= 4; terrain[xTile][yTile] |= 1 << 2;} else terrain[xTile][yTile] |= 0 << 2;
		if (Value >= 2) {Value -= 2; terrain[xTile][yTile] |= 1 << 1;} else terrain[xTile][yTile] |= 0 << 1;
		if (Value >= 1) {Value -= 1; terrain[xTile][yTile] |= 1 << 0;} else terrain[xTile][yTile] |= 0 << 0;
	}
}


void generate()
{
	for (int x = 0; x < mWidth; x++)
	{
		for (int y = 0; y < mHeight; y++)
		{
			// Bit 0 - ID 1
			// Bit 1 - ID 2
			// Bit 2 - ID 4
			// Bit 3 - ID 8
			// Bit 4 - Light 1
			// Bit 5 - Light 2
			// Bit 6 - Light 4
			// Bit 7 - Light 8

			// Get Noise from Coordinate
			if (!std::floor(perlin.noise(x * 0.025, y * 0.025, noiseLvl) + 1.0625))
			{
				setID(x, y, 0);
			}
			else 
			{
				setID(x, y, 1);
			}
		}
	}
	resetLight(true);
	for (int x = -1; x < 2; x++)
		for (int y = -1; y < 2; y++)
			setID(play.x - x, play.y - y, 0);
}


void render()
{
	double light;
	window.clear();

	for (int x = play.x - viewDist; x < play.x + viewDist; x++)
	{
		for (int y = play.y - viewDist; y < play.y + viewDist; y++)
		{
			if (x >= 0 && x < mWidth && y >= 0 && y < mHeight)
			{
				light = std::ceil((getLight(x, y) + 1) / 2.0) * 2.0 / 16.0;
				tile.setFillColor(sf::Color(std::ceil(255 * light), std::ceil(255 * light), std::ceil(255 * light)));
				tile.setTexture(tex[getID(x, y)]);
				tile.setPosition(sf::Vector2f((WIDTH * 0.5 - mWidth * 0.5 * tileSize + (mWidth / 2 - play.x) * tileSize) + (x * tileSize - tileSize * 0.5), (HEIGHT * 0.5 - mHeight * 0.5 * tileSize + (mHeight / 2 - play.y) * tileSize) + (y * tileSize - tileSize * 0.5)));
				window.draw(tile);
			}
		}
	}

	light = std::ceil((getLight(play.x, play.y) + 1) / 2.0) * 2.0 / 16.0;
	playTile.setFillColor(sf::Color(64 * light, 192 * light, 224 * light));
	playTile.setPosition(int(WIDTH / 2 - play.width / 2), int(HEIGHT / 2 - play.width / 2));
	window.draw(playTile);

	switch (play.dir)
	{
	case 0:
		tile.setPosition(int(WIDTH / 2 - tileSize / 2), int(HEIGHT / 2 - tileSize / 2 - tileSize));
		playTile.setPosition(sf::Vector2f(WIDTH / 2 - tileSize / 2 - tileSize, HEIGHT / 2 - tileSize / 2 - play.width - tileSize));
		break;
	case 1:
		tile.setPosition(int(WIDTH / 2 - tileSize / 2 + tileSize), int(HEIGHT / 2 - tileSize / 2 - tileSize));
		break;
	case 2:
		tile.setPosition(int(WIDTH / 2 - tileSize / 2 + tileSize), int(HEIGHT / 2 - tileSize / 2));
		playTile.setPosition(sf::Vector2f(WIDTH / 2 - tileSize / 2 + play.width - tileSize, HEIGHT / 2 - tileSize / 2 - tileSize));
		break;
	case 3:
		tile.setPosition(int(WIDTH / 2 - tileSize / 2 + tileSize), int(HEIGHT / 2 - tileSize / 2 + tileSize));
		break;
	case 4:
		tile.setPosition(int(WIDTH / 2 - tileSize / 2), int(HEIGHT / 2 - tileSize / 2 + tileSize));
		playTile.setPosition(sf::Vector2f(WIDTH / 2 - tileSize / 2 - tileSize, HEIGHT / 2 - tileSize / 2 + play.width - tileSize));
		break;
	case 5:
		tile.setPosition(int(WIDTH / 2 - tileSize / 2 - tileSize), int(HEIGHT / 2 - tileSize / 2 + tileSize));
		break;
	case 6:
		tile.setPosition(int(WIDTH / 2 - tileSize / 2 - tileSize), int(HEIGHT / 2 - tileSize / 2));
		playTile.setPosition(sf::Vector2f(WIDTH / 2 - tileSize / 2 - play.width - tileSize, HEIGHT / 2 - tileSize / 2 - tileSize));
		break;
	case 7:
		tile.setPosition(int(WIDTH / 2 - tileSize / 2 - tileSize), int(HEIGHT / 2 - tileSize / 2 - tileSize));
		break;
	default:
		tile.setPosition(int(WIDTH / 2 - tileSize / 2), int(HEIGHT / 2 - tileSize / 2));
	}

	tile.setFillColor(sf::Color(96, 224, 255));
	tile.setTexture(tex[0]);
	window.draw(tile);

	if (play.dir == 0 || play.dir == 2 || play.dir == 4 || play.dir == 6)
	{
		playTile.setFillColor(sf::Color(255, 0, 0, 128 * play.destroyTime * 0.1));
		window.draw(playTile);
	}

	for (int i = 0; i < 9; i++)
	{
		if (play.invNum == 8 - i) invTile.setFillColor(sf::Color(96, 96, 96));
		else invTile.setFillColor(sf::Color(64, 64, 64));
		invTile.setPosition(WIDTH / 2 - (i - 4) * 55 - 25, HEIGHT - 75);
		window.draw(invTile);
		invTileItem.setFillColor(sf::Color(255, 255, 255));
		invTileItem.setTexture(tex[play.inv[8 - i].id]);
		invTileItem.setPosition(WIDTH / 2 - (i - 4) * 55 - 16, HEIGHT - 75 + 9);
		window.draw(invTileItem);
	}

	ss.str("");
	ss << "X: " << play.x << " Y: " << play.y;
	debug.setString(ss.str());
	window.draw(debug);

	window.display();
}


void key()
{
	// "0xnn < 0" Means Key is Down
	// "0xnn == 0" Means Key is Up

	if (GetAsyncKeyState(0x30) < 0) play.invNum = -1;
	if (GetAsyncKeyState(0x31) < 0) play.invNum = 0;
	if (GetAsyncKeyState(0x32) < 0) play.invNum = 1;
	if (GetAsyncKeyState(0x33) < 0) play.invNum = 2;
	if (GetAsyncKeyState(0x34) < 0) play.invNum = 3;
	if (GetAsyncKeyState(0x35) < 0) play.invNum = 4;
	if (GetAsyncKeyState(0x36) < 0) play.invNum = 5;
	if (GetAsyncKeyState(0x37) < 0) play.invNum = 6;
	if (GetAsyncKeyState(0x38) < 0) play.invNum = 7;
	if (GetAsyncKeyState(0x39) < 0) play.invNum = 8;

	if (GetAsyncKeyState(0x57) < 0) w = true;
	if (GetAsyncKeyState(0x41) < 0) a = true;
	if (GetAsyncKeyState(0x53) < 0) s = true;
	if (GetAsyncKeyState(0x44) < 0) d = true;

	if (GetAsyncKeyState(0x10) < 0) shift = true;
	if (GetAsyncKeyState(0x11) < 0) ctrl = true;

	if (GetAsyncKeyState(0x52) < 0) r = true;
	if (GetAsyncKeyState(0x1B) < 0) window.close();

	if (GetAsyncKeyState(0x57) == 0) w = false;
	if (GetAsyncKeyState(0x41) == 0) a = false;
	if (GetAsyncKeyState(0x53) == 0) s = false;
	if (GetAsyncKeyState(0x44) == 0) d = false;

	if (GetAsyncKeyState(0x10) == 0) shift = false;
	if (GetAsyncKeyState(0x11) == 0) ctrl = false;

	if (GetAsyncKeyState(0x52) == 0) r = false;
}


void lightDiamond(int xTile, int yTile, int Value)
{
	if (Value > 0)
	{
		if (xTile >= 0 && xTile < mWidth && yTile >= 0 && yTile < mHeight)
		{
			setLight(xTile, yTile, Value);
			if (Value > getLight(xTile, yTile - 1) && (getID(xTile, yTile) == 0 || getID(xTile, yTile) == 5)) lightDiamond(xTile, yTile - 1, Value - 1);
			if (Value > getLight(xTile + 1, yTile) && (getID(xTile, yTile) == 0 || getID(xTile, yTile) == 5)) lightDiamond(xTile + 1, yTile, Value - 1);
			if (Value > getLight(xTile, yTile + 1) && (getID(xTile, yTile) == 0 || getID(xTile, yTile) == 5)) lightDiamond(xTile, yTile + 1, Value - 1);
			if (Value > getLight(xTile - 1, yTile) && (getID(xTile, yTile) == 0 || getID(xTile, yTile) == 5)) lightDiamond(xTile - 1, yTile, Value - 1);
		}
	}
}


void tick()
{
	ticks++;

	play.xVelo -= (a * play.speed - d * play.speed);
	play.yVelo -= (w * play.speed - s * play.speed);

	if (ctrl)
	{
		play.destroyTime++;
		if (play.destroyTime >= 10)
		{
			play.destroyTime = 0;

			int xDir = 0;
			int yDir = 0;
			bool placeOk = true;

			switch (play.dir)
			{
			case 0:
				yDir = -1;
				break;
			case 2:
				xDir = 1;
				break;
			case 4:
				yDir = 1;
				break;
			case 6:
				xDir = -1;
				break;
			default:
				placeOk = false;
			}

			int itemid;
			if (placeOk)
			{
				if (play.invNum == -1) setID(play.x + 3 * xDir, play.y + 3 * yDir, 0);
				if (play.inv[play.invNum].id == 5)
				{
					if (getID(play.x + 3 * xDir, play.y + 3 * yDir) == 0)
						setID(play.x + 3 * xDir, play.y + 3 * yDir, 5);
				}
				else
				{
					itemid = play.inv[play.invNum].id;
					if (itemid != 5) for (int x = -1; x < 2; x++) for (int y = -1; y < 2; y++)
						setID(play.x - x + 3 * xDir, play.y - y + 3 * yDir, itemid);
				}
			}
		}
	}
	else play.destroyTime = 0;

	if (shift && !lightKey) play.light = !play.light;
	lightKey = shift;
	
	if (play.xVelo > 0)
	{
		for (int x = 0; x < play.xVelo; x++)
		{
			if (play.x + 2 < mWidth) if (
				(getID(play.x + 2, play.y - 1) == 0 || getID(play.x + 2, play.y - 1) == 5) &&
				(getID(play.x + 2, play.y) == 0 || getID(play.x + 2, play.y) == 5) &&
				(getID(play.x + 2, play.y + 1) == 0 || getID(play.x + 2, play.y + 1) == 5)
				) play.x++;
		}
	}
	if (play.xVelo < 0)
	{
		for (int x = 0; x > play.xVelo; x--)
		{
			if (play.x - 1 > 0) if (
				(getID(play.x - 2, play.y - 1) == 0 || getID(play.x - 2, play.y - 1) == 5) &&
				(getID(play.x - 2, play.y) == 0 || getID(play.x - 2, play.y) == 5) &&
				(getID(play.x - 2, play.y + 1) == 0 || getID(play.x - 2, play.y + 1) == 5)
				) play.x--;
		}
	}
	if (play.yVelo > 0)
	{
		for (int y = 0; y < play.yVelo; y++)
		{
			if (play.y + 1 <= mHeight) if (
				(getID(play.x - 1, play.y + 2) == 0 || getID(play.x - 1, play.y + 2) == 5) &&
				(getID(play.x, play.y + 2) == 0 || getID(play.x, play.y + 2) == 5) &&
				(getID(play.x + 1, play.y + 2) == 0 || getID(play.x + 1, play.y + 2) == 5)
				) play.y++;
		}
	}
	if (play.yVelo < 0)
	{
		for (int y = 0; y > play.yVelo; y--)
		{
			if (play.y - 1 > 0) if (
				(getID(play.x - 1, play.y - 2) == 0 || getID(play.x - 1, play.y - 2) == 5) &&
				(getID(play.x, play.y - 2) == 0 || getID(play.x, play.y - 2) == 5) &&
				(getID(play.x + 1, play.y - 2) == 0 || getID(play.x + 1, play.y - 2) == 5)
				) play.y--;
		}
	}

	if (play.x <= 0 + 1) play.x = 0 + 1;
	if (play.x > mWidth - 2) play.x = mWidth - 2;
	if (play.y - 1 <= 0) play.y = 1;
	if (play.y + 2 > mHeight) play.y = mHeight - 2;

	if (w && !d && !a) play.dir = 0;
	if (w && d) play.dir = 1;
	if (d && !w && !s) play.dir = 2;
	if (d && s) play.dir = 3;
	if (s && !a && !d) play.dir = 4;
	if (s && a) play.dir = 5;
	if (a && !w && !s) play.dir = 6;
	if (a && w) play.dir = 7;

	play.xVelo = play.xVelo * 0.25;
	play.yVelo = play.yVelo * 0.25;

	if (r && !regenKey)
	{
		play.x = play.xStart;
		play.y = play.yStart;
		play.xVelo = 0;
		play.yVelo = 0;
		play.health = 50;
		play.dir = 0;
		if (shift)
		{
			noiseLvl++;
			generate();
		}
	}
	regenKey = r;

	for (int x = 0; x < mWidth; x++)
	{
		for (int y = 0; y < mHeight; y++)
		{
			setLight(x, y, 0);
		}
	}
	if (play.light) lightDiamond(play.x, play.y, 15);

	for (int x = play.x - viewDist - 15; x < play.x + viewDist + 15; x++)
	{
		for (int y = play.y - viewDist - 15; y < play.y + viewDist + 15; y++)
		{
			if (getID(x, y) == 5)
			{
				lightDiamond(x, y, 12);
			}
		}
	}
}


int main()
{
	window.setPosition(sf::Vector2i(window.getPosition().x, 0));
	window.setFramerateLimit(60);

	font.loadFromFile("res/cas.ttf");
	debug.setFont(font);
	debug.setPosition(0, 0);
	debug.setFillColor(sf::Color(255, 255, 255));

	tex[0] = new sf::Texture; tex[0]->loadFromFile("res/air.png");
	tex[1] = new sf::Texture; tex[1]->loadFromFile("res/stone.png");
	tex[2] = new sf::Texture; tex[2]->loadFromFile("res/air.png");
	tex[3] = new sf::Texture; tex[3]->loadFromFile("res/air.png");
	tex[4] = new sf::Texture; tex[4]->loadFromFile("res/air.png");
	tex[5] = new sf::Texture; tex[5]->loadFromFile("res/light.png");

	playTile.setSize(sf::Vector2f(play.width, play.width));
	tile.setSize(sf::Vector2f(tileSize, tileSize));
	invTile.setSize(sf::Vector2f(50, 50));
	invTileItem.setSize(sf::Vector2f(32, 32));


	for (int x = 0; x < mWidth; x++)
	{
		terrain[x] = new char[mHeight];
		lights[x] = new char[mHeight];
	}

	generate();

	play.inv[0].count = 100;
	play.inv[0].id = 1;
	play.inv[1].count = 100;
	play.inv[1].id = 5;
	play.inv[2].count = 100;
	play.inv[2].id = 0;
	play.inv[3].count = 100;
	play.inv[3].id = 0;
	play.inv[4].count = 100;
	play.inv[4].id = 0;

	now = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
	last = now;

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				window.close();
			}
		}

		now = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
		delta += now - last;
		last = now;

		while (delta >= tps)
		{
			delta -= tps;
			tick();
		}

		key();
		render();
	}

	return 0;
}
