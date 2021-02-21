#include "main.h"

using namespace std;

DWORD dwBytesWritten = 0; // useless debaging

// class for game =)
class Game
{
public:

	HANDLE hConsole;

	wchar_t* screen; // buffer for rendering screen
	int wScreen; //width
	int hScreen; // height
	wstring map; // map in string format. # is wall, ' ' is empty
	int wMap; //width map
	int hMap; //height

	double vPlayer; // velocity of player
	double xPlayer; // x position of player
	double yPlayer; // y position
double aPlayer; // angle of view
double dt; // delta time for moving
double viewAngle; // max view angle

Game()
{
	// set standart sizes
	wScreen = 80; 
	hScreen = 25;
	// and buffer
	screen = new wchar_t[wScreen * hScreen + 1];
	// and window handler
	hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
}

bool isRun;
void setDefault()
{
	// default map for testing
	wMap = 16;
	hMap = 16;
	map = L"################";
	map += L"#              #";
	map += L"#              #";
	map += L"#              #";
	map += L"#   ##         #";
	map += L"#   ##         #";
	map += L"#              #";
	map += L"#############  #";
	map += L"#              #";
	map += L"#           #  #";
	map += L"#           #  #";
	map += L"#  ###      #  #";
	map += L"#    #      #  #";
	map += L"#    ### ####  #";
	map += L"#              #";
	map += L"################";
	xPlayer = 2.0;
	yPlayer = 2.0;
	aPlayer = 0.0;
	vPlayer = 1.0;
	viewAngle = 1.0;
}

// one day there will be loading map
void loadMap(const char* c)
{}


// drawing screen
void draw()
{
	// add minimap to screen
	for (int i = 0; i < wMap; i++)
		for (int j = 0; j < hMap; j++)
		{
			screen[(j + 1) * wScreen + i] = map[j * wMap + i];
		}
	// add player position
	screen[((int)yPlayer + 1) * wScreen + (int)xPlayer] = 'X';
	screen[wScreen * hScreen] = '\0';
	// add player info
	swprintf_s(screen, 42, L"X=%5.2f, Y=%5.2f, A=%5.2f FPS=%7.2f ", xPlayer, yPlayer, aPlayer, 1.0f / dt);
	WriteConsoleOutputCharacterW(hConsole, screen, wScreen * hScreen, { 0,0 }, &dwBytesWritten);
}

void rayCasting()
{
	// distances for corner. it's not working very well
	double dist_1 = 1e10;
	double dist_2 = 2e10;
	// drawing every columns of screen
	for (int i = 0; i < wScreen; i++)
	{
		// ray casting
		// we set ray angle for all "pixels" of screen
		double aRay = aPlayer - 0.5 * viewAngle + (viewAngle * i) / wScreen;
		// from player point
		double xRay = xPlayer;
		double yRay = yPlayer;
		double dist = 0.0;
		int maxIter = 50;
		int iter = 0;
		double step = 0.1;
		// trace while ray don't touch a wall
		while (iter < maxIter)
		{
			double newx = xRay + step * cos(aRay);
			double newy = yRay + step * sin(aRay);
			// if touch wall reduce step and try again
			if (map[int(newx) + int(newy) * wMap] == '#')
			{
				step /= 2.0;
			}
			else
			{
				xRay = newx;
				yRay = newy;
				dist += step;
				// if step too small then stop
				if (step < 1e-3)
				{
					break;
				}
			}
			iter++;
		}
		// correction for fish eye
		dist *= cos(aRay - aPlayer);

		// for drawing corners. not work very well
		if ((dist_1 < dist) && (dist_1 < dist_2))
		{
   		    wchar_t view = 'I';
			int sizeWall = hScreen / (dist * 2.0);
			if (sizeWall > hScreen / 2)
				sizeWall = hScreen / 2;
			if (sizeWall < 2)
				sizeWall = 2;
			int up = hScreen / 2 - sizeWall;
			int down = hScreen / 2 + sizeWall;

			for (int j = up; j < down; j++)
			{
				screen[i - 1 + j * wScreen] = view;
			}

		}
		dist_2 = dist_1;
		dist_1 = dist;
		// filling symbol depend on distance to wall
		wchar_t view = ' ';
		if (dist < 0.7)
		{
			view = '#';
		}
		else
		{
			if (dist < 1.0)
			{
				view = '@';
			}
			else
			{
				if (dist < 1.4)
				{
					view = 'O';
				}
				else
				{
					if (dist < 2.0)
					{
						view = 'o';
					}
					else
					{
						if (dist < 3.0)
						{
							view = '*';
						}
						else
						{
							if (dist < 4.0)
							{
								view = '.';
							}
						}
					}
				}
			}
		}
		
		// size of wall. more distance all walls are smaller
		int sizeWall = hScreen / (dist*2.0);
		if (sizeWall > hScreen / 2)
			sizeWall = hScreen / 2;
		if (sizeWall < 2)
			sizeWall = 2;
		int up = hScreen / 2 - sizeWall;
		int down = hScreen / 2 + sizeWall;
		// drawing a ceiling
		for (int j = 0; j < up; j++)
		{
			screen[i + j * wScreen] = '+';
		}
		// drawing wall
		for (int j = up; j < down; j++)
		{
			screen[i + j * wScreen] = view;
		}
		// drawing floor
		for (int j = down; j < hScreen; j++)
		{
			screen[i + j * wScreen] = '-';
		}
		}
	}

	void run()
	{
		// main function of all program
		isRun = true;
		// set console for drawing
		SetConsoleActiveScreenBuffer(hConsole);
		// timers for delta time calc
		auto tp1 = chrono::steady_clock::now();
		auto tp2 = chrono::steady_clock::now();

		// main loop while isRun
		while (isRun)
		{
			// delta time
			tp2 = chrono::steady_clock::now();
			chrono::duration<double> elapsedTime = tp2 - tp1;
			tp1 = tp2;
			dt = elapsedTime.count();
			// exit
			if (GetAsyncKeyState((unsigned short)'3') & 0x8000)
			{
				isRun = false;
			}
			// go forward
			if (GetAsyncKeyState((unsigned short)'W') & 0x8000)
			{
				double newX = xPlayer + vPlayer * cos(aPlayer) * dt;
				double newY = yPlayer + vPlayer * sin(aPlayer) * dt;
				// if you can
				if (map[int(newX) + int(newY) * wMap] == ' ')
				{
					xPlayer = newX;
					yPlayer = newY;
				}
			}

			// go back
			if (GetAsyncKeyState((unsigned short)'S') & 0x8000)
			{
				double newX = xPlayer - vPlayer * cos(aPlayer) * dt;
				double newY = yPlayer - vPlayer * sin(aPlayer) * dt;
				if (map[int(newX) + int(newY) * wMap] == ' ')
				{
					xPlayer = newX;
					yPlayer = newY;
				}
			}
			// go left
			if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
			{
				double newX = xPlayer + vPlayer * sin(aPlayer) * dt;
				double newY = yPlayer - vPlayer * cos(aPlayer) * dt;
				if (map[int(newX) + int(newY) * wMap] == ' ')
				{
					xPlayer = newX;
					yPlayer = newY;
				}
			}
			// go right
			if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
			{
				double newX = xPlayer - vPlayer * sin(aPlayer) * dt;
				double newY = yPlayer + vPlayer * cos(aPlayer) * dt;
				if (map[int(newX) + int(newY) * wMap] == ' ')
				{
					xPlayer = newX;
					yPlayer = newY;
				}
			}
			// turn left and normalize angle to [-pi, pi]
			if (GetAsyncKeyState((unsigned short)'Q') & 0x8000)
			{
				aPlayer -= 1.0 * dt;
				if (aPlayer < -M_PI)
				{
					aPlayer += M_PI* 2.0;
				}
			}
			// turn right
			if (GetAsyncKeyState((unsigned short)'E') & 0x8000)
			{
				aPlayer += 1.0 * dt;
				if (aPlayer > M_PI)
				{
					aPlayer -= M_PI * 2.0;
				}
			}
			// trace
			rayCasting();
			draw();
		}
	
	}
};


int main()
{
	std::cout << "Welcome to main menu!" << std::endl;
	std::cout << " * Press 1 to start default map" << std::endl;
	std::cout << " * Press 2 to load map from file" << std::endl; /////  TODO
	std::cout << " * Press 3 to close program" << std::endl;
	Game game;

	// choose your destiny =)
	while (1)
	{
		// load default map
		if (GetAsyncKeyState((unsigned short)'1') & 0x8000)
		{
			game.setDefault();
			break;
		}
		// load map from map.txt
		if (GetAsyncKeyState((unsigned short)'2') & 0x8000)
		{
			game.loadMap("map.txt");
			break;
		}
		// go away
		if (GetAsyncKeyState((unsigned short)'3') & 0x8000)
		{
			return 0;
		}
	}

	game.run();

	return 0;
}