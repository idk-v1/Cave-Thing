#include "SFML/Graphics.hpp"
#include <vector>
#include "Perlin.h"
#include <chrono>


const int width = 640;
const int height = 480;

sf::RenderWindow window(sf::VideoMode(width, height), "Cave Game");

sf::RectangleShape rect;
sf::RectangleShape tile;
sf::RectangleShape player;
sf::Transform trans;

int mwidth = 500;
int mheight = 500;
int tilesize = 10;
int maxll = 20;
int viewdist = 30; // Cannot be less than max light level (20)

long int now = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
long int last = now;
long int delta = 0;
long int ticks = 0;

int tps = 20;

bool w = false;
bool a = false;
bool s = false;
bool d = false;
bool shift = false;

std::vector <std::vector <int>> terrain(mwidth);
std::vector <std::vector <int>> light(mwidth);


class Player
{
public:
	double x = std::floor(mwidth / 2);
	double y = std::floor(mheight / 2);
	double vx = 0;
	double vy = 0;
	int dir = 0;
	double speed = 1;
	int width = tilesize * 3;
};

Player play;

Perlin perlin;


void lighttile(int x, int y, int level)
{

	if (x < mwidth && x >= 0 && y >= 0 && y < mheight) if (terrain[x][y] != 0) light[x][y] = level;
	if (level > 0)
	{
		if (x < mwidth && x >= 0 && y >= 0 && y < mheight) if (light[x - 1][y] < level - 1 && terrain[x - 1][y] != 0) lighttile(x - 1, y, level - 1);
		if (x < mwidth && x >= 0 && y >= 0 && y < mheight) if (light[x + 1][y] < level - 1 && terrain[x + 1][y] != 0) lighttile(x + 1, y, level - 1);
		if (x < mwidth && x >= 0 && y >= 0 && y < mheight) if (light[x][y - 1] < level - 1 && terrain[x][y - 1] != 0) lighttile(x, y - 1, level - 1);
		if (x < mwidth && x >= 0 && y >= 0 && y < mheight) if (light[x][y + 1] < level - 1 && terrain[x][y + 1] != 0) lighttile(x, y + 1, level - 1);
	}
}


void render()
{
	window.clear();
	rect.setFillColor(sf::Color(0, 0, 0));
	window.draw(rect);
	trans.translate(width / 2 - mwidth / 2 * tilesize + (mwidth / 2 - play.x) * tilesize, height / 2 - mheight / 2 * tilesize + (mheight / 2 - play.y) * tilesize);

	for (int x = 0; x < mwidth; x++)
	{
		for (int y = 0; y < mheight; y++)
		{
			double xr = std::abs(play.x - x - fmod(play.x, 1));
			double yr = std::abs(play.y - y - fmod(play.y, 1));

			if (xr <= viewdist && xr >= -viewdist && yr <= viewdist && yr >= -viewdist)
			{
				switch (terrain[x][y])
				{
				case 0:
					tile.setFillColor(sf::Color(light[x][y] * 0.05 * 32, light[x][y] * 0.05 * 32, light[x][y] * 0.05 * 32));
					break;
				case 1:
					tile.setFillColor(sf::Color(light[x][y] * 0.05 * 128, light[x][y] * 0.05 * 128, light[x][y] * 0.05 * 128));
					break;
				case 2:
					tile.setFillColor(sf::Color(light[x][y] * 0.05 * 196, light[x][y] * 0.05 * 196, light[x][y] * 0.05 * 196));
					break;
				case 3:
					tile.setFillColor(sf::Color(0, 0, light[x][y] * 0.05 * 255));
					break;
				default:
					tile.setFillColor(sf::Color(255, 0, 255));
				}
				tile.setPosition(sf::Vector2f(x * tilesize - tilesize / 2, y * tilesize - tilesize / 2));
				window.draw(tile);
			}
		}
	}

	trans = sf::Transform::Identity;

	player.setFillColor(sf::Color(0, 0, 0, 128));
	player.setPosition(width / 2 - play.width / 2, height / 2 - play.width / 2);
	window.draw(player);
}


void generate()
{
	int t = 0;
	for (int x = 0; x < mheight; x++)
	{
		for (int y = 0; y < mwidth; y++)
		{
			t++;
			double output = 0;
			output = perlin.noise(x / 10, y / 10, 0.1) + 1.01;
			terrain[x][y] = std::floor(output);
		}
	}
	terrain[std::floor(play.x) - 1][std::floor(play.y) - 1] = 1;
	terrain[std::floor(play.x) + 0][std::floor(play.y) - 1] = 1;
	terrain[std::floor(play.x) + 1][std::floor(play.y) - 1] = 1;
	terrain[std::floor(play.x) - 1][std::floor(play.y) + 0] = 1;
	terrain[std::floor(play.x) + 0][std::floor(play.y) + 0] = 1;
	terrain[std::floor(play.x) + 1][std::floor(play.y) + 0] = 1;
	terrain[std::floor(play.x) - 1][std::floor(play.y) + 1] = 1;
	terrain[std::floor(play.x) + 0][std::floor(play.y) + 1] = 1;
	terrain[std::floor(play.x) + 1][std::floor(play.y) + 1] = 1;
}


void update()
{

}


void tick()
{
	ticks++;

	play.vx -= (a * play.speed - d * play.speed) * (0.125 * shift + 1 * !shift);
	play.vy -= (w * play.speed - s * play.speed) * (0.125 * shift + 1 * !shift);

	play.x += play.vx;
	play.y += play.vy;

	play.vx *= 0.3;
	play.vy *= 0.3;

	/*
		0 = W = U
		1 = D = R
		2 = S = D
		3 = A = L
	*/

	if (w && !s) play.dir = 0;
	if (d && !a) play.dir = 1;
	if (s && !w) play.dir = 2;
	if (a && !d) play.dir = 3;

	for (int x = 0; x < mwidth; x++)
	{
		for (int y = 0; y < mheight; y++)
		{
			light[x][y] = 0;
		}
	}
	lighttile(round(play.x), round(play.y), 20);
}

int main()
{
	player.setSize(sf::Vector2f(play.width, play.width));
	tile.setSize(sf::Vector2f(tilesize, tilesize));

	rect.setPosition(0, 0);
	rect.setSize(sf::Vector2f(width, height));

	for (int x = 0; x < mwidth; x++)
	{
		for (int y = 0; y < mheight; y++)
		{
			terrain[x].push_back(0);
		}
	}

	for (int x = 0; x < mwidth; x++)
	{
		for (int y = 0; y < mheight; y++)
		{
			light[x].push_back(0);
		}
	}

	generate();

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();

			now = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
			delta = now - last;
			last = now;

			while (delta >= 1000 / tps)
			{
				delta -= 1000 / tps;
				tick();
			}

			update();
			render();

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) w = true;
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) a = true;
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) s = true;
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) d = true;

			if (!sf::Keyboard::isKeyPressed(sf::Keyboard::W)) w = false;
			if (!sf::Keyboard::isKeyPressed(sf::Keyboard::A)) a = false;
			if (!sf::Keyboard::isKeyPressed(sf::Keyboard::S)) s = false;
			if (!sf::Keyboard::isKeyPressed(sf::Keyboard::D)) d = false;
		}

	}

	return 0;
}