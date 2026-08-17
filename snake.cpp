#include <iostream>
#include <vector>
#include <windows.h>
#include <ctime>
#include <cstdio>
#include <conio.h>
#include <fstream>
#include <string>
#include <mmsystem.h>
#include <algorithm>

#pragma comment(lib, "winmm.lib")
using namespace std;

// Console Color Definitions
enum ConsoleColor
{
    BLACK = 0,
    DARKBLUE = 1,
    DARKGREEN = 2,
    DARKCYAN = 3,
    DARKRED = 4,
    DARKMAGENTA = 5,
    DARKYELLOW = 6,
    GREY = 7,
    DARKGREY = 8,
    BLUE = 9,
    GREEN = 10,
    CYAN = 11,
    RED = 12,
    MAGENTA = 13,
    YELLOW = 14,
    WHITE = 15
};

// Structure to store high score entries
struct HighScoreEntry
{
    string playerName;
    int score;
};

bool borderWrap = false;
bool rainbowTrail = true;
int rainbowPhase = 0;
void SetColor(ConsoleColor textColor, ConsoleColor bgColor);
void ResetColor();
void GotoXY(int x, int y);
void ClearScreen();
void Draw();
void PlayMenuMusic();
void PlayGameMusic();
void StopMenuMusic();
void StopGameMusic();
void StopAllMusic();
void ToggleMusic();
void ShowPauseOverlay();
void Input();
void Logic();
void Setup();
void LoadHighScore();
void SaveHighScore();
void ApplySlowTimeEffect();
void SpawnSpecialFruit();
void ShowSettingsPage();
void ShowHighScorePage();
void ShowHelpPage();
int ShowMenu();
string GetPlayerName();

// MUSIC VARIABLES
bool musicEnabled = true;
string menuMusicFile = "menu.mp3";
string gameMusicFile = "game.mp3";

// Console Utility Functions
void SetColor(ConsoleColor textColor, ConsoleColor bgColor = BLACK)
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (WORD)((bgColor << 4) | textColor));
}
void ResetColor() { SetColor(WHITE, BLACK); }
void GotoXY(int x, int y)
{
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// Clear screen function without flickering
void ClearScreen()
{
    COORD topLeft = {0, 0};
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO screen;
    DWORD written;
    GetConsoleScreenBufferInfo(console, &screen);
    FillConsoleOutputCharacterA(console, ' ', screen.dwSize.X * screen.dwSize.Y, topLeft, &written);
    FillConsoleOutputAttribute(console, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE,
        screen.dwSize.X * screen.dwSize.Y, topLeft, &written);
    SetConsoleCursorPosition(console, topLeft);
}

// MUSIC FUNCTIONS
void PlayMenuMusic()
{
    if (!musicEnabled) return;
    mciSendString("close menumusic", NULL, 0, NULL);
    string command = "open \"" + menuMusicFile + "\" type mpegvideo alias menumusic";
    mciSendString(command.c_str(), NULL, 0, NULL);
    mciSendString("play menumusic repeat", NULL, 0, NULL);
    mciSendString("setaudio menumusic volume to 500", NULL, 0, NULL);
}
void PlayGameMusic()
{
    if (!musicEnabled) return;
    mciSendString("close gamemusic", NULL, 0, NULL);
    string command = "open \"" + gameMusicFile + "\" type mpegvideo alias gamemusic";
    mciSendString(command.c_str(), NULL, 0, NULL);
    mciSendString("play gamemusic repeat", NULL, 0, NULL);
    mciSendString("setaudio gamemusic volume to 400", NULL, 0, NULL);
}
void StopMenuMusic()
{
    mciSendString("stop menumusic", NULL, 0, NULL);
    mciSendString("close menumusic", NULL, 0, NULL);
}
void StopGameMusic()
{
    mciSendString("stop gamemusic", NULL, 0, NULL);
    mciSendString("close gamemusic", NULL, 0, NULL);
}
void StopAllMusic()
{
    StopMenuMusic();
    StopGameMusic();
}
void ToggleMusic()
{
    musicEnabled = !musicEnabled;
    if (!musicEnabled) StopAllMusic();
}

// Global Game Variables
bool gameOver;
const int width = 40;
const int height = 20;
int fruitX, fruitY;

// SPECIAL FRUIT VARIABLES
int specialFruitX, specialFruitY;
bool isSpecialFruitOnScreen = false;
const int SPECIAL_FRUIT_SPAWN_CHANCE = 5;
int fruitsEaten = 0;

// SPEED VARIABLES
bool isSlowTimeActive = false;
int slowTimeDuration = 0;
int originalGameSpeed = 110;
const int SLOW_TIME_FRAMES = 83;
int speedLevel = 1;

int gameSpeed = 130;
int frameCount = 0;
int highScore = 0;
string highScorePlayerName = "Player";

enum eDirection
{
    STOP = 0,
    LEFT,
    RIGHT,
    UP,
    DOWN
};
// A snake bundles its head, body, heading, score, and last-erased tail cell.
struct Snake
{
    int headX, headY;
    int tailX[1000], tailY[1000];
    int nTail;
    int score;
    eDirection dir;
    eDirection lastDir;
    int lastTailX, lastTailY;
    bool isAlive;
};

// Single source of truth: how many snakes are in play (1 or 2, chosen at the menu).
const int MAX_SNAKES = 2;
int numSnakes = 1;
Snake snakes[MAX_SNAKES];

// FILE I/O FUNCTIONS
void LoadHighScore()
{
    ifstream fileIn("highscore.txt");
    if (fileIn.is_open())
    {
        fileIn >> highScore;
        fileIn.ignore();
        getline(fileIn, highScorePlayerName);
        if (highScorePlayerName.empty())
            highScorePlayerName = "Player";
        fileIn.close();
    }
    else
    {
        highScore = 0;
        highScorePlayerName = "Player";
    }
}

void SaveHighScore()
{
    ofstream fileOut("highscore.txt");
    if (fileOut.is_open())
    {
        fileOut << highScore << endl;
        fileOut << highScorePlayerName;
        fileOut.close();
    }
}

// GET PLAYER NAME FUNCTION
string GetPlayerName()
{
    ClearScreen();
    SetColor(YELLOW);
    GotoXY(width / 2 - 12, 8);
    cout << "==========================";
    GotoXY(width / 2 - 12, 9);
    cout << "   NEW HIGH SCORE!!!   ";
    GotoXY(width / 2 - 12, 10);
    cout << "==========================";
    
    SetColor(GREEN);
    GotoXY(width / 2 - 10, 12);
    cout << "Score: " << snakes[0].score;
    
    SetColor(CYAN);
    GotoXY(width / 2 - 15, 15);
    cout << "Enter your name (max 20 chars):";
    
    GotoXY(width / 2 - 10, 17);
    SetColor(WHITE);
    
    // Enable cursor
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
    cursorInfo.bVisible = true;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
    
    string name = "";
    char ch;
    
    while (true)
    {
        ch = _getch();
        
        if (ch == 13 && name.length() > 0) // Enter key
        {
            break;
        }
        else if (ch == 8 && name.length() > 0) // Backspace
        {
            name.pop_back();
            GotoXY(width / 2 - 10, 17);
            cout << string(20, ' ');
            GotoXY(width / 2 - 10, 17);
            cout << name;
        }
        else if (name.length() < 20 && ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || 
                 (ch >= '0' && ch <= '9') || ch == ' ' || ch == '_'))
        {
            name += ch;
            GotoXY(width / 2 - 10, 17);
            cout << name;
        }
    }
    
    // Disable cursor again
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
    
    ResetColor();
    return name.empty() ? "Player" : name;
}

// HELP PAGE
void ShowHelpPage()
{
    ClearScreen();
    SetColor(CYAN);
    GotoXY(width / 2 - 10, 2);
    cout << "====================";
    GotoXY(width / 2 - 10, 3);
    cout << "   GAME HELP PAGE  ";
    GotoXY(width / 2 - 10, 4);
    cout << "====================";
    
    SetColor(YELLOW);
    GotoXY(5, 6);
    cout << "OBJECTIVE:";
    SetColor(WHITE);
    GotoXY(5, 7);
    cout << "  Eat fruits to grow your snake and score points!";
    GotoXY(5, 8);
    cout << "  Avoid hitting walls (if Border Wrap is OFF) and yourself.";
    
    SetColor(YELLOW);
    GotoXY(5, 10);
    cout << "CONTROLS:";
    SetColor(WHITE);
    GotoXY(5, 11);
    cout << "  Arrow Keys or WASD - Move the snake";
    GotoXY(5, 12);
    cout << "  P - Pause the game";
    GotoXY(5, 13);
    cout << "  X - Exit to main menu";
    GotoXY(5, 14);
    cout << "  M - Toggle music on/off";
    
    SetColor(YELLOW);
    GotoXY(5, 16);
    cout << "SPECIAL ITEMS:";
    SetColor(RED);
    GotoXY(5, 17);
    cout << "  ♥ Red Heart - Normal fruit (+10 points, grows snake)";
    SetColor(BLUE);
    GotoXY(5, 18);
    cout << "  ♦ Blue Diamond - Slow Time effect (temporary speed reduction)";
    
    SetColor(YELLOW);
    GotoXY(5, 20);
    cout << "FEATURES:";
    SetColor(WHITE);
    GotoXY(5, 21);
    cout << "  - Speed increases every 5 fruits eaten";
    GotoXY(5, 22);
    cout << "  - Rainbow Trail mode for colorful snake";
    GotoXY(5, 23);
    cout << "  - Border Wrap to teleport through walls";
    GotoXY(5, 24);
    cout << "  - High score tracking with player names";
    
    SetColor(GREY);
    GotoXY(width / 2 - 15, 27);
    cout << "Press ESC to return to menu";
    ResetColor();
    
    while (true)
    {
        if (kbhit())
        {
            char key = getch();
            if (key == 27) break; // ESC
        }
        Sleep(50);
    }
}

// HIGH SCORE PAGE
void ShowHighScorePage()
{
    ClearScreen();
    SetColor(YELLOW);
    GotoXY(width / 2 - 10, 5);
    cout << "====================";
    GotoXY(width / 2 - 10, 6);
    cout << "    HIGH SCORE     ";
    GotoXY(width / 2 - 10, 7);
    cout << "====================";
    
    SetColor(GREEN);
    GotoXY(width / 2 - 10, 10);
    cout << "Player: " << highScorePlayerName;
    
    SetColor(CYAN);
    GotoXY(width / 2 - 10, 12);
    cout << "Score: " << highScore;
    
    SetColor(GREY);
    GotoXY(width / 2 - 15, 18);
    cout << "Press ESC to return to menu";
    ResetColor();
    
    while (true)
    {
        if (kbhit())
        {
            char key = getch();
            if (key == 27) break;
        }
        Sleep(50);
    }
}

// SETTINGS PAGE
void ShowSettingsPage()
{
    ClearScreen();
    int selectedOption = 0;
    int lastSelectedOption = -1;
    const int numOptions = 3;
    
    while (true)
    {
        if (selectedOption != lastSelectedOption)
        {
            lastSelectedOption = selectedOption;
            SetColor(CYAN);
            GotoXY(width / 2 - 10, 5);
            cout << "====================";
            GotoXY(width / 2 - 10, 6);
            cout << "     SETTINGS      ";
            GotoXY(width / 2 - 10, 7);
            cout << "====================";
            
            GotoXY(width / 2 - 12, 11);
            if (selectedOption == 0)
            { SetColor(BLACK, WHITE); cout << " > Border Wrap: " << (borderWrap ? "ON " : "OFF") << " < "; }
            else
            { SetColor(WHITE); cout << "   Border Wrap: " << (borderWrap ? "ON " : "OFF") << "   "; }
            
            GotoXY(width / 2 - 12, 13);
            if (selectedOption == 1)
            { SetColor(BLACK, WHITE); cout << " > Music: " << (musicEnabled ? "ON " : "OFF") << "        < "; }
            else
            { SetColor(WHITE); cout << "   Music: " << (musicEnabled ? "ON " : "OFF") << "          "; }
            
            GotoXY(width / 2 - 12, 15);
            if (selectedOption == 2)
            { SetColor(BLACK, WHITE); cout << " > Rainbow Trail: " << (rainbowTrail ? "ON " : "OFF") << " < "; }
            else
            { SetColor(WHITE); cout << "   Rainbow Trail: " << (rainbowTrail ? "ON " : "OFF") << "   "; }
            
            SetColor(GREY);
            GotoXY(width / 2 - 18, 19);
            cout << "Use ↑↓ to navigate, ENTER to toggle";
            GotoXY(width / 2 - 15, 20);
            cout << "Press ESC to return to menu";
            ResetColor();
        }
        
        if (kbhit())
        {
            char key = getch();
            if (key == 0 || key == -32)
            {
                key = getch();
                switch (key)
                {
                    case 72: selectedOption = (selectedOption - 1 + numOptions) % numOptions; break;
                    case 80: selectedOption = (selectedOption + 1) % numOptions; break;
                }
            }
            else if (key == 13)
            {
                switch (selectedOption)
                {
                    case 0: borderWrap = !borderWrap; break;
                    case 1: 
                        ToggleMusic(); 
                        if (musicEnabled) PlayMenuMusic();
                        else StopAllMusic();
                        break;
                    case 2: rainbowTrail = !rainbowTrail; break;
                }
            }
            else if (key == 27)
            {
                return;
            }
        }
        Sleep(50);
    }
}

// MAIN MENU PAGE
int ShowMenu()
{
    ClearScreen();
    PlayMenuMusic();
    int selectedOption = 0;
    int lastSelectedOption = -1;
    const int numOptions = 6;
    string menuOptions[numOptions] = { "1 Player", "2 Player", "Help", "Settings", "High Score", "Quit Game" };
    
    while (true)
    {
        if (selectedOption != lastSelectedOption)
        {
            lastSelectedOption = selectedOption;
            SetColor(GREEN);
            GotoXY(width / 2 - 10, 5);
            cout << "====================";
            GotoXY(width / 2 - 10, 6);
            cout << "   SNAKE GAME++    ";
            GotoXY(width / 2 - 10, 7);
            cout << "====================";
            
            SetColor(YELLOW);
            GotoXY(width / 2 - 10, 9);
            cout << "High Score: " << highScore;
            
            for (int i = 0; i < numOptions; i++)
            {
                GotoXY(width / 2 - 10, 12 + i * 2);
                if (i == selectedOption)
                {
                    SetColor(BLACK, WHITE);
                    cout << " > " << menuOptions[i];
                    int padding = 15 - menuOptions[i].length();
                    for (int j = 0; j < padding; j++) cout << " ";
                    cout << " < ";
                }
                else
                {
                    SetColor(CYAN);
                    cout << "   " << menuOptions[i];
                    int padding = 15 - menuOptions[i].length();
                    for (int j = 0; j < padding; j++) cout << " ";
                    cout << "   ";
                }
            }
            
            SetColor(MAGENTA);
            GotoXY(width / 2 - 15, 24);
            cout << "Red Heart = +10 points";
            GotoXY(width / 2 - 15, 25);
            cout << "Blue Diamond = Slow Time";
            
            SetColor(GREY);
            GotoXY(width / 2 - 18, 27);
            cout << "Use UP/DOWN to navigate, ENTER to select";
            GotoXY(width / 2 - 10, 28);
            cout << "Music: " << (musicEnabled ? "ON" : "OFF");
            ResetColor();
        }
        
        if (kbhit())
        {
            char key = getch();
            if (key == 0 || key == -32)
            {
                key = getch();
                switch (key)
                {
                    case 72: selectedOption = (selectedOption - 1 + numOptions) % numOptions; break;
                    case 80: selectedOption = (selectedOption + 1) % numOptions; break;
                }
            }
            else if (key == 13) { return selectedOption; }
            else if (key == 'm' || key == 'M')
            {
                ToggleMusic();
                if (musicEnabled) PlayMenuMusic();
            }
        }
        Sleep(50);
    }
}

// PAUSE/SETTINGS OVERLAY
void ShowPauseOverlay()
{
    int selectedOption = 0;
    int lastSelectedOption = -1;
    const int numOptions = 4;
    string options[numOptions] = { "Resume Game", "Border Wrap", "Music", "Rainbow Trail" };

    while (true)
    {
        if (selectedOption != lastSelectedOption)
        {
            lastSelectedOption = selectedOption;
            SetColor(GREY, BLACK);
            for (int y = height / 2 - 4; y <= height / 2 + 8; ++y)
            {
                GotoXY(width / 2 - 15, y);
                cout << string(30, ' ');
            }
            SetColor(CYAN, BLACK);
            GotoXY(width / 2 - 7, height / 2 - 3);
            cout << "=== PAUSED ===";
            for (int i = 0; i < numOptions; i++)
            {
                GotoXY(width / 2 - 8, height / 2 - 1 + i * 2);
                if (i == selectedOption)
                {
                    SetColor(BLACK, WHITE); cout << "> ";
                }
                else
                {
                    SetColor(WHITE, BLACK); cout << "  ";
                }
                cout << options[i];
                if (i == 1) cout << ": " << (borderWrap ? "ON " : "OFF");
                if (i == 2) cout << ": " << (musicEnabled ? "ON " : "OFF");
                if (i == 3) cout << ": " << (rainbowTrail ? "ON " : "OFF");
            }
            SetColor(GREY, BLACK);
            GotoXY(width / 2 - 11, height / 2 + 7);
            cout << "↑/↓: Navigate  Enter: Toggle";
            GotoXY(width / 2 - 8, height / 2 + 8);
            cout << "R or ESC: Resume game";
            ResetColor();
        }
        if (kbhit())
        {
            char key = getch();
            if (key == 0 || key == -32)
            {
                key = getch();
                switch (key)
                {
                    case 72: selectedOption = (selectedOption - 1 + numOptions) % numOptions; break;
                    case 80: selectedOption = (selectedOption + 1) % numOptions; break;
                }
            }
            else if ((key == 13 && selectedOption == 0) || key == 27 || key == 'r' || key == 'R')
            {
                ClearScreen();
                Draw();
                return;
            }
            else if (key == 13)
            {
                switch (selectedOption)
                {
                    case 1: borderWrap = !borderWrap; break;
                    case 2: 
                        ToggleMusic(); 
                        if (musicEnabled) PlayGameMusic();
                        else StopAllMusic();
                        break;
                    case 3: rainbowTrail = !rainbowTrail; break;
                    default: break;
                }
            }
        }
        Sleep(50);
    }
}

// GAME LOGIC AND DRAWING
void Setup()
{
    SetConsoleOutputCP(65001);
    srand(static_cast<unsigned int>(time(0)));
    gameOver = false;

    for (int s = 0; s < numSnakes; s++)
    {
        snakes[s].isAlive = true;
        snakes[s].score = 0;
        snakes[s].nTail = 2;
        snakes[s].headY = height / 2;
        snakes[s].lastTailX = -1;
        snakes[s].lastTailY = -1;
        if (s == 0)
        {
            // Player 1 starts on the left, heading right; tail trails to the left.
            snakes[s].dir = RIGHT;
            snakes[s].lastDir = RIGHT;
            snakes[s].headX = (numSnakes == 2) ? (width / 4) : (width / 2);
            snakes[s].tailX[0] = snakes[s].headX - 1; snakes[s].tailY[0] = snakes[s].headY;
            snakes[s].tailX[1] = snakes[s].headX - 2; snakes[s].tailY[1] = snakes[s].headY;
        }
        else
        {
            // Player 2 starts on the right, heading left; tail trails to the right.
            snakes[s].dir = LEFT;
            snakes[s].lastDir = LEFT;
            snakes[s].headX = (3 * width) / 4;
            snakes[s].tailX[0] = snakes[s].headX + 1; snakes[s].tailY[0] = snakes[s].headY;
            snakes[s].tailX[1] = snakes[s].headX + 2; snakes[s].tailY[1] = snakes[s].headY;
        }
    }

    fruitX = rand() % width;
    fruitY = rand() % height;

    bool validPos = false;
    while (!validPos)
    {
        validPos = true;
        for (int s = 0; s < numSnakes; s++)
        {
            if (fruitX == snakes[s].headX && fruitY == snakes[s].headY)
                validPos = false;
            for (int i = 0; i < snakes[s].nTail; i++)
                if (snakes[s].tailX[i] == fruitX && snakes[s].tailY[i] == fruitY)
                    validPos = false;
        }
        if (!validPos)
        {
            fruitX = rand() % width;
            fruitY = rand() % height;
        }
    }

    speedLevel = 1;
    gameSpeed = originalGameSpeed;
    frameCount = 0;
    isSpecialFruitOnScreen = false;
    isSlowTimeActive = false; 
    slowTimeDuration = 0;
    fruitsEaten = 0;
    
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
    ClearScreen();
    SetColor(DARKGREY);
    for (int i = 0; i < width + 2; i++)
    {
        GotoXY(i, 0); cout << "█";
        GotoXY(i, height + 1); cout << "█";
    }
    for (int i = 0; i < height + 2; i++)
    {
        GotoXY(0, i); cout << "█";
        GotoXY(width + 1, i); cout << "█";
    }
    ResetColor();
    StopMenuMusic();
    PlayGameMusic();
}

void Draw()
{
    SetColor(DARKGREY);
    for (int i = 0; i < width + 2; i++)
    {
        GotoXY(i, 0);
        cout << "█";
        GotoXY(i, height + 1);
        cout << "█";
    }
    for (int i = 0; i < height + 2; i++)
    {
        GotoXY(0, i);
        cout << "█";
        GotoXY(width + 1, i);
        cout << "█";
    }
    ResetColor();

    for (int s = 0; s < numSnakes; s++)
    {
        if (snakes[s].lastTailX != -1)
        {
            GotoXY(snakes[s].lastTailX + 1, snakes[s].lastTailY + 1);
            cout << " ";
            snakes[s].lastTailX = -1;
        }
    }
    for (int s = 0; s < numSnakes; s++)
    {
        if (!snakes[s].isAlive) continue;
        GotoXY(snakes[s].headX + 1, snakes[s].headY + 1);
        char headChar;
        if (snakes[s].dir == UP) headChar = '^';
        else if (snakes[s].dir == DOWN) headChar = 'v';
        else if (snakes[s].dir == LEFT) headChar = '<';
        else if (snakes[s].dir == RIGHT) headChar = '>';
        else headChar = '@';
        if (s == 0)
        {
            if (frameCount % 2 == 0) SetColor(GREEN, BLACK);
            else if (isSlowTimeActive) SetColor(CYAN, BLACK);
            else SetColor(YELLOW, BLACK);
        }
        else
        {
            if (frameCount % 2 == 0) SetColor(CYAN, BLACK);
            else SetColor(MAGENTA, BLACK);
        }
        cout << headChar;
    }
    frameCount++;
    for (int s = 0; s < numSnakes; s++)
    {
        if (!snakes[s].isAlive || snakes[s].nTail <= 0) continue;
        if (s == 0)
        {
            for (int i = 0; i < snakes[s].nTail; i++)
            {
                GotoXY(snakes[s].tailX[i] + 1, snakes[s].tailY[i] + 1);
                if (rainbowTrail)
                {
                    int colorCycle = (rainbowPhase + i) % 6;
                    switch (colorCycle)
                    {
                    case 0: SetColor(GREEN); break;
                    case 1: SetColor(CYAN); break;
                    case 2: SetColor(BLUE); break;
                    case 3: SetColor(MAGENTA); break;
                    case 4: SetColor(RED); break;
                    case 5: SetColor(YELLOW); break;
                    }
                }
                else SetColor(DARKGREEN);
                cout << "o";
            }
            ResetColor();
            rainbowPhase = (rainbowPhase + 1) % 6;
        }
        else
        {
            for (int i = 0; i < snakes[s].nTail; i++)
            {
                GotoXY(snakes[s].tailX[i] + 1, snakes[s].tailY[i] + 1);
                SetColor(CYAN);
                cout << "x";
            }
            ResetColor();
        }
    }
    ResetColor();
    GotoXY(fruitX + 1, fruitY + 1); 
    SetColor(RED); 
    cout << "♥"; 
    ResetColor();
    if (isSpecialFruitOnScreen) 
    { 
        GotoXY(specialFruitX + 1, specialFruitY + 1); 
        SetColor(BLUE); 
        cout << "♦"; 
        ResetColor(); 
    }
    GotoXY(0, height + 2);
    if (numSnakes == 1)
    {
        SetColor(YELLOW);
        cout << "Score: " << snakes[0].score << " | High Score: " << highScore;
        cout << " | Speed Level: " << speedLevel;
        if (isSlowTimeActive)
        {
            SetColor(CYAN);
            cout << " | STATUS: SLOW TIME (" << slowTimeDuration / (1000 / originalGameSpeed) << "s)";
        }
        SetColor(GREY);
        cout << " | Music: " << (musicEnabled ? "ON" : "OFF") << "  ";
    }
    else
    {
        SetColor(GREEN);
        cout << "P1: " << snakes[0].score << "  ";
        SetColor(CYAN);
        cout << "P2: " << snakes[1].score << "  ";
        SetColor(YELLOW);
        cout << "| High: " << highScore << " | Lvl " << speedLevel;
        if (isSlowTimeActive) { SetColor(CYAN); cout << " | SLOW"; }
        SetColor(GREY);
        cout << " | Music: " << (musicEnabled ? "ON" : "OFF") << "  ";
    }
    ResetColor();
}

void Input()
{
    if (kbhit())
    {
        char key = getch();
        if (key == 0 || key == -32)
        {
            key = getch();
            switch (key)
            {
            case 72: if (snakes[0].lastDir != DOWN) snakes[0].dir = UP; break;
            case 80: if (snakes[0].lastDir != UP) snakes[0].dir = DOWN; break;
            case 75: if (snakes[0].lastDir != RIGHT) snakes[0].dir = LEFT; break;
            case 77: if (snakes[0].lastDir != LEFT) snakes[0].dir = RIGHT; break;
            }
        }
        else
        {
            char lowerKey = tolower(key);
            int wasdSnake = (numSnakes == 2) ? 1 : 0;  // WASD drives player 2 in 2P, player 1 otherwise
            switch (lowerKey)
            {
            case 'w': if (snakes[wasdSnake].lastDir != DOWN) snakes[wasdSnake].dir = UP; break;
            case 's': if (snakes[wasdSnake].lastDir != UP) snakes[wasdSnake].dir = DOWN; break;
            case 'a': if (snakes[wasdSnake].lastDir != RIGHT) snakes[wasdSnake].dir = LEFT; break;
            case 'd': if (snakes[wasdSnake].lastDir != LEFT) snakes[wasdSnake].dir = RIGHT; break;
            case 'x': gameOver = true; break;
            case 'p': ShowPauseOverlay(); ClearScreen(); Draw(); break;
            case 'm': ToggleMusic(); if (musicEnabled) PlayGameMusic(); break;
            }
        }
    }
}

void SpawnSpecialFruit()
{
    if (isSpecialFruitOnScreen) return;
    if (rand() % SPECIAL_FRUIT_SPAWN_CHANCE == 0)
    {
        isSpecialFruitOnScreen = true;
        bool validPos;
        do
        {
            validPos = true;
            specialFruitX = rand() % width;
            specialFruitY = rand() % height;
            if (specialFruitX == fruitX && specialFruitY == fruitY)
                validPos = false;
            for (int s = 0; s < numSnakes; s++)
            {
                if (specialFruitX == snakes[s].headX && specialFruitY == snakes[s].headY)
                    validPos = false;
                for (int i = 0; i < snakes[s].nTail; i++)
                    if (snakes[s].tailX[i] == specialFruitX && snakes[s].tailY[i] == specialFruitY)
                        validPos = false;
            }
        } while (!validPos);
    }
}

void ApplySlowTimeEffect()
{
    isSlowTimeActive = true;
    slowTimeDuration = SLOW_TIME_FRAMES;
    gameSpeed = originalGameSpeed;
}

void Logic()
{
    for (int s = 0; s < numSnakes; s++)
        if (snakes[s].isAlive) snakes[s].lastDir = snakes[s].dir;

    if (isSlowTimeActive)
    {
        slowTimeDuration--;
        if (slowTimeDuration <= 0)
        {
            isSlowTimeActive = false;
            gameSpeed = originalGameSpeed;
        }
    }

    int topScore = 0;
    for (int s = 0; s < numSnakes; s++)
        topScore = max(topScore, snakes[s].score);
    originalGameSpeed = max(120 - (topScore / 50) * 10, 10);
    if (!isSlowTimeActive) gameSpeed = originalGameSpeed;

    // Advance every living snake: shift its tail, then move its head.
    for (int s = 0; s < numSnakes; s++)
    {
        if (!snakes[s].isAlive) continue;
        if (snakes[s].nTail > 0)
        {
            snakes[s].lastTailX = snakes[s].tailX[snakes[s].nTail - 1];
            snakes[s].lastTailY = snakes[s].tailY[snakes[s].nTail - 1];
        }
        else
        {
            snakes[s].lastTailX = snakes[s].headX;
            snakes[s].lastTailY = snakes[s].headY;
        }
        for (int i = snakes[s].nTail - 1; i > 0; i--)
        {
            snakes[s].tailX[i] = snakes[s].tailX[i - 1];
            snakes[s].tailY[i] = snakes[s].tailY[i - 1];
        }
        if (snakes[s].nTail > 0)
        {
            snakes[s].tailX[0] = snakes[s].headX;
            snakes[s].tailY[0] = snakes[s].headY;
        }

        switch (snakes[s].dir)
        {
            case LEFT: snakes[s].headX--; break;
            case RIGHT: snakes[s].headX++; break;
            case UP: snakes[s].headY--; break;
            case DOWN: snakes[s].headY++; break;
            case STOP: snakes[s].lastTailX = -1; snakes[s].lastTailY = -1; break;
        }
    }

    // Wall and self collision for every living snake.
    for (int s = 0; s < numSnakes; s++)
    {
        if (!snakes[s].isAlive) continue;
        if (borderWrap)
        {
            if (snakes[s].headX < 0) snakes[s].headX = width - 1;
            else if (snakes[s].headX >= width) snakes[s].headX = 0;
            if (snakes[s].headY < 0) snakes[s].headY = height - 1;
            else if (snakes[s].headY >= height) snakes[s].headY = 0;
        }
        else
        {
            if (snakes[s].headX < 0 || snakes[s].headX >= width || snakes[s].headY < 0 || snakes[s].headY >= height)
                snakes[s].isAlive = false;
        }
        for (int i = 0; i < snakes[s].nTail; i++)
        {
            if (snakes[s].tailX[i] == snakes[s].headX && snakes[s].tailY[i] == snakes[s].headY)
                snakes[s].isAlive = false;
        }
    }

    // Snake-versus-snake collision (2-player only).
    if (numSnakes == 2 && snakes[0].isAlive && snakes[1].isAlive)
    {
        if (snakes[0].headX == snakes[1].headX && snakes[0].headY == snakes[1].headY)
        {
            snakes[0].isAlive = false;
            snakes[1].isAlive = false;
        }
        for (int i = 0; i < snakes[1].nTail; i++)
            if (snakes[1].tailX[i] == snakes[0].headX && snakes[1].tailY[i] == snakes[0].headY)
                snakes[0].isAlive = false;
        for (int i = 0; i < snakes[0].nTail; i++)
            if (snakes[0].tailX[i] == snakes[1].headX && snakes[0].tailY[i] == snakes[1].headY)
                snakes[1].isAlive = false;
    }

    // The round ends as soon as any snake in play has died.
    for (int s = 0; s < numSnakes; s++)
        if (!snakes[s].isAlive) gameOver = true;
    if (gameOver) return;

    // Fruit eating: the first living snake on the fruit eats it.
    bool fruitEaten = false;
    for (int s = 0; s < numSnakes && !fruitEaten; s++)
    {
        if (snakes[s].isAlive && snakes[s].headX == fruitX && snakes[s].headY == fruitY)
        {
            snakes[s].score += 10;
            snakes[s].nTail++;
            fruitEaten = true;
        }
    }
    if (fruitEaten)
    {
        fruitsEaten++;
        if (fruitsEaten % 5 == 0 && originalGameSpeed > 40)
        {
            originalGameSpeed -= 10;
            if (!isSlowTimeActive) gameSpeed = originalGameSpeed;
            speedLevel++;
        }
        SpawnSpecialFruit();
        bool validPos;
        do
        {
            validPos = true;
            fruitX = rand() % width;
            fruitY = rand() % height;
            if ((fruitX == specialFruitX && fruitY == specialFruitY) && isSpecialFruitOnScreen)
                validPos = false;
            for (int s = 0; s < numSnakes; s++)
            {
                if (fruitX == snakes[s].headX && fruitY == snakes[s].headY) validPos = false;
                for (int i = 0; i < snakes[s].nTail; i++)
                    if (snakes[s].tailX[i] == fruitX && snakes[s].tailY[i] == fruitY) validPos = false;
            }
        } while (!validPos);
    }

    // Special fruit: any living snake reaching it triggers slow time.
    if (isSpecialFruitOnScreen)
    {
        bool specialEaten = false;
        for (int s = 0; s < numSnakes; s++)
            if (snakes[s].isAlive && snakes[s].headX == specialFruitX && snakes[s].headY == specialFruitY) specialEaten = true;
        if (specialEaten)
        {
            isSpecialFruitOnScreen = false;
            ApplySlowTimeEffect();
            GotoXY(specialFruitX + 1, specialFruitY + 1);
            cout << " ";
            specialFruitX = -1;
            specialFruitY = -1;
        }
    }
}

// Main Program
int main()
{
    LoadHighScore();
    while (true)
    {
        int menuChoice = ShowMenu();
        switch (menuChoice)
        {
        case 0: // 1 Player
            numSnakes = 1;
            Setup();
            while (!gameOver)
            {
                Input();
                Logic();
                Draw();
                Sleep(gameSpeed);
            }
            StopGameMusic();
            PlayMenuMusic();

            ClearScreen();
            SetColor(RED);
            GotoXY(width / 2 - 5, height / 2 - 2);
            cout << "GAME OVER!";
            SetColor(YELLOW);
            GotoXY(width / 2 - 8, height / 2);
            cout << "Final Score: " << snakes[0].score;
            if (snakes[0].score > highScore && snakes[0].score > 0)
            {
                highScore = snakes[0].score;
                highScorePlayerName = GetPlayerName();
                SaveHighScore();
            }
            else
            {
                SetColor(CYAN);
                GotoXY(width / 2 - 12, height / 2 + 1);
                cout << "Current High Score: " << highScore;
                SetColor(GREY);
                GotoXY(width / 2 - 12, height / 2 + 3);
                cout << "Press any key to continue...";
                ResetColor();
                getch();
            }
            break;
        case 1: // 2 Player
        {
            numSnakes = 2;
            Setup();
            while (!gameOver)
            {
                Input();
                Logic();
                Draw();
                Sleep(gameSpeed);
            }
            StopGameMusic();
            PlayMenuMusic();

            ClearScreen();
            SetColor(YELLOW);
            GotoXY(width / 2 - 10, height / 2 - 4);
            cout << "====================";
            GotoXY(width / 2 - 10, height / 2 - 3);
            if (snakes[0].isAlive && !snakes[1].isAlive)
            {
                SetColor(GREEN);
                cout << "  PLAYER 2 LOST!   ";
            }
            else if (snakes[1].isAlive && !snakes[0].isAlive)
            {
                SetColor(CYAN);
                cout << "  PLAYER 1 LOST!   ";
            }
            else
            {
                SetColor(YELLOW);
                cout << "    IT'S A DRAW!   ";
            }
            SetColor(YELLOW);
            GotoXY(width / 2 - 10, height / 2 - 2);
            cout << "====================";

            SetColor(GREEN);
            GotoXY(width / 2 - 10, height / 2);
            cout << "Player 1 Score: " << snakes[0].score;
            SetColor(CYAN);
            GotoXY(width / 2 - 10, height / 2 + 1);
            cout << "Player 2 Score: " << snakes[1].score;

            int maxScore = max(snakes[0].score, snakes[1].score);
            if (maxScore > highScore && maxScore > 0)
            {
                highScore = maxScore;
                highScorePlayerName = GetPlayerName();
                SaveHighScore();
            }
            else
            {
                SetColor(GREY);
                GotoXY(width / 2 - 12, height / 2 + 3);
                cout << "Press any key to continue...";
                ResetColor();
                getch();
            }
            break;
        }
        case 2: ShowHelpPage(); break;
        case 3: ShowSettingsPage(); break;
        case 4: ShowHighScorePage(); break;
        case 5: StopAllMusic(); return 0;
        }
    }
    return 0;
}
