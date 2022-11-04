/*
A shitty game that is sort of like Minecraft, but I don't want to learn 3d maths.
If you are reading this, good luck. This code follows the rule of "if it works, it works".
- Ben Hamilton - Nov 2, 2022
*/

#include "SFML/Graphics.hpp"
#include <chrono>
#include <thread>
#include <windows.h>
#include <iostream>
#include <sstream>
#include "SimplexNoise.hpp"
#include <vector>
#include <functional>
#include <string>
#include <fstream>
#include <iomanip>
#include <direct.h>


int
WIDTH = 960,
HEIGHT = 720;

enum blockID
{
	air,
	stone,
	bricks,
	workBench,
	furnace,
	light,
	border,
	cobblestone,
	water,
	waterT,
	waterR,
	waterD,
	waterL,
	oreIron,
	oreGold,
	ore1,
	ore2,
	ore3,
	ore4,
};


int
tps = 1000 / 20,
tileSize = 32,
lightLevel = 16,
viewDist = 32,
mWidth = 500,
mHeight = 500,
saveCD = 0,

pageNum = 0;

long
now,
last,
delta = 0,
ticks = 0;

long long seed = 0;

double fps;

std::stringstream ss;

bool
w = false,
a = false,
s = false,
d = false,
r = false,
click = false,
clickL = false,
ctrl = false,
shift = false,
caps = false,
esc = false,
escL = false,
enter = false,
enterL = false,
regenKey = false,
lightKey = false,

inGame = false,
gen = false;

char** terrain;
char** lights;


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


enum entID
{
	zombie,
	chicken
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
		x = 2,
		y = 2,
		xStart = 2,
		yStart = 2,
		dir = 0,
		width = tileSize * 3,
		destroyTime = 0,
		mineSpeed = 0,
		speed = 1,
		health = 50,
		invNum = -1;

	Item
		inv[256];

	bool
		light = true;
};


class Button
{
public:
	int x = 0, y = 0, w = 0, h = 0;
	std::string str = "";

	Button(int x, int y, int w, int h, std::string str)
	{
		this->x = x * 15;
		this->y = y * 15;
		this->w = w * 15;
		this->h = h * 15;
		this->str = str;
	}
};

class Label
{
public:
	int x = 0, y = 0, w = 0, h = 0;
	std::string str = "";

	Label(int x, int y, int w, int h, std::string str)
	{
		this->x = x * 15;
		this->y = y * 15;
		this->w = w * 15;
		this->h = h * 15;
		this->str = str;
	}
};

class Textbox
{
public:
	int x = 0, y = 0, w = 0, h = 0;
	bool sel = false;
	std::string str = "";

	Textbox(int x, int y, int w, int h)
	{
		this->x = x * 15;
		this->y = y * 15;
		this->w = w * 15;
		this->h = h * 15;
	}
};

class Title
{
public:
	int x = 0, y = 0, w = 0, h = 0;
	std::string str = "";

	Title(int x, int y, int w, int h, std::string str)
	{
		this->x = x * 15;
		this->y = y * 15;
		this->w = w * 15;
		this->h = h * 15;
		this->str = str;
	}
};

class Image
{
public:
	int x = 0, y = 0, w = 0, h = 0;
	std::string src = "";
	sf::Texture img;

	Image(int x, int y, int w, int h, std::string src)
	{
		this->x = x * 15;
		this->y = y * 15;
		this->w = w * 15;
		this->h = h * 15;
		this->src = src;
		this->img.loadFromFile(src + "/wIcon.png");
	}
};

class Page
{
public:
	std::vector<Textbox*> tbx;
	int tbxNum = 0;
	std::vector<Button*> btn;
	int btnNum = 0;
	std::vector<Label*> lbl;
	int lblNum = 0;
	std::vector<Title*> ttl;
	int ttlNum = 0;
	std::vector<Image*> img;
	int imgNum = 0;
	int selID = -1;
};

Page* pages[4];


SimplexNoise noise;

Player
play,
def = play;


class Entity
{
public:
	int moveRange = 15,
		health = 25,
		speed = 1,
		Velo = 0,
		yVelo = 0,
		x = 250,
		y = 250,
		dir = 0,
		width = 0,
		id = 0;
	bool done = false;

	sf::Color color;
	int** path = new int* [moveRange * 2 + 3];
	int** pathTrace = new int* [moveRange * 2 + 3];
	bool isAg = false;

	Entity(int id, int x, int y)
	{
		this->x = x;
		this->y = y;
		for (int x = 0; x < moveRange * 2 + 3; x++)
		{
			path[x] = new int[moveRange * 2 + 3];
			pathTrace[x] = new int[moveRange * 2 + 3];
		}
		switch (id)
		{
		case zombie:
			this->id = zombie;
			this->speed = 2;
			this->width = tileSize;
			this->color = sf::Color(255, 128, 0);
			this->isAg = true;
			break;
		}
	}

	void PathDiamond(int xTile, int yTile, int Value)
	{
		if (Value > 0)
		{
			if (xTile >= 0 && xTile < (moveRange * 2 + 3) && yTile >= 0 && yTile < (moveRange * 2 + 3))
			{
				path[xTile][yTile] = Value;
				if (path[xTile][yTile - 1] < Value && !getID(this->x + xTile - moveRange - 1, this->y + yTile - moveRange - 1 - 1)) PathDiamond(xTile, yTile - 1, Value - 1);
				if (path[xTile + 1][yTile] < Value && !getID(this->x + xTile - moveRange - 1 + 1, this->y + yTile - moveRange - 1)) PathDiamond(xTile + 1, yTile, Value - 1);
				if (path[xTile][yTile + 1] < Value && !getID(this->x + xTile - moveRange - 1, this->y + yTile - moveRange - 1 + 1)) PathDiamond(xTile, yTile + 1, Value - 1);
				if (path[xTile - 1][yTile] < Value && !getID(this->x + xTile - moveRange - 1 - 1, this->y + yTile - moveRange - 1)) PathDiamond(xTile - 1, yTile, Value - 1);
			}
		}
	}

	void PathTrace(int xTile, int yTile, int Value)
	{
		if (Value <= moveRange)
		{
			if (xTile >= 0 && xTile < (moveRange * 2 + 3) && yTile >= 0 && yTile < (moveRange * 2 + 3) && !done)
			{
				pathTrace[xTile][yTile] = Value;
				if (!(xTile == moveRange + 1 && yTile == moveRange + 1))
				{
					if (play.x > x)
					{
						if (path[xTile + 1][yTile] > Value) PathTrace(xTile + 1, yTile, Value + 1);
						if (path[xTile][yTile - 1] > Value) PathTrace(xTile, yTile - 1, Value + 1);
						if (path[xTile][yTile + 1] > Value) PathTrace(xTile, yTile + 1, Value + 1);
						if (path[xTile - 1][yTile] > Value) PathTrace(xTile - 1, yTile, Value + 1);
					}
					else if (play.x < x)
					{
						if (path[xTile - 1][yTile] > Value) PathTrace(xTile - 1, yTile, Value + 1);
						if (path[xTile][yTile - 1] > Value) PathTrace(xTile, yTile - 1, Value + 1);
						if (path[xTile + 1][yTile] > Value) PathTrace(xTile + 1, yTile, Value + 1);
						if (path[xTile][yTile + 1] > Value) PathTrace(xTile, yTile + 1, Value + 1);
					}
					else if (play.y > y)
					{
						if (path[xTile][yTile + 1] > Value) PathTrace(xTile, yTile + 1, Value + 1);
						if (path[xTile][yTile - 1] > Value) PathTrace(xTile, yTile - 1, Value + 1);
						if (path[xTile + 1][yTile] > Value) PathTrace(xTile + 1, yTile, Value + 1);
						if (path[xTile - 1][yTile] > Value) PathTrace(xTile - 1, yTile, Value + 1);
					}
					else
					{
						if (path[xTile][yTile - 1] > Value) PathTrace(xTile, yTile - 1, Value + 1);
						if (path[xTile + 1][yTile] > Value) PathTrace(xTile + 1, yTile, Value + 1);
						if (path[xTile][yTile + 1] > Value) PathTrace(xTile, yTile + 1, Value + 1);
						if (path[xTile - 1][yTile] > Value) PathTrace(xTile - 1, yTile, Value + 1);
					}
				}
				else done = true;
				/*
					for movement compare all sides for largest then move
					otherwise it will go backwards
				*/
			}
		}
	}

	void Pathfind(int xPos, int yPos)
	{
		for (int x = 0; x < moveRange * 2 + 3; x++)
		{
			for (int y = 0; y < moveRange * 2 + 3; y++)
			{
				path[x][y] = 0;
				pathTrace[x][y] = 0;
			}
		}
		if (isAg)
		{
			done = false;
			PathDiamond(this->moveRange + 1, this->moveRange + 1, this->moveRange);
			if ((abs(xPos - this->x) + abs(yPos - this->y)) < moveRange)
				PathTrace(moveRange + 1 + (xPos - this->x), moveRange + 1 + (yPos - this->y), 0);

		}
	}
};

std::vector<Entity> ent;


sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Cave", sf::Style::Titlebar | sf::Style::Close);

sf::Font font;

sf::Text debug, worldText1, itemNum, btnTxt;

sf::Texture* tex[16];

sf::RectangleShape
frame,
button,
worldOpt1,

tile,
playTile,
invTile,
invTileItem,
minimap;

POINT mPos;

std::string wName = "";


int getLight(int xTile, int yTile)
{
	if (xTile >= 0 && xTile < mWidth && yTile >= 0 && yTile < mHeight)
		return lights[xTile][yTile];
}


void setLight(int xTile, int yTile, int Level)
{
	if (xTile >= 0 && xTile < mWidth && yTile >= 0 && yTile < mHeight)
		lights[xTile][yTile] = Level;
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


bool isBreakable(int x, int y)
{
	switch (getID(x, y))
	{
	case stone:
	case bricks:
	case workBench:
	case furnace:
	case light:
	case cobblestone:
	case oreIron:
	case oreGold:
	case ore1:
	case ore2:
	case ore3:
	case ore4:
		return true;
		break;
	default:
		return false;
	}
}


bool isSolid(int x, int y)
{
	switch (getID(x, y))
	{
	case stone:
	case bricks:
	case workBench:
	case furnace:
	case border:
	case cobblestone:
	case oreIron:
	case oreGold:
	case ore1:
	case ore2:
	case ore3:
	case ore4:
		return true;
		break;
	default:
		return false;
	}
}


bool isRemovable(int x, int y)
{
	switch (getID(x, y))
	{
	case air:
	case water:
	case waterT:
	case waterR:
	case waterD:
	case waterL:
		return true;
		break;
	default:
		return false;
	}
}


bool isLight(int x, int y)
{
	switch (getID(x, y))
	{
	case light:
	case border:
		return true;
		break;
	default:
		return false;
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

		if (Value >= 8) { Value -= 8; terrain[xTile][yTile] |= 1 << 3; }
		else terrain[xTile][yTile] |= 0 << 3;
		if (Value >= 4) { Value -= 4; terrain[xTile][yTile] |= 1 << 2; }
		else terrain[xTile][yTile] |= 0 << 2;
		if (Value >= 2) { Value -= 2; terrain[xTile][yTile] |= 1 << 1; }
		else terrain[xTile][yTile] |= 0 << 1;
		if (Value >= 1) { Value -= 1; terrain[xTile][yTile] |= 1 << 0; }
		else terrain[xTile][yTile] |= 0 << 0;
	}
}


void load(std::string name)
{
	std::cout << "Loading... ";
	wName = name.substr(7);
	std::fstream file;
	file.open(name + "/trn.txt");
	std::string data;
	if (file.is_open())
	{
		int count = 0, width = 0, height = 0;
		while (std::getline(file, data))
		{
			switch (count)
			{
			case 0:
				width = stoi(data);
				break;
			case 1:
				height = stoi(data);
				break;
			case 2:
				seed = stoi(data);
				break;
			}
			count++;
		}
		mWidth = width;
		mHeight = height;

		terrain = new char* [mWidth];
		lights = new char* [mWidth];
		for (int x = 0; x < mWidth; x++)
		{
			terrain[x] = new char[mHeight];
			lights[x] = new char[mHeight];
		}

		std::cout << "\n" << width << "\n" << height << "\n";
		for (int x = 0; x < mWidth; x++)
			for (int y = 0; y < mHeight; y++)
				setID(x, y, std::stoi(data.substr((x + y * width) * 3, 3)));
		resetLight(true);
		gen = true;
	}
	else std::cout << " Error ";
	file.close();
	std::cout << "Done!\n";
}


void save()
{
	if (_mkdir("worlds") == 0 || errno == EEXIST)
	{
		if (_mkdir(("worlds/" + wName).c_str()) == 0 || errno == EEXIST)
		{
			std::ofstream file;
			file.open("worlds/" + wName + "/trn.txt", std::ios::binary | std::ios::out);
			if (file.is_open())
			{
				file << mWidth << "\n";
				file << mHeight << "\n";
				file << seed << "\n";
				for (int y = 0; y < mHeight; y++)
				{
					for (int x = 0; x < mWidth; x++)
					{
						file << std::setw(3) << std::setfill('0') << getID(x, y);
					}
				}
				file.close();
			}

			file.open("worlds/" + wName + "/player.txt", std::ios::binary | std::ios::out);
			if (file.is_open())
			{
				file << play.xStart << "\n";
				file << play.yStart << "\n";
				file << play.x << "\n";
				file << play.y << "\n";
				file << play.xVelo << "\n";
				file << play.yVelo << "\n";
				file << play.speed << "\n";
				file << play.mineSpeed << "\n";
				file << play.dir << "\n";
				file << play.width << "\n";
				file << play.health << "\n";
				file << play.light << "\n";
				file << play.invNum << "\n";
				for (int i = 0; i < 256; i++)
				{
					file << std::setw(3) << std::setfill('0') << play.inv[i].id << std::setw(3) << std::setfill('0') << play.inv[i].count;
				}

				file.close();
			}

			std::ifstream manifest;
			manifest.open("worlds/manifest.txt");
			if (manifest.is_open())
			{
				std::string data;
				bool exists = false;
				while (std::getline(manifest, data))
				{
					if (data.substr(6) == wName)
					{
						exists = true;
					}
				}
				manifest.close();
				if (!exists)
				{
					std::ofstream man;
					man.open("worlds/manifest.txt", std::ios::app);
					man << "\n111111" << wName;
					man.close(); 
				}
			}
			else
			{
				manifest.close();
				std::ofstream man;
				man.open("worlds/manifest.txt");
				man << "111111" << wName;
				man.close();
			}
			sf::Image among;
			among.loadFromFile("res/among.png");
			among.saveToFile("worlds/" + wName + "/wIcon.png");
		}
	}
}


void generate()
{
	if (pages[1]->tbx[2]->str.length() > 0)
	{
		int width = std::stoi(pages[1]->tbx[2]->str) * 2;
		if (width > 5 && width < 32768)
			mWidth = width;
	}
	if (pages[1]->tbx[3]->str.length() > 0)
	{
		int height = std::stoi(pages[1]->tbx[3]->str) * 2;
		if (height > 5 && height < 32768)
			mHeight = height;
	}
	terrain = new char* [mWidth];
	lights = new char* [mWidth];
	for (int x = 0; x < mWidth; x++)
	{
		terrain[x] = new char[mHeight];
		lights[x] = new char[mHeight];
	}

	int r = std::hash<time_t>{}(time(nullptr));
	noise.setSeed(r);
	seed = r;
	if (pages[1]->tbx[1]->str.length() > 4 && pages[1]->tbx[1]->str.length() < 11)
	{
		bool isnum = true;
		for (int i = 0; i < pages[1]->tbx[1]->str.length(); i++)
			if (!isdigit(pages[1]->tbx[1]->str[i])) isnum = false;
		if (isnum)
			seed = std::stol(pages[1]->tbx[1]->str);
		noise.setSeed(seed);
	}

	for (int x = 0; x < mWidth; x++)
	{
		for (int y = 0; y < mHeight; y++)
		{
			if (!std::floor(noise.signedFBM(x * 0.025, y * 0.025, 1.4, 0.1, 1.4) + 1)) setID(x, y, air);
			else setID(x, y, stone);
			if (x == 0 || x == mWidth - 1 || y == 0 || y == mHeight - 1) setID(x, y, border);
		}
	}
	resetLight(true);
	for (int x = -1; x < 2; x++)
		for (int y = -1; y < 2; y++)
			setID(play.x - x, play.y - y, air);
	gen = true;
	save();
}


void render()
{
	double light;
	window.clear();

	// Draws Tiles Start
	for (int x = play.x - viewDist; x < play.x + viewDist; x++)
	{
		for (int y = play.y - viewDist; y < play.y + viewDist; y++)
		{
			if (x >= 0 && x < mWidth && y >= 0 && y < mHeight)
			{
				light = std::ceil((getLight(x, y) + 1)) * (1 / 16.0);
				tile.setFillColor(sf::Color(std::ceil(255 * light), std::ceil(224 * light), std::ceil(128 * light)));
				tile.setTexture(tex[getID(x, y)]);
				tile.setPosition(sf::Vector2f((WIDTH * 0.5 - mWidth * 0.5 * tileSize + (mWidth / 2 - play.x) * tileSize) + (x * tileSize - tileSize * 0.5), (HEIGHT * 0.5 - mHeight * 0.5 * tileSize + (mHeight / 2 - play.y) * tileSize) + (y * tileSize - tileSize * 0.5)));
				window.draw(tile);
			}
		}
	}
	// Draws Tiles End

	// Draws Entities Start
	for (int i = 0; i < ent.size(); i++)
	{
		for (int x = 0; x < ent[i].moveRange * 2 + 3; x++)
		{
			for (int y = 0; y < ent[i].moveRange * 2 + 3; y++)
			{
				if (ent[i].pathTrace[x][y])
				{
					tile.setTexture(tex[0]);
					tile.setPosition(sf::Vector2f((WIDTH * 0.5 - mWidth * 0.5 * tileSize + (mWidth / 2 - play.x) * tileSize) + ((x + ent[i].x - ent[i].moveRange / 2 + 1 - std::floor(ent[i].moveRange / 5) * 3 - 1) * tileSize - tileSize * 0.5), (HEIGHT * 0.5 - mHeight * 0.5 * tileSize + (mHeight / 2 - play.y) * tileSize) + ((y + ent[i].y - ent[i].moveRange / 2 + 1 - std::floor(ent[i].moveRange / 5) * 3 - 1)) * tileSize - tileSize * 0.5));
					if (ent[i].pathTrace[x][y]) tile.setFillColor(sf::Color(0, 255 * (ent[i].pathTrace[x][y] * (1.0 / ent[i].moveRange)), 0));
					window.draw(tile);
				}
			}
		}
	}

	for (int i = 0; i < ent.size(); i++)
	{
		if (ent[i].x > play.x - viewDist && ent[i].x < play.x + viewDist && ent[i].y > play.y - viewDist && ent[i].y < play.y + viewDist)
		{
			tile.setTexture(tex[0]);
			for (int x = -std::floor(ent[i].width / 2); x < std::floor(ent[i].width / 2); x++)
				for (int y = -std::floor(ent[i].width / 2); y < std::floor(ent[i].width / 2); y++)
					tile.setPosition(sf::Vector2f((WIDTH * 0.5 - mWidth * 0.5 * tileSize + (mWidth / 2 - play.x) * tileSize) + ((ent[i].x + x - (tileSize / 2 - 1)) * tileSize - tileSize * 0.5), (HEIGHT * 0.5 - mHeight * 0.5 * tileSize + (mHeight / 2 - play.y) * tileSize) + ((ent[i].y + y - (tileSize / 2 - 1)) * tileSize - tileSize * 0.5)));
			tile.setFillColor(ent[i].color);
			window.draw(tile);
		}
	}

	// Draws Entities End

	// Draws Player Start
	light = std::ceil((getLight(play.x, play.y) + 1) / 2.0) * 2.0 / 16.0;
	playTile.setFillColor(sf::Color(64 * light, 192 * light, 224 * light));
	playTile.setPosition(WIDTH / 2 - play.width / 2, HEIGHT / 2 - play.width / 2);
	window.draw(playTile);

	int xOff = 0;
	int yOff = 0;
	int pd = play.dir;
	if (pd == 7 || pd == 0 || pd == 1) yOff--;
	if (pd == 1 || pd == 2 || pd == 3) xOff++;
	if (pd == 3 || pd == 4 || pd == 5) yOff++;
	if (pd == 5 || pd == 6 || pd == 7) xOff--;

	tile.setPosition(WIDTH / 2 - tileSize / 2 + tileSize * xOff, HEIGHT / 2 - tileSize / 2 + tileSize * yOff);
	tile.setFillColor(sf::Color(96, 224, 255));
	tile.setTexture(tex[0]);
	window.draw(tile);

	if (play.dir == 0 || play.dir == 2 || play.dir == 4 || play.dir == 6)
	{
		if (caps)
		{
			playTile.setPosition(WIDTH / 2 - tileSize / 2 + play.width * xOff - tileSize, HEIGHT / 2 - tileSize / 2 + play.width * yOff - tileSize);
			playTile.setFillColor(sf::Color(255, 0, 0, 128 * play.destroyTime * 0.1));
			window.draw(playTile);
		}
		else
		{
			tile.setPosition(WIDTH / 2 - tileSize / 2 + 2 * tileSize * xOff, HEIGHT / 2 - tileSize / 2 + 2 * tileSize * yOff);
			tile.setFillColor(sf::Color(255, 0, 0, 128 * play.destroyTime * 0.1));
			window.draw(tile);
		}
	}
	// Draws Player End

	// Draws Inventory Start
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

		itemNum.setPosition(WIDTH / 2 - (i - 4) * 55 - 16, HEIGHT - 75 - 2);
		ss.str("");
		ss << play.inv[8 - i].count;
		itemNum.setString(ss.str());
		window.draw(itemNum);
	}
	// Draws Inventory End

	// UNCOMMENT FOR MINIMAP
	for (int x = play.x - viewDist * 0.5; x < play.x + viewDist * 0.5; x++)
	{
		for (int y = play.y - viewDist * 0.5; y < play.y + viewDist * 0.5; y++)
		{
			minimap.setPosition((viewDist * 0.125 + 0.5 - (viewDist * 2 + 1) + ((viewDist + 1) - play.x) * 4) + (x * 4 - 1), (viewDist * 0.125 + 0.5 - (viewDist * 2 + 1) + ((viewDist + 1) - play.y) * 4) + (y * 4 - 1));
			if (!isSolid(x, y)) minimap.setFillColor(sf::Color(64, 64, 64));
			else minimap.setFillColor(sf::Color(32, 32, 32));
			window.draw(minimap);
		}
	}
	minimap.setFillColor(sf::Color(96, 224, 255));
	for (int x = -1; x < 2; x++)
		for (int y = -1; y < 2; y++)
		{
			minimap.setPosition(viewDist * 2 + 5 + x * 4, viewDist * 2 + 5 + y * 4);
			window.draw(minimap);
		}
		// END UNCOMMENT

	ss.str("");
	ss << "X: " << play.x << "\nY: " << play.y << "\nSeed: " << seed << "\nMap W: " << mWidth << "\nMap H: " << mHeight << "\nBuild Mode: " << caps * 2 + 1 << "x" << caps * 2 + 1;
	ss << "\n" << (saveCD > 0 ? "Saving..." : "Saved");
	debug.setString(ss.str());
	window.draw(debug);

	window.display();
}


void key()
{
	// "0xnn < 0" Means Key is Down
	// "0xnn == 0" Means Key is Up

	if (inGame)
	{
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

		if (GetKeyState(0x14) < 0) caps = true;

		if (GetAsyncKeyState(0x52) < 0) r = true;

		if (GetAsyncKeyState(0x57) == 0) w = false;
		if (GetAsyncKeyState(0x41) == 0) a = false;
		if (GetAsyncKeyState(0x53) == 0) s = false;
		if (GetAsyncKeyState(0x44) == 0) d = false;

		if (GetAsyncKeyState(0x10) == 0) shift = false;
		if (GetAsyncKeyState(0x11) == 0) ctrl = false;

		if (GetKeyState(0x14) == 0) caps = false;

		if (GetAsyncKeyState(0x52) == 0) r = false;

		enterL = enter;
		if (GetAsyncKeyState(0x0D) < 0) enter = true;
		if (GetAsyncKeyState(0x0D) == 0) enter = false;
	}

	clickL = click;
	if (GetAsyncKeyState(0x01) < 0) click = true;
	if (GetAsyncKeyState(0x01) == 0) click = false;

	escL = esc;
	if (GetAsyncKeyState(0x1B) < 0) esc = true;
	if (GetAsyncKeyState(0x1B) == 0) esc = false;

	GetCursorPos(&mPos);
	ScreenToClient(window.getSystemHandle(), &mPos);
}


void lightDiamond(int xTile, int yTile, int Value)
{
	if (Value > 0)
	{
		if (xTile >= 0 && xTile < mWidth && yTile >= 0 && yTile < mHeight)
		{
			setLight(xTile, yTile, Value);
			if (Value > getLight(xTile, yTile - 1) && !isSolid(xTile, yTile)) lightDiamond(xTile, yTile - 1, Value - 1);
			if (Value > getLight(xTile + 1, yTile) && !isSolid(xTile, yTile)) lightDiamond(xTile + 1, yTile, Value - 1);
			if (Value > getLight(xTile, yTile + 1) && !isSolid(xTile, yTile)) lightDiamond(xTile, yTile + 1, Value - 1);
			if (Value > getLight(xTile - 1, yTile) && !isSolid(xTile, yTile)) lightDiamond(xTile - 1, yTile, Value - 1);
		}
	}
}


void tick()
{
	if (ticks % 3 == 0)
	{
		ent[0].Pathfind(play.x, play.y);
	}

	if (saveCD > 0)
		saveCD--;

	if (enter && saveCD == 0)
	{
		save();
		saveCD = 25;
	}

	play.xVelo -= (a * play.speed - d * play.speed);
	play.yVelo -= (w * play.speed - s * play.speed);

	for (int i = 0; i < 256; i++)
	{
		if (play.inv[i].count == 0)
		{
			play.inv[i].id = 0;
		}
	}

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

			if (placeOk)
			{
				int itemNum = play.invNum;
				if (play.invNum == -1) itemNum = 255;

				if (!caps)
				{
					if (play.inv[itemNum].count > 0 || play.inv[itemNum].id == air)
					{
						if (isRemovable(play.x + 2 * xDir, play.y + 2 * yDir))
						{
							setID(play.x + 2 * xDir, play.y + 2 * yDir, play.inv[itemNum].id);
							if (play.inv[itemNum].id != air)
							{
								play.inv[itemNum].count--;
							}
						}
						else if (isBreakable(play.x + 2 * xDir, play.y + 2 * yDir) && play.inv[itemNum].id == air)
						{
							int searchID = getID(play.x + 2 * xDir, play.y + 2 * yDir);
							setID(play.x + 2 * xDir, play.y + 2 * yDir, air);

							bool foundItem = false;
							for (int i = 0; i < 256; i++)
							{
								if (play.inv[i].id == searchID && play.inv[i].count < 450 && !foundItem)
								{
									play.inv[i].count++;
									foundItem = true;
								}
							}
							for (int i = 0; i < 256; i++)
							{
								if (play.inv[i].id == air && !foundItem)
								{
									play.inv[i].id = searchID;
									play.inv[i].count++;
									foundItem = true;
								}
							}
						}
					}
				}
				else
				{
					if (play.inv[itemNum].count > 8 || play.inv[itemNum].id == air)
					{
						for (int x = -1; x < 2; x++)
						{
							for (int y = -1; y < 2; y++)
							{
								if (isRemovable(play.x - x + 3 * xDir, play.y - y + 3 * yDir))
								{
									setID(play.x - x + 3 * xDir, play.y - y + 3 * yDir, play.inv[itemNum].id);
									if (play.inv[itemNum].id != air)
									{
										play.inv[itemNum].count--;
									}
								}
								else if (isBreakable(play.x - x + 3 * xDir, play.y - y + 3 * yDir) && play.inv[itemNum].id == air)
								{
									int searchID = getID(play.x - x + 3 * xDir, play.y - y + 3 * yDir);
									setID(play.x - x + 3 * xDir, play.y - y + 3 * yDir, air);

									bool foundItem = false;
									for (int i = 0; i < 256; i++)
									{
										if (play.inv[i].id == searchID && play.inv[i].count < 450 && !foundItem)
										{
											play.inv[i].count++;
											foundItem = true;
										}
									}
									for (int i = 0; i < 256; i++)
									{
										if (play.inv[i].id == air && !foundItem)
										{
											play.inv[i].id = searchID;
											play.inv[i].count++;
											foundItem = true;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	else play.destroyTime = 0;

	if (shift && !lightKey) play.light = !play.light;
	lightKey = shift;

	if (play.xVelo > 0)
		for (int x = 0; x < play.xVelo; x++)
			if (play.x + 2 < mWidth)
				if (!isSolid(play.x + 2, play.y - 1) && !isSolid(play.x + 2, play.y) && !isSolid(play.x + 2, play.y + 1))
					play.x += play.speed;
	if (play.xVelo < 0)
		for (int x = 0; x > play.xVelo; x--)
			if (play.x - 1 > 0)
				if (!isSolid(play.x - 2, play.y - 1) && !isSolid(play.x - 2, play.y) && !isSolid(play.x - 2, play.y + 1))
					play.x -= play.speed;
	if (play.yVelo > 0)
		for (int y = 0; y < play.yVelo; y++)
			if (play.y + 1 <= mHeight)
				if (!isSolid(play.x - 1, play.y + 2) && !isSolid(play.x, play.y + 2) && !isSolid(play.x + 1, play.y + 2))
					play.y += play.speed;
	if (play.yVelo < 0)
		for (int y = 0; y > play.yVelo; y--)
			if (play.y - 1 > 0)
				if (!isSolid(play.x - 1, play.y - 2) && !isSolid(play.x, play.y - 2) && !isSolid(play.x + 1, play.y - 2))
					play.y -= play.speed;

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
		play = def;
		if (shift)
		{
			noise.randomizeSeed();
			generate();
		}
	}
	regenKey = r;

	for (int x = play.x - viewDist * 2; x < play.x + viewDist * 2; x++)
	{
		for (int y = play.y - viewDist * 2; y < play.y + viewDist * 2; y++)
		{
			setLight(x, y, 0);
		}
	}

	if (play.light) lightDiamond(play.x, play.y, 15);

	for (int x = play.x - viewDist - 15; x < play.x + viewDist + 15; x++)
		for (int y = play.y - viewDist - 15; y < play.y + viewDist + 15; y++)
			if (isLight(x, y)) lightDiamond(x, y, 12);
}


void rMenu()
{
	window.clear();

	frame.setFillColor(sf::Color(20, 40, 80));
	window.draw(frame);

	int i = pageNum;
	// Buttons
	for (int e = 0; e < pages[i]->btnNum; e++)
	{
		button.setPosition(pages[i]->btn[e]->x, pages[i]->btn[e]->y);
		button.setSize(sf::Vector2f(pages[i]->btn[e]->w, pages[i]->btn[e]->h));

		btnTxt.setCharacterSize(30);
		btnTxt.setPosition(pages[i]->btn[e]->x + 0.5 * pages[i]->btn[e]->w - 9 * pages[i]->btn[e]->str.length(), pages[i]->btn[e]->y + 0.5 * pages[i]->btn[e]->h - 20);
		btnTxt.setString(pages[i]->btn[e]->str);

		if (mPos.x >= pages[i]->btn[e]->x && mPos.x < pages[i]->btn[e]->x + pages[i]->btn[e]->w &&
			mPos.y >= pages[i]->btn[e]->y && mPos.y < pages[i]->btn[e]->y + pages[i]->btn[e]->h)
		{
			button.setFillColor(sf::Color(96, 96, 96));
			if (!click && clickL)
			{
				switch (pageNum)
				{
				case 0: // Main Menu ***********************
					switch (e)
					{
					case 0: // Play
						pageNum = 3;
						break;
					case 1: // Quit
						window.close();
						break;
					case 2: // Settings
						pageNum = 2;
						break;
					}
					break; // *********************************
				case 1: // Create World ***********************
					switch (e)
					{
					case 0: // Back
						pageNum = 3;
						for (int e = 0; e < pages[i]->tbxNum; e++)
						{
							pages[i]->tbx[e]->str = "";
						}
						pages[i]->selID = -1;
						break;
					case 1: // Create
						if (pages[i]->tbx[0]->str.length() > 0)
						{
							inGame = true;
							wName = pages[i]->tbx[0]->str;
							generate();
						}
						break;
					}
					break; // *********************************
				case 2: // Settings ***************************
					switch (e)
					{
					case 0: // Back
						pageNum = 0;
						for (int e = 0; e < pages[i]->tbxNum; e++)
						{
							pages[i]->tbx[e]->str = "";
						}
						pages[i]->selID = -1;
						break;
					}
					break; // *********************************
				case 3: // Worlds List ************************
					switch (e)
					{
					case 0: // Back
						pageNum = 0;
						break;
					case 1: // New World
						pageNum = 1;
						break;
					default:
						load(pages[i]->img[e - 2]->src);
						//wName = pages[i]->img[e]->src.substr(7);
						inGame = true;
					}
					break; // **********************************
				}
			}
		}
		else
			button.setFillColor(sf::Color(64, 64, 64));

		window.draw(button);
		window.draw(btnTxt);
	}

	// Titles
	for (int e = 0; e < pages[i]->ttlNum; e++)
	{
		double charSz = 85 + 15.0 * std::sin(ticks / 4.0);
		btnTxt.setCharacterSize(charSz);
		btnTxt.setPosition(pages[i]->ttl[e]->x + 0.5 * pages[i]->ttl[e]->w - (charSz * 0.3) * pages[i]->ttl[e]->str.length(), pages[i]->ttl[e]->y + 0.5 * pages[i]->ttl[e]->h - (charSz * 0.667));
		btnTxt.setString(pages[i]->ttl[e]->str);

		window.draw(btnTxt);
	}

	// Labels
	for (int e = 0; e < pages[i]->lblNum; e++)
	{
		button.setPosition(pages[i]->lbl[e]->x, pages[i]->lbl[e]->y);
		button.setSize(sf::Vector2f(pages[i]->lbl[e]->w, pages[i]->lbl[e]->h));
		button.setFillColor(sf::Color(64, 64, 64));

		btnTxt.setCharacterSize(30);
		btnTxt.setPosition(pages[i]->lbl[e]->x + 0.5 * pages[i]->lbl[e]->w - 9 * pages[i]->lbl[e]->str.length(), pages[i]->lbl[e]->y + 0.5 * pages[i]->lbl[e]->h - 20);
		btnTxt.setString(pages[i]->lbl[e]->str);

		window.draw(button);
		window.draw(btnTxt);
	}

	// Textboxes
	for (int e = 0; e < pages[i]->tbxNum; e++)
	{
		button.setPosition(pages[i]->tbx[e]->x, pages[i]->tbx[e]->y);
		button.setSize(sf::Vector2f(pages[i]->tbx[e]->w, pages[i]->tbx[e]->h));

		btnTxt.setCharacterSize(30);
		btnTxt.setPosition(pages[i]->tbx[e]->x + 0.5 * pages[i]->tbx[e]->w - 9 * pages[i]->tbx[e]->str.length(), pages[i]->tbx[e]->y + 0.5 * pages[i]->tbx[e]->h - 20);
		btnTxt.setString(pages[i]->tbx[e]->str);

		if ((mPos.x >= pages[i]->tbx[e]->x && mPos.x < pages[i]->tbx[e]->x + pages[i]->tbx[e]->w &&
			mPos.y >= pages[i]->tbx[e]->y && mPos.y < pages[i]->tbx[e]->y + pages[i]->tbx[e]->h) || pages[i]->selID == e)
		{
			button.setFillColor(sf::Color(96, 96, 96));
			if (!click && clickL)
			{
				pages[i]->selID = e;
			}
		}
		else button.setFillColor(sf::Color(64, 64, 64));

		window.draw(button);
		window.draw(btnTxt);
	}

	// Images
	for (int e = 0; e < pages[i]->imgNum; e++)
	{
		button.setFillColor(sf::Color(255, 255, 255));
		button.setTexture(&pages[i]->img[e]->img);
		button.setPosition(pages[i]->img[e]->x, pages[i]->img[e]->y);
		button.setSize(sf::Vector2f(pages[i]->img[e]->w, pages[i]->img[e]->h));
		window.draw(button);
		button.setTexture(NULL);
	}

	window.display();
}


int main()
{
	window.setPosition(sf::Vector2i(window.getPosition().x, 0));
	window.setFramerateLimit(60);

	font.loadFromFile("res/cas.ttf");
	debug.setFont(font);
	debug.setPosition(viewDist / 2, (viewDist + 3) * 4);

	itemNum.setCharacterSize(10);
	itemNum.setFont(font);

	tex[0] = new sf::Texture; tex[0]->loadFromFile("res/blocks/air.png");
	tex[1] = new sf::Texture; tex[1]->loadFromFile("res/blocks/stone.png");
	tex[2] = new sf::Texture; tex[2]->loadFromFile("res/blocks/bricks.png");
	tex[3] = new sf::Texture; tex[3]->loadFromFile("res/blocks/workbench.png");
	tex[4] = new sf::Texture; tex[4]->loadFromFile("res/blocks/furnace.png");
	tex[5] = new sf::Texture; tex[5]->loadFromFile("res/blocks/light.png");
	tex[6] = new sf::Texture; tex[6]->loadFromFile("res/blocks/border.png");

	// Set Player Inventory - Change Later
	play.inv[1].count = 100; play.inv[1].id = 5;
	play.inv[0].count = 100; play.inv[0].id = 2;
	play.inv[2].count = 100; play.inv[2].id = 3;
	play.inv[3].count = 100; play.inv[3].id = 4;
	// ***********************************

	enum blockID
	{
		air,
		stone,
		bricks,
		workBench,
		furnace,
		light,
		border,
		cobblestone,
		water,
		waterT,
		waterR,
		waterD,
		waterL,
		oreIron,
		oreGold,
		ore1,
		ore2,
		ore3,
		ore4,
	};

	playTile.setSize(sf::Vector2f(play.width, play.width));
	tile.setSize(sf::Vector2f(tileSize, tileSize));
	invTile.setSize(sf::Vector2f(50, 50));
	invTileItem.setSize(sf::Vector2f(32, 32));
	minimap.setSize(sf::Vector2f(4, 4));
	frame.setSize(sf::Vector2f(WIDTH, HEIGHT));
	frame.setPosition(0, 0);

	btnTxt.setFillColor(sf::Color(255, 255, 255));
	btnTxt.setFont(font);


	pages[0] = new Page;

	pages[0]->btn.push_back(new Button(18, 22, 28, 9, "PLAY"));
	pages[0]->btn.push_back(new Button(18, 33, 13, 9, "QUIT"));
	pages[0]->btn.push_back(new Button(33, 33, 13, 9, "SETTINGS"));
	pages[0]->btnNum = 3;

	pages[0]->ttl.push_back(new Title(0, 6, 64, 8, "CAVE TEST"));
	pages[0]->ttlNum = 1;


	pages[1] = new Page;

	pages[1]->btn.push_back(new Button(1, 41, 7, 5, "BACK"));
	pages[1]->btn.push_back(new Button(43, 18, 8, 5, "CREATE"));
	pages[1]->btnNum = 2;

	pages[1]->lbl.push_back(new Label(10, 11, 13, 5, "WORLD NAME"));
	pages[1]->lbl.push_back(new Label(10, 18, 13, 5, "WORLD SEED"));
	pages[1]->lbl.push_back(new Label(10, 25, 15, 5, "MAP RADIUS X"));
	pages[1]->lbl.push_back(new Label(10, 32, 15, 5, "MAP RADIUS Y"));
	pages[1]->lblNum = 4;

	pages[1]->tbx.push_back(new Textbox(25, 11, 26, 5));
	pages[1]->tbx.push_back(new Textbox(25, 18, 16, 5));
	pages[1]->tbx.push_back(new Textbox(27, 25, 10, 5));
	pages[1]->tbx.push_back(new Textbox(27, 32, 10, 5));
	pages[1]->tbxNum = 4;


	pages[2] = new Page;

	pages[2]->btn.push_back(new Button(1, 41, 7, 5, "BACK"));
	pages[2]->btn.push_back(new Button(10, 41, 7, 5, "SAVE"));
	pages[2]->btnNum = 2;

	
	pages[3] = new Page;

	pages[3]->btn.push_back(new Button(1, 41, 7, 5, "BACK"));
	pages[3]->btn.push_back(new Button(18, 5, 28, 9, "NEW WORLD"));

	std::fstream manifest;
	std::string worlds;
	manifest.open("worlds/manifest.txt");
	int count = 0;
	if (manifest.is_open())
	{
		while (std::getline(manifest, worlds))
		{
			std::string name = worlds.substr(6);
			std::string month = worlds.substr(0, 2);
			std::string day = worlds.substr(2, 2);
			std::string year = worlds.substr(4, 2);
			pages[3]->btn.push_back(new Button(18, 16 + count * 11, 18, 9, "\t" + name + "\n\t" + month + "/" + day + "/" + year));
			pages[3]->img.push_back(new Image(37, 16 + count * 11, 9, 9, "worlds/" + name));
			count++;
		}
	}

	pages[3]->btnNum = 2 + count;

	pages[3]->imgNum = count;

	debug.setFillColor(sf::Color(255, 255, 255));
	debug.setCharacterSize(15);

	ent.push_back(Entity(zombie, 8, 8));

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

			if (event.type == sf::Event::TextEntered)
			{
				if (!inGame)
				{
					char c = static_cast<char>(event.text.unicode);
					if (pages[pageNum]->selID != -1)
					{
						if (c == '\b' && pages[pageNum]->tbx[pages[pageNum]->selID]->str.length() > 0)
							pages[pageNum]->tbx[pages[pageNum]->selID]->str.pop_back();
						else if (c == '\r')
							pages[pageNum]->selID = -1;
						else if (c == '\t')
							pages[pageNum]->selID = (pages[pageNum]->selID + 1) % pages[pageNum]->tbxNum;
						else if (isprint(c))
							pages[pageNum]->tbx[pages[pageNum]->selID]->str.push_back(c);
					}
				}
			}
		}

		now = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
		delta += now - last;
		if (ticks % 5 == 0) fps = 1000.0 / ((now - last));
		last = now;

		while (delta >= tps)
		{
			delta -= tps;
			ticks++;
			if (gen) tick();
		}

		key();

		if (inGame) render();
		else rMenu();
	}

	return 0;
}
