#include "SFML/Graphics.hpp"
#include "Perlin.h"
#include <chrono>
#include <iostream>
#include <thread>
#include <windows.h>


const int width = 640;
const int height = 480;
const int tps = 1000 / 60;

int mwidth = 500;
int mheight = 500;
int tilesize = 10;
int maxll = 20;
int viewdist = 30;

long int now;
long int last;
long int delta;
long int ticks;

bool w = false;
bool a = false;
bool s = false;
bool d = false;

bool r = false;

bool ctrl = false;
bool shift = false;

char** terrain = new char*[mwidth];


class Player
{
public:
	double x = mwidth / 2;
	double y = mheight / 2;
	double xs = mwidth / 2;
	double ys = mheight / 2;
	double vx = 0;
	double vy = 0;
	int dir = 0;
	double speed = 0.25;
	int width = tilesize * 3;
};

Player play;

Perlin perlin;

sf::RenderWindow window(sf::VideoMode(width, height), "Cave");

sf::RectangleShape tile;
sf::RectangleShape playtile;


void lighttile(int x, int y, int level)
{
	/*
	if (x < mwidth && x >= 0 && y >= 0 && y < mheight) if (terrain[x][y] != 0) light[x][y] = level;
	if (level > 0)
	{
		if (x < mwidth && x >= 0 && y >= 0 && y < mheight) if (light[x - 1][y] < level - 1 && terrain[x - 1][y] != 0) lighttile(x - 1, y, level - 1);
		if (x < mwidth && x >= 0 && y >= 0 && y < mheight) if (light[x + 1][y] < level - 1 && terrain[x + 1][y] != 0) lighttile(x + 1, y, level - 1);
		if (x < mwidth && x >= 0 && y >= 0 && y < mheight) if (light[x][y - 1] < level - 1 && terrain[x][y - 1] != 0) lighttile(x, y - 1, level - 1);
		if (x < mwidth && x >= 0 && y >= 0 && y < mheight) if (light[x][y + 1] < level - 1 && terrain[x][y + 1] != 0) lighttile(x, y + 1, level - 1);
	}
	*/
}


void render()
{
	window.clear();

	for (int x = 0; x < mwidth; x++)
	{
		for (int y = 0; y < mheight; y++)
		{
			if (abs(play.x - x - fmod(play.x, 1)) <= viewdist &&
				abs(play.x - x - fmod(play.x, 1)) >= -viewdist &&
				abs(play.y - y - fmod(play.y, 1)) <= viewdist &&
				abs(play.y - y - fmod(play.y, 1)) >= -viewdist)
			{

				if (!((terrain[x][y] >> 0) & 1))
				{
					tile.setFillColor(sf::Color(32, 32, 32));
				}
				else 
				{
					tile.setFillColor(sf::Color(128, 128, 128));
				}

				tile.setPosition(sf::Vector2f((width / 2 - mwidth * 0.5 * tilesize + (mwidth / 2 - play.x) * tilesize) + (x * tilesize - tilesize / 2), (height / 2 - mheight * 0.5 * tilesize + (mheight / 2 - play.y) * tilesize) + (y * tilesize - tilesize / 2)));
				window.draw(tile);
			}
		}
	}

	playtile.setFillColor(sf::Color(255, 0, 0, 128));
	playtile.setPosition(int (width / 2 - play.width / 2), int (height / 2 - play.width / 2));
	window.draw(playtile);
	
	tile.setFillColor(sf::Color(255, 0, 0, 128));
	switch (play.dir)
	{
	case 0:
		tile.setPosition(int(width / 2 - tilesize / 2), int(height / 2 - tilesize / 2 - tilesize));
		break;
	case 1:
		tile.setPosition(int(width / 2 - tilesize / 2 + tilesize), int(height / 2 - tilesize / 2));
		break;
	case 2:
		tile.setPosition(int(width / 2 - tilesize / 2), int(height / 2 - tilesize / 2 + tilesize));
		break;
	case 3:
		tile.setPosition(int(width / 2 - tilesize / 2 - tilesize), int(height / 2 - tilesize / 2));
		break;
	}

	window.draw(tile);


	window.display();
}


void clear(int dir)
{
	/*
		0 = W = U
		1 = D = R
		2 = S = D
		3 = A = L

		4 = WD = UR
		5 = SD = DR
		6 = SA = DL
		7 = WA = UL
	*/

	terrain[int(std::round(play.x)) - 1][int(std::round(play.y)) - 1] |= 1 << 0;
	terrain[int(std::round(play.x)) + 0][int(std::round(play.y)) - 1] |= 1 << 0;
	terrain[int(std::round(play.x)) + 1][int(std::round(play.y)) - 1] |= 1 << 0;
	terrain[int(std::round(play.x)) - 1][int(std::round(play.y)) + 0] |= 1 << 0;
	terrain[int(std::round(play.x)) + 0][int(std::round(play.y)) + 0] |= 1 << 0;
	terrain[int(std::round(play.x)) + 1][int(std::round(play.y)) + 0] |= 1 << 0;
	terrain[int(std::round(play.x)) - 1][int(std::round(play.y)) + 1] |= 1 << 0;
	terrain[int(std::round(play.x)) + 0][int(std::round(play.y)) + 1] |= 1 << 0;
	terrain[int(std::round(play.x)) + 1][int(std::round(play.y)) + 1] |= 1 << 0;

	switch (dir)
	{
	case 0:
		if (play.y - 2 <= 0)
		{
			terrain[int(std::round(play.x)) - 1][int(std::round(play.y)) - 2] |= 1 << 0;
			terrain[int(std::round(play.x)) + 0][int(std::round(play.y)) - 2] |= 1 << 0;
			terrain[int(std::round(play.x)) + 1][int(std::round(play.y)) - 2] |= 1 << 0;
		}
		break;
	case 1:
		if (play.x + 2 <= mwidth - 1)
		{
			terrain[int(std::round(play.x)) + 2][int(std::round(play.y)) - 1] |= 1 << 0;
			terrain[int(std::round(play.x)) + 2][int(std::round(play.y)) + 0] |= 1 << 0;
			terrain[int(std::round(play.x)) + 2][int(std::round(play.y)) + 1] |= 1 << 0;
		}
		break;
	case 2:
		if (play.y + 2 >= mheight - 1)
		{
			terrain[int(std::round(play.x)) - 1][int(std::round(play.y)) + 2] |= 1 << 0;
			terrain[int(std::round(play.x)) + 0][int(std::round(play.y)) + 2] |= 1 << 0;
			terrain[int(std::round(play.x)) + 1][int(std::round(play.y)) + 2] |= 1 << 0;
		}
		break;
	case 3:
		if (play.x - 2 >= 0)
		{
			terrain[int(std::round(play.x)) - 2][int(std::round(play.y)) - 1] |= 1 << 0;
			terrain[int(std::round(play.x)) - 2][int(std::round(play.y)) + 0] |= 1 << 0;
			terrain[int(std::round(play.x)) - 2][int(std::round(play.y)) + 1] |= 1 << 0;
		}
		break;
	}
}


void generate()
{
	for (int x = 0; x < mheight; x++)
	{
		for (int y = 0; y < mwidth; y++)
		{
			double output = 0;
			output = perlin.noise(x * 0.05, y * 0.05, 1.1) + 1.01;
			if (std::floor(output))
			{
				terrain[x][y] ^= 1 << 0;
			}
		}
	}
	clear(-1);
}


void tick()
{
	ticks++;

	play.vx -= (a * play.speed - d * play.speed) * (0.2 * shift + 1 * !shift);
	play.vy -= (w * play.speed - s * play.speed) * (0.2 * shift + 1 * !shift);

	//if (w && !d && !s && !a) play.vy -= play.speed - play.speed / 2 * shift;
	//if (!w && d && !s && !a) play.vx += play.speed - play.speed / 2 * shift;
	//if (!w && !d && s && !a) play.vy += play.speed - play.speed / 2 * shift;
	//if (!w && !d && !s && a) play.vx -= play.speed - play.speed / 2 * shift;

	play.x += play.vx;
	play.y += play.vy;

	if (play.x + 1 >= mwidth - 1) play.x = mwidth - 2;
	if (play.x - 1 <= 0) play.x = 1;
	if (play.y + 1 >= mheight - 1) play.y = mheight - 2;
	if (play.y - 1 <= 0) play.y = 1;

	if (w && !d && !s && !a) play.dir = 0;
	if (!w && d && !s && !a) play.dir = 1;
	if (!w && !d && s && !a) play.dir = 2;
	if (!w && !d && !s && a) play.dir = 3;

	if (ctrl)
	{
		clear(play.dir);
	}

	bool mu = false, mr = false, md = false, ml = false;

	if (
		(~(terrain[int(std::round(play.x)) - 1][int(std::round(play.y)) - 1] >> 0) & 1) ||
		(~(terrain[int(std::round(play.x)) + 0][int(std::round(play.y)) - 1] >> 0) & 1) ||
		(~(terrain[int(std::round(play.x)) + 1][int(std::round(play.y)) - 1] >> 0) & 1)
		) mu = true;

	if (
		(~(terrain[int(std::round(play.x)) + 1][int(std::round(play.y)) - 1] >> 0) & 1) ||
		(~(terrain[int(std::round(play.x)) + 1][int(std::round(play.y)) + 0] >> 0) & 1) ||
		(~(terrain[int(std::round(play.x)) + 1][int(std::round(play.y)) + 1] >> 0) & 1)
		) mr = true;

	if (
		(~(terrain[int(std::round(play.x)) - 1][int(std::round(play.y)) + 1] >> 0) & 1) ||
		(~(terrain[int(std::round(play.x)) + 0][int(std::round(play.y)) + 1] >> 0) & 1) ||
		(~(terrain[int(std::round(play.x)) + 1][int(std::round(play.y)) + 1] >> 0) & 1)
		) md = true;

	if (
		(~(terrain[int(std::round(play.x)) - 1][int(std::round(play.y)) - 1] >> 0) & 1) ||
		(~(terrain[int(std::round(play.x)) - 1][int(std::round(play.y)) + 0] >> 0) & 1) ||
		(~(terrain[int(std::round(play.x)) - 1][int(std::round(play.y)) + 1] >> 0) & 1)
		) ml = true;

	if (mu) play.y = std::floor(play.y) + 1;
	if (mr) play.x = std::ceil(play.x) - 1;
	if (md) play.y = std::ceil(play.y) - 1;
	if (ml) play.x = std::floor(play.x) + 1;

	play.vx *= 0.3;
	play.vy *= 0.3;

	/*
	for (int x = 0; x < mwidth; x++)
	{
		for (int y = 0; y < mheight; y++)
		{
			light[x][y] = 0;
		}
	}
	*/

	lighttile(round(play.x), round(play.y), 20);

	//std::cout << play.x << " | " << play.y << "\n";
	if (r)
	{
		play.x = play.xs;
		play.y = play.ys;
		play.dir = 0;
		play.vx = 0;
		play.vy = 0;
	}
}


void key()
{
	if (GetAsyncKeyState(0x57) < 0) w = true;
	if (GetAsyncKeyState(0x41) < 0) a = true;
	if (GetAsyncKeyState(0x53) < 0) s = true;
	if (GetAsyncKeyState(0x44) < 0) d = true;

	if (GetAsyncKeyState(0x10) < 0) shift = true;
	if (GetAsyncKeyState(0x11) < 0) ctrl = true;

	if (GetAsyncKeyState(0x52) < 0) r = true;

	if (GetAsyncKeyState(0x57) == 0) w = false;
	if (GetAsyncKeyState(0x41) == 0) a = false;
	if (GetAsyncKeyState(0x53) == 0) s = false;
	if (GetAsyncKeyState(0x44) == 0) d = false;

	if (GetAsyncKeyState(0x10) == 0) shift = false;
	if (GetAsyncKeyState(0x11) == 0) ctrl = false;

	if (GetAsyncKeyState(0x52) == 0) r = false;
}


int main()
{
	window.setFramerateLimit(60);

	for (int x = 0; x < mwidth; x++)
	{
		terrain[x] = new char[mheight];
	}

	now = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
	last = now;
	delta = 0;
	ticks = 0;

	playtile.setSize(sf::Vector2f(play.width, play.width));
	tile.setSize(sf::Vector2f(tilesize, tilesize));

	generate();

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
		delta = now - last;
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
