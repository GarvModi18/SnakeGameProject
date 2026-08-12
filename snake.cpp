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

enum GameMode
{
    MODE_SINGLE = 0,
    MODE_MULTI = 1
};

enum eDirection
{
    STOP = 0,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

struct SnakePlayer
{
    int headX, headY;
    int tailX[1000], tailY[1000];
    int nTail;
    int score;
    eDirection dir;
    eDirection lastDir;
    bool isAlive;
    int lastTailX, lastTailY;
};

// Global Settings
bool borderWrap = false;
bool rainbowTrail = true;
int rainbowPhase = 0;
GameMode gameMode = MODE_SINGLE;

// Global Game Variables
bool gameOver;
const int width = 40;
const int height = 20;
int fruitX, fruitY;

// Players
SnakePlayer p1, p2;

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

// MUSIC VARIABLES
bool musicEnabled = true;
string menuMusicFile = "menu.mp3";
string gameMusicFile = "game.mp3";

// Function Declarations
void SetColor(ConsoleColor textColor, ConsoleColor bgColor = BLACK);
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
string GetPlayerName(string defaultName = "Player", string titleStr = "NEW HIGH SCORE!");

// Console Utility Functions
void SetColor(ConsoleColor textColor, ConsoleColor bgColor)
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
string GetPlayerName(string defaultName, string titleStr)
{
    ClearScreen();
    SetColor(YELLOW);
    GotoXY(width / 2 - 12, 8);
    cout << "==========================";
    GotoXY(width / 2 - 12, 9);
    cout << "   " << titleStr << "   ";
    GotoXY(width / 2 - 12, 10);
    cout << "==========================";
    
    SetColor(CYAN);
    GotoXY(width / 2 - 15, 14);
    cout << "Enter player name (max 20 chars):";
    
    GotoXY(width / 2 - 10, 16);
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
            GotoXY(width / 2 - 10, 16);
            cout << string(20, ' ');
            GotoXY(width / 2 - 10, 16);
            cout << name;
        }
        else if (name.length() < 20 && ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || 
                 (ch >= '0' && ch <= '9') || ch == ' ' || ch == '_'))
        {
            name += ch;
            GotoXY(width / 2 - 10, 16);
            cout << name;
        }
    }
    
    // Disable cursor again
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
    
    ResetColor();
    return name.empty() ? defaultName : name;
}

// HELP PAGE
void ShowHelpPage()
{
    ClearScreen();
    SetColor(CYAN);
    GotoXY(width / 2 - 10, 1);
    cout << "====================";
    GotoXY(width / 2 - 10, 2);
    cout << "   GAME HELP PAGE  ";
    GotoXY(width / 2 - 10, 3);
    cout << "====================";
    
    SetColor(YELLOW);
    GotoXY(3, 5);
    cout << "OBJECTIVE:";
    SetColor(WHITE);
    GotoXY(3, 6);
    cout << "  Eat fruits to grow your snake and score points!";
    GotoXY(3, 7);
    cout << "  In 2P mode, outlast your opponent or get the higher score!";
    
    SetColor(YELLOW);
    GotoXY(3, 9);
    cout << "CONTROLS:";
    SetColor(GREEN);
    GotoXY(3, 10);
    cout << "  Player 1: Arrow keys (Green Snake)";
    SetColor(CYAN);
    GotoXY(3, 11);
    cout << "  Player 2: WASD keys (Cyan Snake - 2P Mode)";
    SetColor(WHITE);
    GotoXY(3, 12);
    cout << "  System Controls: P (Pause), M (Toggle Music), X (Exit round)";
    
    SetColor(YELLOW);
    GotoXY(3, 14);
    cout << "SPECIAL ITEMS:";
    SetColor(RED);
    GotoXY(3, 15);
    cout << "  ♥ Red Heart - Normal fruit (+10 points, grows snake)";
    SetColor(BLUE);
    GotoXY(3, 16);
    cout << "  ♦ Blue Diamond - Slow Time effect";
    
    SetColor(YELLOW);
    GotoXY(3, 18);
    cout << "MULTIPLAYER RULES:";
    SetColor(WHITE);
    GotoXY(3, 19);
    cout << "  - Avoid hitting walls (if Border Wrap is OFF)";
    GotoXY(3, 20);
    cout << "  - Avoid hitting your own body or opponent's body";
    GotoXY(3, 21);
    cout << "  - Head-on collision eliminates both players!";
    
    SetColor(GREY);
    GotoXY(width / 2 - 15, 24);
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
    string menuOptions[numOptions] = { "1 Player Mode", "2 Player Mode", "Help", "Settings", "High Score", "Quit Game" };
    
    while (true)
    {
        if (selectedOption != lastSelectedOption)
        {
            lastSelectedOption = selectedOption;
            SetColor(GREEN);
            GotoXY(width / 2 - 10, 4);
            cout << "====================";
            GotoXY(width / 2 - 10, 5);
            cout << "   SNAKE GAME++    ";
            GotoXY(width / 2 - 10, 6);
            cout << "====================";
            
            SetColor(YELLOW);
            GotoXY(width / 2 - 10, 8);
            cout << "High Score: " << highScore;
            
            for (int i = 0; i < numOptions; i++)
            {
                GotoXY(width / 2 - 10, 11 + i * 2);
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
            cout << "P1: Arrow Keys  |  P2: WASD";
            GotoXY(width / 2 - 15, 25);
            cout << "Red Heart: +10 | Blue Diamond: Slow Time";
            
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
    
    // Player 1 Setup
    p1.isAlive = true;
    p1.dir = RIGHT;
    p1.lastDir = RIGHT;
    p1.headX = (gameMode == MODE_MULTI) ? (width / 4) : (width / 2);
    p1.headY = height / 2;
    p1.nTail = 2;
    p1.tailX[0] = p1.headX - 1; p1.tailY[0] = p1.headY;
    p1.tailX[1] = p1.headX - 2; p1.tailY[1] = p1.headY;
    p1.score = 0;
    p1.lastTailX = -1; p1.lastTailY = -1;

    // Player 2 Setup
    if (gameMode == MODE_MULTI)
    {
        p2.isAlive = true;
        p2.dir = LEFT;
        p2.lastDir = LEFT;
        p2.headX = (3 * width) / 4;
        p2.headY = height / 2;
        p2.nTail = 2;
        p2.tailX[0] = p2.headX + 1; p2.tailY[0] = p2.headY;
        p2.tailX[1] = p2.headX + 2; p2.tailY[1] = p2.headY;
        p2.score = 0;
        p2.lastTailX = -1; p2.lastTailY = -1;
    }
    else
    {
        p2.isAlive = false;
        p2.nTail = 0;
    }
    
    // Spawn Fruit
    bool validPos = false;
    while (!validPos)
    {
        validPos = true;
        fruitX = rand() % width; 
        fruitY = rand() % height;
        
        if (fruitX == p1.headX && fruitY == p1.headY) validPos = false;
        for (int i = 0; i < p1.nTail; i++)
            if (p1.tailX[i] == fruitX && p1.tailY[i] == fruitY) validPos = false;
            
        if (gameMode == MODE_MULTI)
        {
            if (fruitX == p2.headX && fruitY == p2.headY) validPos = false;
            for (int i = 0; i < p2.nTail; i++)
                if (p2.tailX[i] == fruitX && p2.tailY[i] == fruitY) validPos = false;
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
        GotoXY(i, 0); cout << "█";
        GotoXY(i, height + 1); cout << "█";
    }
    for (int i = 0; i < height + 2; i++)
    {
        GotoXY(0, i); cout << "█";
        GotoXY(width + 1, i); cout << "█";
    }
    ResetColor();

    // Erase last tail segment P1
    if (p1.lastTailX != -1)
    {
        GotoXY(p1.lastTailX + 1, p1.lastTailY + 1); 
        cout << " "; 
        p1.lastTailX = -1;
    }
    // Erase last tail segment P2
    if (gameMode == MODE_MULTI && p2.lastTailX != -1)
    {
        GotoXY(p2.lastTailX + 1, p2.lastTailY + 1);
        cout << " ";
        p2.lastTailX = -1;
    }

    // Draw P1 Head
    if (p1.isAlive)
    {
        GotoXY(p1.headX + 1, p1.headY + 1);
        char headChar1;
        if (p1.dir == UP) headChar1 = '^';
        else if (p1.dir == DOWN) headChar1 = 'v';
        else if (p1.dir == LEFT) headChar1 = '<';
        else if (p1.dir == RIGHT) headChar1 = '>';
        else headChar1 = '@';

        if (frameCount % 2 == 0) SetColor(GREEN, BLACK);
        else if (isSlowTimeActive) SetColor(CYAN, BLACK);
        else SetColor(YELLOW, BLACK);
        cout << headChar1;
    }

    // Draw P2 Head
    if (gameMode == MODE_MULTI && p2.isAlive)
    {
        GotoXY(p2.headX + 1, p2.headY + 1);
        char headChar2;
        if (p2.dir == UP) headChar2 = '^';
        else if (p2.dir == DOWN) headChar2 = 'v';
        else if (p2.dir == LEFT) headChar2 = '<';
        else if (p2.dir == RIGHT) headChar2 = '>';
        else headChar2 = 'P';

        if (frameCount % 2 == 0) SetColor(CYAN, BLACK);
        else if (isSlowTimeActive) SetColor(MAGENTA, BLACK);
        else SetColor(MAGENTA, BLACK);
        cout << headChar2;
    }
    frameCount++;

    // Draw P1 Tail
    if (p1.isAlive && p1.nTail > 0)
    {
        for (int i = 0; i < p1.nTail; i++)
        {
            GotoXY(p1.tailX[i] + 1, p1.tailY[i] + 1);
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

    // Draw P2 Tail
    if (gameMode == MODE_MULTI && p2.isAlive && p2.nTail > 0)
    {
        for (int i = 0; i < p2.nTail; i++)
        {
            GotoXY(p2.tailX[i] + 1, p2.tailY[i] + 1);
            SetColor(CYAN);
            cout << "x";
        }
        ResetColor();
    }

    // Draw Fruit
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

    // Bottom Status Line
    GotoXY(0, height + 2); 
    if (gameMode == MODE_SINGLE)
    {
        SetColor(YELLOW);
        cout << "Score: " << p1.score << " | High Score: " << highScore;
        cout << " | Speed: Lvl " << speedLevel;
        if (isSlowTimeActive)
        { 
            SetColor(CYAN); 
            cout << " | SLOW TIME"; 
        }
        SetColor(GREY); 
        cout << " | Music: " << (musicEnabled ? "ON " : "OFF");
    }
    else // MODE_MULTI
    {
        SetColor(GREEN);
        cout << "P1 Score: " << p1.score << " ";
        SetColor(WHITE);
        cout << "| ";
        SetColor(CYAN);
        cout << "P2 Score: " << p2.score << " ";
        SetColor(WHITE);
        cout << "| ";
        SetColor(YELLOW);
        cout << "High: " << highScore << " ";
        SetColor(GREY);
        cout << "| Lvl " << speedLevel;
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
            case 72: if (p1.lastDir != DOWN) p1.dir = UP; break;
            case 80: if (p1.lastDir != UP) p1.dir = DOWN; break;
            case 75: if (p1.lastDir != RIGHT) p1.dir = LEFT; break;
            case 77: if (p1.lastDir != LEFT) p1.dir = RIGHT; break;
            }
        }
        else
        {
            char lowerKey = tolower(key);
            if (gameMode == MODE_MULTI)
            {
                switch (lowerKey)
                {
                case 'w': if (p2.lastDir != DOWN) p2.dir = UP; break;
                case 's': if (p2.lastDir != UP) p2.dir = DOWN; break;
                case 'a': if (p2.lastDir != RIGHT) p2.dir = LEFT; break;
                case 'd': if (p2.lastDir != LEFT) p2.dir = RIGHT; break;
                case 'x': gameOver = true; break;
                case 'p': ShowPauseOverlay(); ClearScreen(); Draw(); break;
                case 'm': ToggleMusic(); if (musicEnabled) PlayGameMusic(); break;
                }
            }
            else
            {
                switch (lowerKey)
                {
                case 'w': if (p1.lastDir != DOWN) p1.dir = UP; break;
                case 's': if (p1.lastDir != UP) p1.dir = DOWN; break;
                case 'a': if (p1.lastDir != RIGHT) p1.dir = LEFT; break;
                case 'd': if (p1.lastDir != LEFT) p1.dir = RIGHT; break;
                case 'x': gameOver = true; break;
                case 'p': ShowPauseOverlay(); ClearScreen(); Draw(); break;
                case 'm': ToggleMusic(); if (musicEnabled) PlayGameMusic(); break;
                }
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
            
            if ((specialFruitX == p1.headX && specialFruitY == p1.headY) ||
                (specialFruitX == fruitX && specialFruitY == fruitY))
                validPos = false;
            for (int i = 0; i < p1.nTail; i++)
                if (p1.tailX[i] == specialFruitX && p1.tailY[i] == specialFruitY)
                    validPos = false;
                    
            if (gameMode == MODE_MULTI)
            {
                if (specialFruitX == p2.headX && specialFruitY == p2.headY) validPos = false;
                for (int i = 0; i < p2.nTail; i++)
                    if (p2.tailX[i] == specialFruitX && p2.tailY[i] == specialFruitY)
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
    p1.lastDir = p1.dir;
    if (gameMode == MODE_MULTI) p2.lastDir = p2.dir;

    if (isSlowTimeActive)
    {
        slowTimeDuration--;
        if (slowTimeDuration <= 0)
        {
            isSlowTimeActive = false;
            gameSpeed = originalGameSpeed;
        }
    }
    
    int currentMaxScore = max(p1.score, p2.score);
    originalGameSpeed = max(120 - (currentMaxScore / 50) * 10, 10);
    if (!isSlowTimeActive) gameSpeed = originalGameSpeed;

    // --- PLAYER 1 TAIL UPDATE ---
    if (p1.isAlive)
    {
        if (p1.nTail > 0)
        {
            p1.lastTailX = p1.tailX[p1.nTail - 1];
            p1.lastTailY = p1.tailY[p1.nTail - 1];
        }
        else
        {
            p1.lastTailX = p1.headX;
            p1.lastTailY = p1.headY;
        }
        for (int i = p1.nTail - 1; i > 0; i--)
        {
            p1.tailX[i] = p1.tailX[i - 1];
            p1.tailY[i] = p1.tailY[i - 1];
        }
        if (p1.nTail > 0)
        {
            p1.tailX[0] = p1.headX;
            p1.tailY[0] = p1.headY;
        }

        switch (p1.dir)
        {
        case LEFT: p1.headX--; break;
        case RIGHT: p1.headX++; break;
        case UP: p1.headY--; break;
        case DOWN: p1.headY++; break;
        case STOP: p1.lastTailX = -1; p1.lastTailY = -1; break;
        }
    }

    // --- PLAYER 2 TAIL UPDATE ---
    if (gameMode == MODE_MULTI && p2.isAlive)
    {
        if (p2.nTail > 0)
        {
            p2.lastTailX = p2.tailX[p2.nTail - 1];
            p2.lastTailY = p2.tailY[p2.nTail - 1];
        }
        else
        {
            p2.lastTailX = p2.headX;
            p2.lastTailY = p2.headY;
        }
        for (int i = p2.nTail - 1; i > 0; i--)
        {
            p2.tailX[i] = p2.tailX[i - 1];
            p2.tailY[i] = p2.tailY[i - 1];
        }
        if (p2.nTail > 0)
        {
            p2.tailX[0] = p2.headX;
            p2.tailY[0] = p2.headY;
        }

        switch (p2.dir)
        {
        case LEFT: p2.headX--; break;
        case RIGHT: p2.headX++; break;
        case UP: p2.headY--; break;
        case DOWN: p2.headY++; break;
        case STOP: p2.lastTailX = -1; p2.lastTailY = -1; break;
        }
    }

    // --- PLAYER 1 BOUNDS & COLLISIONS ---
    if (p1.isAlive)
    {
        if (borderWrap)
        {
            if (p1.headX < 0) p1.headX = width - 1;
            else if (p1.headX >= width) p1.headX = 0;
            if (p1.headY < 0) p1.headY = height - 1;
            else if (p1.headY >= height) p1.headY = 0;
        }
        else
        {
            if (p1.headX < 0 || p1.headX >= width || p1.headY < 0 || p1.headY >= height)
                p1.isAlive = false;
        }
        for (int i = 0; i < p1.nTail; i++)
        {
            if (p1.tailX[i] == p1.headX && p1.tailY[i] == p1.headY)
                p1.isAlive = false;
        }
    }

    // --- PLAYER 2 BOUNDS & COLLISIONS ---
    if (gameMode == MODE_MULTI && p2.isAlive)
    {
        if (borderWrap)
        {
            if (p2.headX < 0) p2.headX = width - 1;
            else if (p2.headX >= width) p2.headX = 0;
            if (p2.headY < 0) p2.headY = height - 1;
            else if (p2.headY >= height) p2.headY = 0;
        }
        else
        {
            if (p2.headX < 0 || p2.headX >= width || p2.headY < 0 || p2.headY >= height)
                p2.isAlive = false;
        }
        for (int i = 0; i < p2.nTail; i++)
        {
            if (p2.tailX[i] == p2.headX && p2.tailY[i] == p2.headY)
                p2.isAlive = false;
        }
    }

    // --- INTER-PLAYER COLLISIONS ---
    if (gameMode == MODE_MULTI && p1.isAlive && p2.isAlive)
    {
        // Head-on collision
        if (p1.headX == p2.headX && p1.headY == p2.headY)
        {
            p1.isAlive = false;
            p2.isAlive = false;
        }
        // P1 head hits P2 body
        for (int i = 0; i < p2.nTail; i++)
        {
            if (p2.tailX[i] == p1.headX && p2.tailY[i] == p1.headY)
                p1.isAlive = false;
        }
        // P2 head hits P1 body
        for (int i = 0; i < p1.nTail; i++)
        {
            if (p1.tailX[i] == p2.headX && p1.tailY[i] == p2.headY)
                p2.isAlive = false;
        }
    }

    // Check game over
    if (gameMode == MODE_SINGLE)
    {
        if (!p1.isAlive) gameOver = true;
    }
    else // MODE_MULTI
    {
        if (!p1.isAlive || !p2.isAlive) gameOver = true;
    }

    if (gameOver) return;

    // --- FRUIT EATING LOGIC ---
    bool fruitEaten = false;
    if (p1.isAlive && p1.headX == fruitX && p1.headY == fruitY)
    {
        p1.score += 10;
        p1.nTail++;
        fruitEaten = true;
    }
    else if (gameMode == MODE_MULTI && p2.isAlive && p2.headX == fruitX && p2.headY == fruitY)
    {
        p2.score += 10;
        p2.nTail++;
        fruitEaten = true;
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
            if (isSpecialFruitOnScreen && fruitX == specialFruitX && fruitY == specialFruitY)
                validPos = false;
            if (fruitX == p1.headX && fruitY == p1.headY) validPos = false;
            for (int i = 0; i < p1.nTail; i++)
                if (p1.tailX[i] == fruitX && p1.tailY[i] == fruitY) validPos = false;

            if (gameMode == MODE_MULTI)
            {
                if (fruitX == p2.headX && fruitY == p2.headY) validPos = false;
                for (int i = 0; i < p2.nTail; i++)
                    if (p2.tailX[i] == fruitX && p2.tailY[i] == fruitY) validPos = false;
            }
        } while (!validPos);
    }

    // Special fruit check
    if (isSpecialFruitOnScreen)
    {
        bool specialEaten = false;
        if (p1.isAlive && p1.headX == specialFruitX && p1.headY == specialFruitY) specialEaten = true;
        if (gameMode == MODE_MULTI && p2.isAlive && p2.headX == specialFruitX && p2.headY == specialFruitY) specialEaten = true;

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
        case 0: // 1 Player Mode
            gameMode = MODE_SINGLE;
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
            cout << "Final Score: " << p1.score;
            if (p1.score > highScore && p1.score > 0)
            {
                highScore = p1.score;
                highScorePlayerName = GetPlayerName("Player", "NEW HIGH SCORE!");
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

        case 1: // 2 Player Mode
            gameMode = MODE_MULTI;
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
            if (p1.isAlive && !p2.isAlive)
            {
                SetColor(GREEN);
                cout << "  PLAYER 2 LOST!   ";
            }
            else if (p2.isAlive && !p1.isAlive)
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
            cout << "Player 1 Score: " << p1.score;
            SetColor(CYAN);
            GotoXY(width / 2 - 10, height / 2 + 1);
            cout << "Player 2 Score: " << p2.score;

            int maxScore;
            maxScore = max(p1.score, p2.score);
            if (maxScore > highScore && maxScore > 0)
            {
                highScore = maxScore;
                string winnerTitle = (p1.score > p2.score) ? "P1 HIGH SCORE!" : "P2 HIGH SCORE!";
                if (p1.score == p2.score) winnerTitle = "NEW HIGH SCORE!";
                highScorePlayerName = GetPlayerName("Player", winnerTitle);
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

        case 2: ShowHelpPage(); break;
        case 3: ShowSettingsPage(); break;
        case 4: ShowHighScorePage(); break;
        case 5: StopAllMusic(); return 0;
        }
    }
    return 0;
}
