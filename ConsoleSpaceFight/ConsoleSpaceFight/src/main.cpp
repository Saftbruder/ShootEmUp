#include <stdio.h>
#include <Windows.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <stdlib.h>
#include <math.h>
#include <chrono>

//typedef struct player_struct {
//
//} player;
//typedef struct projectile_struct {
//
//} projectile;
//typedef struct level_struct {
//
//} level;
//typedef struct output_struct {
//
//} output;
//typedef struct enemy_struct {
//
//} enemy;

//time of the last incrementation of the line position
static std::chrono::milliseconds lastTime;
//time of the last incrementation of the update time of the projectile
static std::chrono::milliseconds lastProjectileTime;
//delay between incrementations of the line position
static std::chrono::milliseconds delay;
//delay between incrementations of the update time of the projectile
static std::chrono::milliseconds delayProjectile;
//how much lines are displayed at the same time
static int height_output;


/*Source: www.cplusplus.com/forum/articles/10515/#msg49080*/
void ClearScreen()
{
	HANDLE                     hStdOut;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	DWORD                      count;
	DWORD                      cellCount;
	COORD                      homeCoords = { 0, 0 };

	hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hStdOut == INVALID_HANDLE_VALUE) return;

	/* Get the number of cells in the current buffer */
	if (!GetConsoleScreenBufferInfo(hStdOut, &csbi)) return;
	cellCount = csbi.dwSize.X *csbi.dwSize.Y;

	/* Fill the entire buffer with spaces */
	if (!FillConsoleOutputCharacter(
		hStdOut,
		(TCHAR) ' ',
		cellCount,
		homeCoords,
		&count
	)) return;

	/* Fill the entire buffer with the current colors and attributes */
	if (!FillConsoleOutputAttribute(
		hStdOut,
		csbi.wAttributes,
		cellCount,
		homeCoords,
		&count
	)) return;

	/* Move the cursor home */
	SetConsoleCursorPosition(hStdOut, homeCoords);
}

/*Open and read the file that contains the played level.
Read the file line by line and after the entire file has been processed reverses the order of the lines (frist line will be last line ...)*/
bool openAndReadFile(std::vector<std::string> &output_lines, std::string fileName, std::string &player, char &projectile, int &lineLength)
{
	std::ifstream myfile;
	myfile.open(fileName, std::ios::in);
	if (myfile.is_open())
	{
		std::string line;
		std::vector<std::string> tmp_output_lines;

		std::getline(myfile, player);
		std::getline(myfile, line);
		projectile = line.at(0);
			
		while (std::getline(myfile, line))
		{
			tmp_output_lines.push_back(line);
			lineLength = max(line.length(), lineLength);
		}
		for (int i = tmp_output_lines.size(); i > 0; i--)
		{
			output_lines.push_back(tmp_output_lines[i]);
		}

		return true;
	}
	return false;
}

/*Check if the output has lines left.
If the current position of the upper most line ist greater than the size of the output, then the end of the output has been reached.*/
bool outputHasLines(std::vector<std::string> &output_lines, int &pos)
{
	if (pos > output_lines.size())
		return false;
	else
		return true;
}

/*Creates the string that represents the character.*/
void createPlayer(std::string &player, int &playerLength, int &playerPos, int &lineLength)
{
	playerPos = (lineLength / 2);
	playerLength = player.length();
}

/*Creates the line that displays the character.*/
void createPlayerLine(std::string &playerLine, std::string &player, int &playerLength, int &playerPos, int &lineLength)
{
	for (int i = 0; i < lineLength; i++)
	{
		if (i < (playerPos - (playerLength / 2)) || i > (playerPos + (playerLength / 2)))
			playerLine.append(" ");
		else
		{
			playerLine = playerLine + player;
			i = i + playerLength;
		}
	}
}

/*Processes the projectile.
Alters the output so the projectile moves.*/
void processProjectiles(std::vector<std::string> &output_lines, int &pos, int &projectilePosX, int &projectilePosY, int &projectilePosY_, char &projectile)
{
	if (projectilePosX == -1 || projectilePosY == -1)
	{
		return;
	}
	else if ((projectilePosY - height_output) < 1)
	{
		output_lines[projectilePosY][projectilePosX] = ' ';
		projectilePosX = -1;
		projectilePosY = -1;
		projectilePosY_ = -1;
	}
	else
	{
		//Delete projectile in previous line if not a the beginning of the projectile life time
		if (projectilePosY_ < height_output-1)
		{
			output_lines[projectilePosY - (projectilePosY_+1)][projectilePosX] = ' ';
		}
		output_lines[projectilePosY - projectilePosY_][projectilePosX] = projectile;
		projectilePosY_--;
	}
}

/*Updates the display of the output.
First clear the screen then print the current lines that are supposed to be displayed.*/
void renderOutput(std::vector<std::string> &output_lines, int &pos, std::string &playerLine)
{
	ClearScreen();
	for (int i = pos - 1; i >= pos - height_output; i--)
	{
		std::cout << output_lines[i] << std::endl;
	}
	std::cout << playerLine << std::endl;
}

/*Changes the position of the player by the amount given.*/
void changePlayerPosition(std::string &playerLine, std::string &player, int &playerLength, int &playerPos, int &lineLength, int posChange)
{
	playerPos = playerPos + posChange;
	playerLine.clear();
	createPlayerLine(playerLine, player, playerLength, playerPos, lineLength);
}

/*Checks the console windows for input.
Left and right arrow key move the player, up arrow key fires projectile.*/
void checkKeyInputs(std::vector<std::string> &output_lines, int &pos, std::string &playerLine, std::string &player, int &playerLength, int &playerPos, int &lineLength, int &projectilePosX, int &projectilePosY, int &projectilePosY_)
{
	//Source: http://stackoverflow.com/questions/24708700/c-detect-when-user-presses-arrow-key
	HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
	DWORD NumInputs = 0;
	DWORD InputsRead = 0;
	bool running = true;

	INPUT_RECORD irInput;

	GetNumberOfConsoleInputEvents(hInput, &NumInputs);

	if (NumInputs < 1)
		return;

	ReadConsoleInput(hInput, &irInput, 1, &InputsRead);
	if (irInput.Event.KeyEvent.bKeyDown)
	{
		switch (irInput.Event.KeyEvent.wVirtualKeyCode)
		{
		case VK_RIGHT:
			changePlayerPosition(playerLine, player, playerLength, playerPos, lineLength, +1);
			break;
		case VK_LEFT:
			changePlayerPosition(playerLine, player, playerLength, playerPos, lineLength , -1);
			break;
		case VK_UP:
			if (projectilePosX != -1 || projectilePosY != -1)
			{
				output_lines[projectilePosY - (projectilePosY_ + 1)][projectilePosX] = ' ';
			}
			projectilePosX = playerPos;
			projectilePosY = pos - 1;
			projectilePosY_ = height_output - 1;
			break;
		}
	}
}

/*Contains the main loop for the game.
The main loop checks key input, then saves the current time, checks if projectiles have to be processed,
checks if the output needs to be rendered again (currently on the same delay as the projectile update), then checks if the output needs to move a line up.*/
void mainLoop(std::vector<std::string> &output_lines, std::string &playerLine, std::string &player, int &playerLength, int &playerPos, char &projectile, int &lineLength)
{
	int pos = height_output;
	int projectilePosX = -1;
	int projectilePosY = -1;
	int projectilePosY_ = -1;
	std::chrono::milliseconds currentTime;

	//wtf is this shit for, if I delete it, the game doesnt work?!
	//WutFace
	for (int i = 0; i < 10; i++)
	{
		std::string tmp = output_lines[i];
	}

	while (outputHasLines(output_lines, pos))
	{
		checkKeyInputs(output_lines, pos, playerLine, player, playerLength, playerPos, lineLength, projectilePosX, projectilePosY, projectilePosY_);
		
		currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
		
		if ((currentTime.count() - lastProjectileTime.count()) > delayProjectile.count())
		{
			processProjectiles(output_lines, pos, projectilePosX, projectilePosY, projectilePosY_, projectile);

			//update of the display depends on projectile update rate, change needed!
			renderOutput(output_lines, pos, playerLine);

			lastProjectileTime = currentTime;
		}

		if ((currentTime.count() - lastTime.count()) > delay.count())
		{
			lastTime = currentTime;
			pos++;
		}
	}
}

/*Initializes the gamestate (player, playerLine).*/
void initGame(std::string &playerLine, std::string &player, int &playerLength, int &playerPos, int &lineLength)
{
	createPlayer(player, playerLength, playerPos, lineLength);
	createPlayerLine(playerLine, player, playerLength, playerPos, lineLength);
}

void main()
{
	lastTime = std::chrono::duration_cast< std::chrono::milliseconds >(std::chrono::system_clock::now().time_since_epoch());
	lastProjectileTime = std::chrono::duration_cast< std::chrono::milliseconds >(std::chrono::system_clock::now().time_since_epoch());
	delay = std::chrono::milliseconds(500);
	delayProjectile = std::chrono::milliseconds(200);
	height_output = 10;

	std::vector<std::string> output_lines;
	int playerLength;
	int playerPos;
	std::string player;
	std::string playerLine;
	char projectile;
	int lineLength;

	std::string fileName = "levels/test_level_one.txt";

	if (openAndReadFile(output_lines, fileName, player, projectile, lineLength))
	{
		initGame(playerLine, player, playerLength, playerPos, lineLength);
		mainLoop(output_lines, playerLine, player, playerLength, playerPos, projectile, lineLength);
	}
	else
	{
		std::cout << "File ... was not found." << std::endl;
	}
	output_lines.clear();
}