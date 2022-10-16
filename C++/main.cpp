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
	int x = mwidth / 2 * 10;
	int y = mheight / 2 * 10;
	int xs = mwidth / 2 * 10;
	int ys = mheight / 2 * 10;
	double vx = 0;
	double vy = 0;
	int dir = 0;
	double speed = 0.25;
	int width = tilesize * 3;
	bool cr = false;
	bool cd = false;
	bool cl = false;
	bool cu = false;
	double dt = 0;
	double ms = 1;
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
			// Check if tile is in view distance
			if (abs(play.x * 0.1 - x - fmod(play.x * 0.1, 1)) <= viewdist &&
				abs(play.x * 0.1 - x - fmod(play.x * 0.1, 1)) >= -viewdist &&
				abs(play.y * 0.1 - y - fmod(play.y * 0.1, 1)) <= viewdist &&
				abs(play.y * 0.1 - y - fmod(play.y * 0.1, 1)) >= -viewdist)
			{

				if (!((terrain[x][y] >> 0) & 1))
				{
					tile.setFillColor(sf::Color(32, 32, 32));
				}
				else 
				{
					tile.setFillColor(sf::Color(128, 128, 128));
				}

				tile.setPosition(sf::Vector2f(
					(width / 2 - mwidth * 0.5 * tilesize + (mwidth / 2 - play.x * 0.1) * tilesize) + (double(x) * tilesize - tilesize / 2.0), 
					(height / 2 - mheight * 0.5 * tilesize + (mheight / 2 - play.y * 0.1) * tilesize) + (double(y) * tilesize - tilesize / 2)));
				window.draw(tile);
			}
		}
	}

	// Draws Player
	playtile.setFillColor(sf::Color(255, 0, 0, 128));
	playtile.setPosition(int (width / 2 - play.width / 2), int (height / 2 - play.width / 2));
	window.draw(playtile);
	
	// Draws Player Eye
	tile.setFillColor(sf::Color(0, 0, 255, 128));
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
	case 4:
		tile.setPosition(int(width / 2 - tilesize / 2 + tilesize), int(height / 2 - tilesize / 2 - tilesize));
		break;
	case 5:
		tile.setPosition(int(width / 2 - tilesize / 2 + tilesize), int(height / 2 - tilesize / 2 + tilesize));
		break;
	case 6:
		tile.setPosition(int(width / 2 - tilesize / 2 - tilesize), int(height / 2 - tilesize / 2 + tilesize));
		break;
	case 7:
		tile.setPosition(int(width / 2 - tilesize / 2 - tilesize), int(height / 2 - tilesize / 2 - tilesize));
		break;
	default:
		tile.setPosition(int(width / 2 - tilesize / 2), int(height / 2 - tilesize / 2));
	}

	window.draw(tile);

	window.display();
}


void cleart(int x, int y)
{
	terrain[int(std::round(play.x * 0.1)) - x][int(std::round(play.y * 0.1)) - y] |= 1 << 0;
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

	for (int x = -1; x < 2; x++) for (int y = -1; y < 2; y++) cleart(x, y);

	switch (dir)
	{
	case 0:
		if (play.y * 0.1 - 2 <= 0)
		{
			for (int x = -1; x < 2; x++) cleart(x, -2);
		}
		break;
	case 1:
		if (play.x * 0.1 + 2 <= mwidth - 1.0)
		{
			for (int y = -1; y < 2; y++) cleart(-2, y);
		}
		break;
	case 2:
		if (play.y * 0.1 + 2 >= mheight - 1.0)
		{
			for (int x = -1; x < 2; x++) cleart(x, 2);
		}
		break;
	case 3:
		if (play.x * 0.1 - 2 >= 0)
		{
			for (int y = -1; y < 2; y++) cleart(2, y);
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
			output = perlin.noise(x * 0.025, y * 0.025, 1.1) + 1.125;
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

	// Set Velocity
	play.vx -= ((a * play.speed * !play.cl - d * play.speed * !play.cr) * !shift);
	play.vy -= ((w * play.speed * !play.cu - s * play.speed * !play.cd) * !shift);

	play.x += int (play.vx * 10);
	play.y += int (play.vy * 10);

	// Prevent Player from leaving map
	if (play.x * 0.1 + 1 >= mwidth - 1.0) play.x = mwidth - 20;
	if (play.x * 0.1 - 1 <= 0) play.x = 10;
	if (play.y * 0.1 + 1 >= mheight - 1.0) play.y = mheight - 20;
	if (play.y * 0.1 - 1 <= 0) play.y = 10;

	// Set Player Direction
	play.dir = -1;
	if (w && !d && !a) play.dir = 0;
	if (d && !s && !w) play.dir = 1;
	if (s && !d && !a) play.dir = 2;
	if (a && !w && !s) play.dir = 3;
	if (w && d) play.dir = 4;
	if (s && d) play.dir = 5;
	if (s && a) play.dir = 6;
	if (w && a) play.dir = 7;

	// Removes blocks
	if (ctrl)
	{
		play.dt += play.ms;
		if (play.dt >= 20)
		{
			clear(play.dir);
			play.dt = 0;
		}
	}
	else
	{
		play.dt = 0;
	}

	play.cr = false;
	play.cd = false;
	play.cl = false;
	play.cu = false;

	// Checks Top Collision
	if (
		(~(terrain[int(std::round(play.x * 0.1)) - 1][int(std::round(play.y * 0.1)) - 1] >> 0) & 1) ||
		(~(terrain[int(std::round(play.x * 0.1)) + 0][int(std::round(play.y * 0.1)) - 1] >> 0) & 1) ||
		(~(terrain[int(std::round(play.x * 0.1)) + 1][int(std::round(play.y * 0.1)) - 1] >> 0) & 1)
		) play.cu = true;

	// Checks Down Collision
	if (
		(~(terrain[int(std::round(play.x * 0.1)) - 1][int(std::round(play.y * 0.1)) + 1] >> 0) & 1) ||
		(~(terrain[int(std::round(play.x * 0.1)) + 0][int(std::round(play.y * 0.1)) + 1] >> 0) & 1) ||
		(~(terrain[int(std::round(play.x * 0.1)) + 1][int(std::round(play.y * 0.1)) + 1] >> 0) & 1)
		) play.cd = true;

	// Checks Left Collision
	if (
		(~(terrain[int(std::round(play.x * 0.1)) - 1][int(std::round(play.y * 0.1)) - 1] >> 0) & 1) ||
		(~(terrain[int(std::round(play.x * 0.1)) - 1][int(std::round(play.y * 0.1)) + 0] >> 0) & 1) ||
		(~(terrain[int(std::round(play.x * 0.1)) - 1][int(std::round(play.y * 0.1)) + 1] >> 0) & 1)
		) play.cl = true;

	// Checks Right Collision
	if (
		(~(terrain[int(std::round(play.x * 0.1)) + 1][int(std::round(play.y * 0.1)) - 1] >> 0) & 1) ||
		(~(terrain[int(std::round(play.x * 0.1)) + 1][int(std::round(play.y * 0.1)) + 0] >> 0) & 1) ||
		(~(terrain[int(std::round(play.x * 0.1)) + 1][int(std::round(play.y * 0.1)) + 1] >> 0) & 1)
		) play.cr = true;

	if (play.cu) play.y += 1;
	if (play.cd) play.y -= 1;
	if (play.cl) play.x += 1;
	if (play.cr) play.x -= 1;

	// Apply Friction
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

	//lighttile(round(play.x), round(play.y), 20);

	// Reset Player
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
	if (GetAsyncKeyState(0x1B) < 0) window.close();

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
