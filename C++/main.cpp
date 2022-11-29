/*
A shitty game that is sort of like Minecraft from a top down view, but I don't want to learn 3d maths.
If you are reading this, good luck. This code follows the rule of "if it works, it works".
- Ben Hamilton - Nov 28, 2022
Cleaned up
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


SimplexNoise noise;

struct Item
{
	int id = 0,
		count = 0;
	Item(int idp, int countp)
	{
		id = idp;
		count = countp;
	}
};

struct Tile
{
	int id = 0,
		grow = 0,
		light = 0;
};
sf::Texture* tex[256];

struct InvElement
{
	int x = 0, y = 0, w = 0, h = 0,
		id = 0, count = 0;
	InvElement(double xp, double yp, double wp, double hp)
	{
		x = int(xp); y = int(yp);
		w = int(wp); h = int(hp);
	}
};
std::vector<InvElement*>invEle;
std::vector<InvElement*>guiEle;


POINT mpos;


std::vector<int>settings;
std::vector<int>settingsid;
std::vector<int>tmpsettings;
std::vector<int>tmpsettingsid;

int width = 960, height = 720, mwidth = 500, mheight = 500,
	tps = 1000 / 20,
	tilesize = 32, viewdist = 1, loaddist = 3,
	pagenum = 0, invsel = -1,
	sfx = 15, sfy = 15;

long now = 0, last = 0, delta = 0, ticks = 0, ticksS = 0, seed;

bool 
	w = false, wL = false,
	a = false, aL = false,
	s = false, sL = false,
	d = false, dL = false,

	q = false, qL = false,
	e = false, eL = false,
	r = false, rL = false,
	l = false, lL = false,

	esc = false, escL = false,
	ctrl = false, ctrlL = false,
	shift = false, shiftL = false,
	enter = false, enterL = false,

	one = false, two = false, thr = false,
	fou = false, fiv = false, six = false,
	sev = false, eig = false, nin = false,
	zer = false,

	left = false, leftL = false,
	right = false, rightL = false,

	focus = true, ininv = false, ingame = false, pause = false;

struct BTN
{
	int x = 0, y = 0, w = 0, h = 0,
		fs = 0, evt = 0;
	std::string str = "";
	BTN(int xp, int yp, int wp, int hp, int fsp, int evtp, std::string strp)
	{
		x = xp; y = yp; w = wp; h = hp;
		fs = fsp; evt = evtp; 
		str = strp;
	}
};

struct TTL
{
	int x = 0, y = 0, w = 0, h = 0,
		fs = 0;
	std::string str = "";
	TTL(int xp, int yp, int wp, int hp, int fsp, std::string strp)
	{
		x = xp;	y = yp;	w = wp; h = hp;
		fs = fsp;
		str = strp;
	}
};

struct LBL
{
	int x = 0, y = 0, w = 0, h = 0,
		fs = 0;
	std::string str = "";
	LBL(int xp, int yp, int wp, int hp, int fsp, std::string strp)
	{
		x = xp; y = yp; w = wp; h = hp;
		fs = fsp;
		str = strp;
	}
};

struct SLD
{
	int x = 0, y = 0, w = 0, h = 0,
		fs = 0,
		val = 0,
		id = 0;
	std::string str = "";
	SLD(int xp, int yp, int wp, int hp, int fsp, int valp, int idp, std::string strp)
	{
		x = xp; y = yp; w = wp; h = hp;
		fs = fsp; val = valp; id = idp;
		str = strp;
	}
};


struct PAGE
{
	std::vector<BTN*>btn;
	std::vector<TTL*>ttl;
	std::vector<LBL*>lbl;
	std::vector<SLD*>sld;
	//std::vector<>;
	//std::vector<>;
	int sel = 0;
};
std::vector<PAGE*>Pages;


struct Chunk
{
	int x = 0, y = 0;
	Tile** tiles[16];
	Tile** tilesf[16];
	Chunk(int xp, int yp)
	{
		x = xp; y = yp;

		for (int x = 0; x < 16; x++)
		{
			tiles[x] = new Tile*[16];
			tilesf[x] = new Tile*[16];
			for (int y = 0; y < 16; y++)
			{
				tiles[x][y] = new Tile;
				tilesf[x][y] = new Tile;
			}
		}
	}
};
std::vector<Chunk*> chunks;


struct Player
{
	int x = 0, y = 0,
		xc = 0, yc = 0,
		xv = 0, yv = 0,
		rad = tilesize * 3, rot = 0,
		mode = 0,
		mineprog = 0, minedist = 5, minedistcur = 0, minespeed = 1,
		invsel = 0, health = 50, energy = 50, enrch = 0, damage = 10, speed = 1;
	bool light = true;
	Item* inv[72];
	Player()
	{
		for (int i = 0; i < 72; i++)
			inv[i] = new Item(0, 0);
	}
};
Player p;


sf::RenderWindow window(sf::VideoMode(width, height), "Cave");
sf::RectangleShape rect;
sf::ConvexShape cnvx;
sf::Texture texture;
sf::Font font;
sf::Text text;


void save();
void load();
void addInv(int);
int getChunkID(int, int, int, int);
int getTileID(int, int, int, int);
void setTileID(int, int, int, int, int);
int getTileLight(int, int, int, int);
void setTileLight(int, int, int, int, int);
void lightDiamond(int, int, int, int, int);
void generate(int, int);
void initGen();
void renderTiles();
void renderPlayer();
void loadGUI();
void getkey();
void updateKey();
void render();
void rmenu();
void update();


void render()
{
	window.clear();

#pragma warning(disable:4244)
	rect.setSize(sf::Vector2f(tilesize, tilesize));
	rect.setOutlineThickness(0);
	renderTiles();

	renderPlayer();

	if (ininv)
	{
		rect.setFillColor(sf::Color(0, 0, 0, 128));
		rect.setSize(sf::Vector2f(width, height));
		rect.setPosition(0, 0);
		rect.setTexture(tex[0]);
		window.draw(rect);
	}

	for (int i = 0; i < invEle.size(); i++)
	{
		rect.setTexture(tex[0]);
		if (i < 12)
		{
			if (p.invsel == i || p.mode + 9 == i)
			{
				rect.setSize(sf::Vector2f(invEle[i]->w * sfy + sfy * 0.66, invEle[i]->h * sfy + sfy * 0.66));
				rect.setPosition(invEle[i]->x * sfy - sfy * 0.33, invEle[i]->y * sfy - sfy * 0.33);
			}
			else
			{
				rect.setSize(sf::Vector2f(invEle[i]->w * sfy, invEle[i]->h * sfy));
				rect.setPosition(invEle[i]->x * sfy, invEle[i]->y * sfy);
			}
			rect.setFillColor(sf::Color(128, 128, 128));
			window.draw(rect);

			rect.setFillColor(sf::Color(64, 64, 64));
			if (p.invsel == i)
			{
				rect.setSize(sf::Vector2f(invEle[i]->w * sfy, invEle[i]->h * sfy));
				rect.setPosition(invEle[i]->x * sfy, invEle[i]->y * sfy);
			}
			else
			{
				rect.setSize(sf::Vector2f(invEle[i]->w * sfy - sfy * 0.66, invEle[i]->h * sfy - sfy * 0.66));
				rect.setPosition(invEle[i]->x * sfy + sfy * 0.33, invEle[i]->y * sfy + sfy * 0.33);
			}
			window.draw(rect);

			if (p.inv[i]->id != 0)
			{
				rect.setOutlineThickness(0);
				rect.setTexture(tex[p.inv[i]->id]);
				rect.setFillColor(sf::Color(255, 255, 255));
				window.draw(rect);
				text.setPosition(invEle[i]->x * sfy + sfy * 0.33, invEle[i]->y * sfy + sfy * 0.33);
				text.setString(std::to_string(p.inv[i]->count));
				text.setCharacterSize(10);
				text.setFillColor(sf::Color(255, 255, 255));
				window.draw(text);
			}
		}
	}

	for (int i = 0; i < guiEle.size(); i++)
	{
		rect.setFillColor(sf::Color(128, 128, 128));
		rect.setSize(sf::Vector2f(guiEle[i]->w * sfy, guiEle[i]->h * sfy));
		rect.setPosition(guiEle[i]->x * sfx, guiEle[i]->y * sfy);
		window.draw(rect);

		rect.setFillColor(sf::Color(64, 64, 64));
		rect.setSize(sf::Vector2f(guiEle[i]->w * sfy - sfy * 0.66, guiEle[i]->h * sfy - sfy * 0.66));
		rect.setPosition(guiEle[i]->x * sfx + sfy * 0.33, guiEle[i]->y * sfy + sfy * 0.33);
		window.draw(rect);

		if (i == 0)
		{
			rect.setFillColor(sf::Color(64 + 190 - p.energy * 1.8, 64, 64));
			window.draw(rect);
		}
		if (i == 1)
		{
			rect.setFillColor(sf::Color(64 + 190 - p.health * 1.9, 64 + p.health * 1.9, 64));
			window.draw(rect);
		}
	}

	window.display();
}


void rmenu()
{
#pragma warning (push)
#pragma warning (disable:4267)
	rect.setFillColor(sf::Color(0, 0, 0, 32));
	rect.setOutlineColor(sf::Color(0, 0, 0, 0));
	rect.setPosition(0, 0);
	rect.setSize(sf::Vector2f(width, height));
	window.draw(rect);

	if (Pages.size() > 0)
	{
		// DRAW BUTTON ###########################################################################
		for (int i = 0; i < Pages[pagenum]->btn.size(); i++)
		{
			BTN btn = *Pages[pagenum]->btn[i];
			btn.x *= sfx;
			btn.y *= sfy;
			btn.w *= sfx;
			btn.h *= sfy;

			btn.y += std::sin(ticks / 10.0) * sfy * 0.66 - sfy * 0.33;

			if (mpos.x >= btn.x && mpos.x <= btn.x + btn.w && mpos.y >= btn.y && mpos.y <= btn.y + btn.h)
			{
				rect.setFillColor(sf::Color(224, 32, 32));
				rect.setOutlineColor(sf::Color(255, 64, 64));
				text.setFillColor(sf::Color(255, 64, 64));
				btn.x -= sfx * 0.33; btn.y -= sfy * 0.33;
				btn.w += sfx * 0.66; btn.h += sfy * 0.66;
				btn.fs += sfx * 0.33;

				// BUTTON EVENTS ##################################################################
				if (!left && leftL)
				{
					if (btn.evt < 100 && btn.evt >= 0) // GOTO PAGE
					{
						pagenum = btn.evt;
						tmpsettings.clear();
						tmpsettingsid.clear();
					}
					if (btn.evt == 100) // EXIT
						window.close();
					if (btn.evt == 101) // CREATE WORLD
					{
						for (int f = 0; f < settingsid.size(); f++)
						{
							int val = -1;
							for (int m = 0; m < Pages.size(); m++)
							{
								for (int d = 0; d < Pages[m]->sld.size(); d++)
								{
									if (settingsid[f] == Pages[m]->sld[d]->id)
									{
										SLD sld = *Pages[m]->sld[d];
										int stops = 1, parselen;
										std::vector<std::string>stop;
										for (int s = 0; s < sld.str.length(); s++)
											if (sld.str[s] == ',')
												stops++;
										for (int s = 0; s < stops + 1; s++)
										{
											parselen = sld.str.find(',');
											if (parselen != std::string::npos)
											{
												stop.push_back(sld.str.substr(0, parselen));
												sld.str.erase(0, (double)parselen + 1);
											}
										}
										stop.push_back(sld.str);
										val = stoi(stop[settings[f]]);
									}
								}
							}

							switch (settingsid[f])
							{
							case 0:
								tilesize = val;
								break;
							case 1:
								viewdist = val;
								break;
							case 2:
								loaddist = val;
							}
						}
						initGen();
						p = Player();
						ingame = true;
					}
					if (btn.evt == 102) // SAVE SETTINGS
					{
						// Check for duplicates and set values
						for (int p = 0; p < tmpsettingsid.size(); p++)
						{
							int index = -1;
							for (int s = 0; s < settingsid.size(); s++)
								if (tmpsettingsid[p] == settingsid[s])
									index = s;
							if (index == -1)
							{
								settingsid.push_back(tmpsettingsid[p]);
								settings.push_back(tmpsettings[p]);
							}
							else
							{
								settingsid[index] = tmpsettingsid[p];
								settings[index] = tmpsettings[p];
							}
						}

						std::string data;
						std::fstream file("res/settings.txt", std::ios::out);
						file << "";
						for (int f = 0; f < settingsid.size(); f++)
							file << settings[f] << ":" << settingsid[f] << "\n";
						file.close();
					}
				}
				// ################################################################################
			}
			else
			{
				rect.setFillColor(sf::Color(128, 128, 128));
				rect.setOutlineColor(sf::Color(160, 160, 160));
				text.setFillColor(sf::Color(160, 160, 160));
			}

			rect.setOutlineThickness(5);
			rect.setSize(sf::Vector2f(btn.w, btn.h));
			rect.setPosition(btn.x, btn.y);
			rect.setTexture(tex[0]);
			window.draw(rect);

			text.setCharacterSize(btn.fs);
			text.setString(btn.str);
			text.setPosition(btn.x, btn.y);
			text.setPosition(rect.getGlobalBounds().left + rect.getGlobalBounds().width * 0.5 - text.getGlobalBounds().width * 0.5, 
				rect.getGlobalBounds().top + rect.getGlobalBounds().height * 0.5 - text.getGlobalBounds().height * 0.5 - btn.fs * 0.25);
			window.draw(text);
		}
		// ########################################################################################

		// DRAW TITLE #############################################################################
		for (int i = 0; i < Pages[pagenum]->ttl.size(); i++)
		{
			TTL ttl = *Pages[pagenum]->ttl[i];
			ttl.x *= sfx;
			ttl.y *= sfy;
			ttl.w *= sfx;
			ttl.h *= sfy;

			ttl.y += std::sin(ticks / 10.0) * sfy * 0.66 - sfy * 0.33;

			rect.setSize(sf::Vector2f(ttl.w, ttl.h));
			rect.setPosition(ttl.x, ttl.y);

			text.setFillColor(sf::Color(255, 64, 64));
			text.setCharacterSize(ttl.fs);
			text.setString(ttl.str);
			text.setPosition(ttl.x, ttl.y);
			text.setPosition(rect.getGlobalBounds().left + rect.getGlobalBounds().width * 0.5 - text.getGlobalBounds().width * 0.5, rect.getGlobalBounds().top + rect.getGlobalBounds().height * 0.5 - text.getGlobalBounds().height * 0.5 - ttl.fs * 0.25);
			window.draw(text);
		}
		// ########################################################################################

		// DRAW LABEL #############################################################################
		for (int i = 0; i < Pages[pagenum]->lbl.size(); i++)
		{
			LBL lbl = *Pages[pagenum]->lbl[i];
			lbl.x *= sfx;
			lbl.y *= sfy;
			lbl.w *= sfx;
			lbl.h *= sfy;

			lbl.y += std::sin(ticks / 10.0) * sfy * 0.66 - sfy * 0.33;

			rect.setSize(sf::Vector2f(lbl.w, lbl.h));
			rect.setPosition(lbl.x, lbl.y);
			rect.setFillColor(sf::Color(128, 128, 128));
			rect.setOutlineColor(sf::Color(160, 160, 160));
			window.draw(rect);

			text.setFillColor(sf::Color(160, 160, 160));
			text.setCharacterSize(lbl.fs);
			text.setString(lbl.str);
			text.setPosition(lbl.x, lbl.y);
			text.setPosition(rect.getGlobalBounds().left + rect.getGlobalBounds().width * 0.5 - text.getGlobalBounds().width * 0.5, rect.getGlobalBounds().top + rect.getGlobalBounds().height * 0.5 - text.getGlobalBounds().height * 0.5 - lbl.fs * 0.25);
			window.draw(text);
		}
		// ########################################################################################

		// DRAW SLIDER ############################################################################
		for (int i = 0; i < Pages[pagenum]->sld.size(); i++)
		{
			SLD sld = *Pages[pagenum]->sld[i];
			sld.x *= sfx;
			sld.y *= sfy;
			sld.w *= sfx;
			sld.h *= sfy;

			int index = -1;
			for (int s = 0; s < tmpsettingsid.size(); s++)
				if (tmpsettingsid[s] == sld.id)
				{
					sld.val = tmpsettings[s];
					index = s;
				}
			if (index == -1)
				for (int s = 0; s < settingsid.size(); s++)
					if (settingsid[s] == sld.id)
						sld.val = settings[s];
			int stops = 1, parselen;
			std::vector<std::string>stop;
			for (int s = 0; s < sld.str.length(); s++)
				if (sld.str[s] == ',')
					stops++;
			for (int s = 0; s < stops + 1; s++)
			{
				parselen = sld.str.find(',');
				if (parselen != std::string::npos)
				{
					stop.push_back(sld.str.substr(0, parselen));
					sld.str.erase(0, (double)parselen + 1);
				}
			}
			stop.push_back(sld.str);

			sld.y += std::sin(ticks / 10.0) * sfy * 0.66 - sfy * 0.33;

			rect.setFillColor(sf::Color(128, 128, 128));
			rect.setOutlineColor(sf::Color(160, 160, 160));
			rect.setSize(sf::Vector2f(sld.w, sld.h - sfy * 2));
			rect.setPosition(sld.x, sld.y + sfy);
			window.draw(rect);

			if (mpos.x >= sld.x + (double)sld.val * (1.0 / (stops - 1)) * sld.w - sfx * 2 && mpos.x <= sld.x + (double)sld.val * (1.0 / (stops - 1)) * sld.w - sfx * 2 + sfx * 4 && mpos.y >= sld.y && mpos.y <= sld.y + sld.h)
			{
				rect.setFillColor(sf::Color(224, 32, 32));
				rect.setOutlineColor(sf::Color(255, 64, 64));
			}

			if (mpos.x >= sld.x && mpos.x <= sld.x + sld.w && mpos.y >= sld.y && mpos.y <= sld.y + sld.h)
			{
				if (left)
				{
					sld.val = std::min(int(mpos.x - sld.x) / (sld.w / stops), stops - 1);

					int index = -1;
					for (int s = 0; s < tmpsettingsid.size(); s++)
						if (sld.id == tmpsettingsid[s])
							index = s;
					if (index == -1)
					{
						tmpsettingsid.push_back(sld.id);
						tmpsettings.push_back(sld.val);
					}
					else
					{
						tmpsettingsid[index] = sld.id;
						tmpsettings[index] = sld.val;
					}
				}
			}
			else
			{
				rect.setFillColor(sf::Color(128, 128, 128));
				rect.setOutlineColor(sf::Color(160, 160, 160));
			}

			rect.setSize(sf::Vector2f(sfx * 4, sld.h));
			rect.setPosition(sld.x + (double)sld.val * (1.0 / (stops - 1)) * sld.w - sfx * 2, sld.y);
			window.draw(rect);

			text.setFillColor(sf::Color(160, 160, 160));
			text.setCharacterSize(sld.fs);
			text.setString(stop[sld.val]);
			text.setPosition(sld.x, sld.y);
			text.setPosition(rect.getGlobalBounds().left + rect.getGlobalBounds().width * 0.5 - text.getGlobalBounds().width * 0.5, rect.getGlobalBounds().top + rect.getGlobalBounds().height * 0.5 - text.getGlobalBounds().height * 0.5 - sld.fs * 0.25);
			window.draw(text);
		}
		// ########################################################################################
	}

	window.display();
#pragma warning (pop)
}


void update()
{
	// COOLDOWN ###################################################################################
	if (p.enrch < -5) 
		p.energy++;
	p.enrch--;
	if (p.energy > 100) 
		p.energy = 100;
	if (p.energy < 0) 
		p.energy = 0;
	// ############################################################################################

	if (!ininv)
	{
		// GENERATE NEW CHUNKS ####################################################################
		for (int x = p.xc - loaddist; x < p.xc + loaddist + 1; x++)
			for (int y = p.yc - loaddist; y < p.yc + loaddist + 1; y++)
			{
				int i = getChunkID(0, 0, -x, -y);
				if (i == -1)
					generate(-x, -y);
			}
		// ########################################################################################


		// PLAYER MOVEMENT ########################################################################
		int lastposx = p.x;
		int lastposy = p.y;
		int speed = p.speed;
		int mines = p.minespeed;
		p.yv += -int(w) + int(s);
		p.xv += -int(a) + int(d);

		if (shift && p.energy > 0)
		{
			speed += 1;
			mines += 2;
		}

		if (p.yv < 0)
			for (int i = 0; i < speed * abs(p.yv); i++)
			{
				bool move = true;
				for (int x = -1; x < 2; x++)
					if (getTileID(p.x + x, p.y - 2, -p.xc, -p.yc) != 0)
						move = false;
				if (move)
				{
					p.y--;
					if (shift)
					{
						p.energy -= 0.01;
						p.enrch = 10;
					}
				}
			}
		if (p.yv > 0)
			for (int i = 0; i < speed * abs(p.yv); i++)
			{
				bool move = true;
				for (int x = -1; x < 2; x++)
					if (getTileID(p.x + x, p.y + 2, -p.xc, -p.yc) != 0)
						move = false;
				if (move)
				{
					p.y++;
					if (shift)
					{
						p.energy -= 0.1;
						p.enrch = 10;
					}
				}
			}
		if (p.xv < 0)
			for (int i = 0; i < speed * abs(p.xv); i++)
			{
				bool move = true;
				for (int y = -1; y < 2; y++)
					if (getTileID(p.x - 2, p.y + y, -p.xc, -p.yc) != 0)
						move = false;
				if (move)
				{
					p.x--;
					if (shift)
					{
						p.energy -= 0.1;
						p.enrch = 10;
					}
				}
			}
		if (p.xv > 0)
			for (int i = 0; i < speed * abs(p.xv); i++)
			{
				bool move = true;
				for (int y = -1; y < 2; y++)
					if (getTileID(p.x + 2, p.y + y, -p.xc, -p.yc) != 0)
						move = false;
				if (move)
				{
					p.x++;
					if (shift)
					{
						p.energy -= 0.1;
						p.enrch = 10;
					}
				}
			}

		p.yv *= 0.5;
		p.xv *= 0.5;
		// ########################################################################################


		// POSITION TO CHUNKS #####################################################################
		if (p.y < 0) 
		{ 
			p.y += 16; 
			p.yc++; 
		}
		if (p.y >= 16) 
		{ 
			p.y -= 16; 
			p.yc--; 
		}
		if (p.x < 0) 
		{ 
			p.x += 16;
			p.xc++; 
		}
		if (p.x >= 16) 
		{
			p.x -= 16;
			p.xc--;
		}
		// ########################################################################################


		// RESET MINE PROGRESS ####################################################################
		if (p.x != lastposx || p.y != lastposy)
			p.minedistcur = 0;
		// ########################################################################################

		int x = (p.rot == 90) - (p.rot == 270);
		int y = (p.rot == 180) - (p.rot == 0);

		// DESTROY TILES ##########################################################################
		if (left)
		{
			switch (p.mode)
			{
			case 0: // 3x1
			{
				for (int s = 0; s < mines; s++)
				{
					if (p.energy > 2 && p.mineprog >= 10)
					{
						for (int i = -1; i < 2; i++)
						{
							if (getTileID(p.x + i * std::abs(y) + (2 + p.minedistcur) * x, p.y + i * std::abs(x) + (2 + p.minedistcur) * y, -p.xc, -p.yc) != 0)
							{
								p.energy -= 0.25;
								p.enrch = 10;
							}
							addInv(getTileID(p.x + i * std::abs(y) + (2 + p.minedistcur) * x, p.y + i * std::abs(x) + (2 + p.minedistcur) * y, -p.xc, -p.yc));
							setTileID(p.x + i * std::abs(y) + (2 + p.minedistcur) * x, p.y + i * std::abs(x) + (2 + p.minedistcur) * y, -p.xc, -p.yc, 0);
						}
						p.mineprog -= 10;
						p.minedistcur++;
						if (p.minedistcur >= p.minedist)
							p.minedistcur -= p.minedist;
					}
					p.mineprog++;
				}
				break;
			}
			case 1: // ATTACK
				break;
			case 2: // 1x1
			{
				for (int s = 0; s < mines; s++)
				{
					if (p.energy > 0 && p.mineprog >= 10)
					{
						if (getTileID(p.x + (2 + p.minedistcur) * x, p.y + (2 + p.minedistcur) * y, -p.xc, -p.yc) != 0)
						{
							p.energy -= 0.25;
							p.enrch = 10;
						}
						addInv(getTileID(p.x + (2 + p.minedistcur) * x, p.y + (2 + p.minedistcur) * y, -p.xc, -p.yc));
						setTileID(p.x + (2 + p.minedistcur) * x, p.y + (2 + p.minedistcur) * y, -p.xc, -p.yc, 0);
						p.mineprog -= 10;
						p.minedistcur++;
						if (p.minedistcur >= p.minedist)
							p.minedistcur -= p.minedist;
					}
					p.mineprog++;
				}
				break;
			}
			}
		}
		else
		{
			p.mineprog = 0;
			p.minedistcur = 0;
		}
		// ########################################################################################


		// PLACE TILES ############################################################################
		if (right)
		{
			switch (p.mode)
			{
			case 0: // 3x1
			{
				if (p.inv[p.invsel]->count > 2)
				{
					for (int i = -1; i < 2; i++)
						if (getTileID(p.x + i * std::abs(y) + x * 2, p.y + i * std::abs(x) + y * 2, -p.xc, -p.yc) == 0)
						{
							if (p.inv[p.invsel]->id != 0)
							{
								setTileID(p.x + i * std::abs(y) + x * 2, p.y + i * std::abs(x) + y * 2, -p.xc, -p.yc, p.inv[p.invsel]->id);
								p.inv[p.invsel]->count--;
								if (p.inv[p.invsel]->count == 0)
									p.inv[p.invsel]->id = 0;
							}
						}
				}
				break;
			}
			case 1: // USE
				break;
			case 2: // 1x1
			{
				if (p.inv[p.invsel]->count > 0)
				{
					if (getTileID(p.x + x * 2, p.y + y * 2, -p.xc, -p.yc) == 0)
					{
						if (p.inv[p.invsel]->id != 0)
						{
							setTileID(p.x + x * 2, p.y + y * 2, -p.xc, -p.yc, p.inv[p.invsel]->id);
							p.inv[p.invsel]->count--;
							if (p.inv[p.invsel]->count == 0)
								p.inv[p.invsel]->id = 0;
						}
					}
				}
			}
				break;
			}
		}
		// ########################################################################################


		// SET ROTATION ###########################################################################
		int lastrot = p.rot;
		double deg = (std::atan2(width / 2 - mpos.x, height / 2 - mpos.y) * 180.0) / -3.1416;
		if (deg < 0) deg += 360;

		if (deg >= 360 - 45 || deg < 45) p.rot = 0;
		if (deg >= 90 - 45 && deg < 90 + 45) p.rot = 90;
		if (deg >= 180 - 45 && deg < 180 + 45) p.rot = 180;
		if (deg >= 270 - 45 && deg < 270 + 45) p.rot = 270;

		if (p.rot != lastrot)
		{
			p.minedistcur = 0;
			p.mineprog = 0;
		}
		// ########################################################################################

		// LIGHT PLAYER ###########################################################################
		for (int x = -15; x < 16; x++)
			for (int y = -15; y < 16; y++)
				setTileLight(p.x + x, p.y + y, -p.xc, -p.yc, 0);

		lightDiamond(p.x, p.y, -p.xc, -p.yc, 12);
		// ########################################################################################
	}	
	else
	{
		p.minedistcur = 0;
		p.mineprog = 0;
	}
}

// NEW TOP ##########################################################################################

int main()
{
	window.setFramerateLimit(60);
	window.setPosition(sf::Vector2i(window.getPosition().x, 0));

	for (int i = 0; i < 256; i++)
		tex[i] = new sf::Texture();
	tex[0]->loadFromFile("res/blocks/air.png");
	tex[1]->loadFromFile("res/blocks/stone.png");
	tex[2]->loadFromFile("res/blocks/border.png");

	font.loadFromFile("res/cas.ttf");
	text.setFont(font);

	loadGUI();

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
			if (event.type == sf::Event::GainedFocus)
				focus = true;
			if (event.type == sf::Event::LostFocus)
				focus = false;
			if (event.type == sf::Event::Resized)
			{
				sf::FloatRect view(0, 0, event.size.width, event.size.height);
				window.setView(sf::View(view));
				width = std::floor(event.size.width / 2) * 2;
				height = std::floor(event.size.height / 2) * 2;
				sfx = width / 64; sfy = height / 48;
			}
		}

		now = std::chrono::time_point_cast<
			std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
		if (last == 0) 
			last = now;
		delta += now - last;
		last = now;

		while (delta >= tps)
		{
			delta -= tps;
			ticks++;
			update();
		}

		if (ingame)
			render();
		else
			rmenu();

		if (focus)
		{
			getkey();
			updateKey();
		}
	}

	return 0;
}


void save()
{

}


void load()
{

}


void addInv(int id)
{
	bool found = false;
	if (id != 0)
	{
		for (int i = 0; i < 72; i++)
		{
			if (p.inv[i]->id == id && p.inv[i]->count < 450 && !found)
			{
				p.inv[i]->count++;
				found = true;
			}
		}
		if (!found)
		{
			for (int i = 0; i < 72; i++)
			{
				if (p.inv[i]->id == 0 && !found)
				{
					p.inv[i]->id = id;
					p.inv[i]->count++;
					found = true;
				}
			}
		}
	}
}


int getChunkID(int x, int y, int xc, int yc)
{
	int xx = xc;
	int yy = yc;
	if (x < 0) xx--;
	if (x >= 16) xx++;
	if (y < 0) yy--;
	if (y >= 16) yy++;

	for (int i = 0; i < chunks.size(); i++)
		if (chunks[i]->x == xx && chunks[i]->y == yy)
			return i;
	return -1;
}


int getTileID(int x, int y, int xc, int yc)
{
	int xx = x;
	int yy = y;
	int i = getChunkID(x, y, xc, yc);
	if (i != -1)
	{
		if (x < 0) xx += 16;
		if (x >= 16) xx -= 16;
		if (y < 0) yy += 16;
		if (y >= 16) yy -= 16;
		return chunks[i]->tiles[xx][yy]->id;
	}
	return -1;
}


void setTileID(int x, int y, int xc, int yc, int id)
{
	int xx = x;
	int yy = y;
	int i = getChunkID(x, y, xc, yc);
	if (i != -1)
	{
		if (x < 0) xx += 16;
		if (x >= 16) xx -= 16;
		if (y < 0) yy += 16;
		if (y >= 16) yy -= 16;
		chunks[i]->tiles[xx][yy]->id = id;
	}
}


int getTileLight(int x, int y, int xc, int yc)
{
	int xx = x;
	int yy = y;
	int i = getChunkID(x, y, xc, yc);
	if (i != -1)
	{
		if (x < 0) xx += 16;
		if (x >= 16) xx -= 16;
		if (y < 0) yy += 16;
		if (y >= 16) yy -= 16;
		return chunks[i]->tiles[xx][yy]->light;
	}
	return -1;
}


void setTileLight(int x, int y, int xc, int yc, int light)
{
	int xx = x;
	int yy = y;
	int i = getChunkID(x, y, xc, yc);
	if (i != -1)
	{
		if (x < 0) xx += 16;
		if (x >= 16) xx -= 16;
		if (y < 0) yy += 16;
		if (y >= 16) yy -= 16;
		chunks[i]->tiles[xx][yy]->light = light;
	}
}


void lightDiamond(int x, int y, int xc, int yc, int light)
{
	if (light > 0)
	{
		if (getTileLight(x, y - 1, xc, yc) < light && getTileID(x, y, xc, yc) == 0)
		{
			setTileLight(x, y - 1, xc, yc, light);
			lightDiamond(x, y - 1, xc, yc, light - 1);
		}
		if (getTileLight(x + 1, y, xc, yc) < light && getTileID(x, y, xc, yc) == 0)
		{
			setTileLight(x + 1, y, xc, yc, light);
			lightDiamond(x + 1, y, xc, yc, light - 1);
		}
		if (getTileLight(x, y + 1, xc, yc) < light && getTileID(x, y, xc, yc) == 0)
		{
			setTileLight(x, y + 1, xc, yc, light);
			lightDiamond(x, y + 1, xc, yc, light - 1);
		}
		if (getTileLight(x - 1, y, xc, yc) < light && getTileID(x, y, xc, yc) == 0)
		{
			setTileLight(x - 1, y, xc, yc, light);
			lightDiamond(x - 1, y, xc, yc, light - 1);
		}
	}
}


void generate(int xc, int yc)
{
	double noiseX, noiseY, noiseLvl;
	Chunk* c = new Chunk(xc, yc);

	for (int x = 0; x < 16; x++)
		for (int y = 0; y < 16; y++)
		{
			noiseX = (xc * 16.0 + x) * 0.015625;
			noiseY = (yc * 16.0 + y) * 0.015625;
			noiseLvl = noise.unsignedFBM(noiseX, noiseY, 3, 2.5, 0.3) + 0.625;
			c->tiles[x][y]->id = int(noiseLvl);

			c->tilesf[x][y]->id = 1;
		}
	chunks.push_back(c);
}


void initGen()
{
	for (int x = p.xc - loaddist; x < p.xc + loaddist + 1; x++)
		for (int y = p.yc - loaddist; y < p.yc + loaddist + 1; y++)
		{
			int i = getChunkID(0, 0, -x, -y);
			if (i == -1)
				generate(-x, -y);
		}
	for (int x = -1; x < 2; x++)
		for (int y = -1; y < 2; y++)
			setTileID(p.x + x, p.y + y, -p.xc, -p.yc, 0);
}


void renderPlayer()
{
	int xpos, ypos;
	if (p.mineprog > 0)
	{
		xpos = width / 2 - p.rad / 2 + tilesize;
		ypos = height / 2 - p.rad / 2 + tilesize;
		rect.setTexture(tex[2]);
		rect.setFillColor(sf::Color(255 * (p.mineprog * 0.066 + 0.33), 255 * (p.mineprog * 0.066 + 0.33), 255 * (p.mineprog * 0.066 + 0.33), 128));

		cnvx.setFillColor(sf::Color(255 * (p.mineprog * 0.066 + 0.33), 0, 0, 128));
		cnvx.setPointCount(3);
		cnvx.setPoint(0, sf::Vector2f(width / 2, height / 2));
		cnvx.setPoint(1, sf::Vector2f(width / 2, height / 2));
		cnvx.setPoint(2, sf::Vector2f(width / 2, height / 2));

		switch (p.rot)
		{
		case 0:
			rect.setPosition(xpos - tilesize, ypos - tilesize * (2 + p.minedistcur));
			rect.setSize(sf::Vector2f(p.rad, tilesize));
			window.draw(rect);

			cnvx.setPoint(1, sf::Vector2f(width / 2 - p.rad / 2, height / 2 - (p.rad / 2 + tilesize * p.minedistcur)));
			cnvx.setPoint(2, sf::Vector2f(width / 2 + p.rad / 2, height / 2 - (p.rad / 2 + tilesize * p.minedistcur)));
			break;
		case 90:
			rect.setPosition(xpos + tilesize * (2 + p.minedistcur), ypos - tilesize);
			rect.setSize(sf::Vector2f(tilesize, p.rad));
			window.draw(rect);

			cnvx.setPoint(1, sf::Vector2f(width / 2 + (p.rad / 2 + tilesize * p.minedistcur), height / 2 - p.rad / 2));
			cnvx.setPoint(2, sf::Vector2f(width / 2 + (p.rad / 2 + tilesize * p.minedistcur), height / 2 + p.rad / 2));
			break;
		case 180:
			rect.setPosition(xpos - tilesize, ypos + tilesize * (2 + p.minedistcur));
			rect.setSize(sf::Vector2f(p.rad, tilesize));
			window.draw(rect);

			cnvx.setPoint(1, sf::Vector2f(width / 2 - p.rad / 2, height / 2 + (p.rad / 2 + tilesize * p.minedistcur)));
			cnvx.setPoint(2, sf::Vector2f(width / 2 + p.rad / 2, height / 2 + (p.rad / 2 + tilesize * p.minedistcur)));
			break;
		case 270:
			rect.setPosition(xpos - tilesize * (2 + p.minedistcur), ypos - tilesize);
			rect.setSize(sf::Vector2f(tilesize, p.rad));
			window.draw(rect);

			cnvx.setPoint(1, sf::Vector2f(width / 2 - (p.rad / 2 + tilesize * p.minedistcur), height / 2 - p.rad / 2));
			cnvx.setPoint(2, sf::Vector2f(width / 2 - (p.rad / 2 + tilesize * p.minedistcur), height / 2 + p.rad / 2));
			break;
		}

		window.draw(cnvx);
	}

	rect.setFillColor(sf::Color(255, 255, 255));
	rect.setTexture(tex[2]);
	rect.setPosition((float)width / 2 - p.rad / 2.0, (float)height / 2 - p.rad / 2.0);
	rect.setSize(sf::Vector2f(p.rad, p.rad));
	window.draw(rect);
}


void renderTiles()
{
	int index, xpos, ypos, light;
	for (int cx = p.xc - viewdist; cx < p.xc + viewdist + 1; cx++)
		for (int cy = p.yc - viewdist; cy < p.yc + viewdist + 1; cy++)
		{
			index = getChunkID(0, 0, -cx, -cy);
			if (index != -1)
			{
				for (int x = 0; x < 16; x++)
					for (int y = 0; y < 16; y++)
					{
						xpos = width / 2.0 + (chunks[index]->x * 16.0 + x + p.xc * 16.0 - p.x) * tilesize - tilesize / 2.0;
						ypos = height / 2.0 + (chunks[index]->y * 16.0 + y + p.yc * 16.0 - p.y) * tilesize - tilesize / 2.0;
						rect.setPosition(xpos, ypos);

						if (chunks[index]->tiles[x][y]->id)
						{
							light = 255 * ((getTileLight(x, y, -cx, -cy) + 1.0) / 16.0);
							rect.setFillColor(sf::Color(light, light, light));
							rect.setTexture(tex[chunks[index]->tiles[x][y]->id]);
						}
						else
						{
							light = 160 * ((getTileLight(x, y, -cx, -cy) + 1.0) / 16.0);
							rect.setFillColor(sf::Color(light, light, light));
							rect.setTexture(tex[chunks[index]->tilesf[x][y]->id]);
						}
						window.draw(rect);
					}
			}
		}
}


void updateKey()
{
	if (!e && eL)
	{
		ininv = !ininv;
	}

	if (one) p.invsel = 0;
	if (two) p.invsel = 1;
	if (thr) p.invsel = 2;
	if (fou) p.invsel = 3;
	if (fiv) p.invsel = 4;
	if (six) p.invsel = 5;
	if (sev) p.invsel = 6;
	if (eig) p.invsel = 7;
	if (nin) p.invsel = 8;

	if (q && !qL) p.mode = (p.mode + 1) % 3;

	if (r) p = Player();
}


void getkey()
{
	w = GetKeyState(0x57) < 0;
	s = GetKeyState(0x53) < 0;
	a = GetKeyState(0x41) < 0;
	d = GetKeyState(0x44) < 0;

	qL = q;
	q = GetKeyState(0x51) < 0;
	eL = e;
	e = GetKeyState(0x45) < 0;
	rL = r;
	r = GetKeyState(0x52) < 0;
	lL = l;
	l = GetKeyState(0x4C) < 0;

	zer = GetKeyState(0x30) < 0;
	one = GetKeyState(0x31) < 0;
	two = GetKeyState(0x32) < 0;
	thr = GetKeyState(0x33) < 0;
	fou = GetKeyState(0x34) < 0;
	fiv = GetKeyState(0x35) < 0;
	six = GetKeyState(0x36) < 0;
	sev = GetKeyState(0x37) < 0;
	eig = GetKeyState(0x38) < 0;
	nin = GetKeyState(0x39) < 0;

	shift = GetKeyState(0x10) < 0;

	leftL = left;
	left = GetKeyState(0x01) < 0;
	rightL = right;
	right = GetKeyState(0x02) < 0;

	GetCursorPos(&mpos);
	ScreenToClient(window.getSystemHandle(), &mpos);
}


void loadGUI()
{
#pragma warning (push)
#pragma warning (disable:4267)
	std::fstream pages("res/menu.mnu", std::ios::in);
	std::string data;
	while (std::getline(pages, data))
	{
		int parselen;
		for (int i = data.size() - 1; i >= 0; i--)
			if (data[i] == ' ')
				data.erase(i, 1);

		switch (data[0] + data[1] + data[2])
		{
		case 'N' + 'E' + 'W':
		{
			Pages.push_back(new PAGE);
			break;
		}
		case 'B' + 'T' + 'N':
		{
			int p[6] = {0, 0, 0, 0, 0, 0};
			std::string str;
			data.erase(0, 3);
			for (int i = 0; i < 6; i++)
			{
				parselen = data.find(',');
				p[i] = stoi(data.substr(0, parselen));
				data.erase(0, (double)parselen + 1);
			}
			parselen = data.find('"', 1);
			str = data.substr(1, (double)parselen - 1);
			data.erase(0, (double)parselen + 1);

			for (int i = 0; i < str.length(); i++)
				if (str[i] == '_')
					str.replace(i, 1, " ");
			Pages[Pages.size() - 1]->btn.push_back(new BTN(p[0], p[1], p[2], p[3], p[4], p[5], str));
			break;
		}
		case 'T' + 'T' + 'L':
		{
			int p[5] = {0, 0, 0, 0, 0};
			std::string str;
			data.erase(0, 3);
			for (int i = 0; i < 5; i++)
			{
				parselen = data.find(',');
				p[i] = stoi(data.substr(0, parselen));
				data.erase(0, (double)parselen + 1);
			}
			parselen = data.find('"', 1);
			str = data.substr(1, (double)parselen - 1);
			data.erase(0, (double)parselen + 1);

			for (int i = 0; i < str.length(); i++)
				if (str[i] == '_')
					str.replace(i, 1, " ");
			Pages[Pages.size() - 1]->ttl.push_back(new TTL(p[0], p[1], p[2], p[3], p[4], str));
			break;
		}
		case 'L' + 'B' + 'L':
		{
			int p[5] = {0, 0, 0, 0, 0};
			std::string str;
			data.erase(0, 3);
			for (int i = 0; i < 5; i++)
			{
				parselen = data.find(',');
				p[i] = stoi(data.substr(0, parselen));
				data.erase(0, (double)parselen + 1);
			}
			parselen = data.find('"', 1);
			str = data.substr(1, (double)parselen - 1);
			data.erase(0, (double)parselen + 1);

			for (int i = 0; i < str.length(); i++)
				if (str[i] == '_')
					str.replace(i, 1, " ");
			Pages[Pages.size() - 1]->lbl.push_back(new LBL(p[0], p[1], p[2], p[3], p[4], str));
			break;
		}
		case 'S' + 'L' + 'D':
		{
			int p[7] = {0, 0, 0, 0, 0, 0, 0};
			std::string str;
			data.erase(0, 3);
			for (int i = 0; i < 7; i++)
			{
				parselen = data.find(',');
				p[i] = stoi(data.substr(0, parselen));
				data.erase(0, (double)parselen + 1.0);
			}
			parselen = data.find('"', 1);
			str = data.substr(1, (double)parselen - 1.0);
			data.erase(0, (double)parselen + 1.0);

			for (int i = 0; i < str.length(); i++)
				if (str[i] == '_')
					str.replace(i, 1, " ");
			Pages[Pages.size() - 1]->sld.push_back(new SLD(p[0], p[1], p[2], p[3], p[4], p[5], p[6], str));
			break;
		}
		}
	}
	pages.close();

	std::fstream set("res/settings.txt", std::ios::in);
	if (set.is_open())
	{
		while (std::getline(set, data))
		{
			if (data.length() > 2)
			{
				settings.push_back(stoi(data.substr(0, data.find(':'))));
				settingsid.push_back(stoi(data.substr(data.find(':') + 1)));
			}
		}
	}
	set.close();

	for (int y = 0; y < 3; y++)
		for (int x = 0; x < 3; x++)
			invEle.push_back(new InvElement(2 + x * 3, 36 + y * 3, 3, 3));
	for (int y = 0; y < 3; y++)
		invEle.push_back(new InvElement(14, 36 + y * 3, 3, 3));

	for (int x = 0; x < 2; x++)
		guiEle.push_back(new InvElement(55 + x * 3, 36, 3, 9));
#pragma warning (pop)
}
