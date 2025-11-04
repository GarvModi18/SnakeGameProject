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
int headX, headY, fruitX, fruitY, score;

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

int tailX[1000], tailY[1000];
int nTail;
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
eDirection dir;
eDirection lastDir;
int lastTailX, lastTailY;

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
    cout << "Score: " << score;
    
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
    const int numOptions = 5;
    string menuOptions[numOptions] = { "Start Game", "Help", "Settings", "High Score", "Quit Game" };
    
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
    dir = RIGHT;
    lastDir = RIGHT;
    headX = width / 2; 
    headY = height / 2;
    
    nTail = 2;
    tailX[0] = headX - 1;
    tailY[0] = headY;
    tailX[1] = headX - 2;
    tailY[1] = headY;
    
    fruitX = rand() % width; 
    fruitY = rand() % height;
    
    bool validPos = false;
    while (!validPos)
    {
        validPos = true;
        if ((fruitX == headX && fruitY == headY))
            validPos = false;
        for (int i = 0; i < nTail; i++)
        {
            if (tailX[i] == fruitX && tailY[i] == fruitY)
            {
                validPos = false;
                break;
            }
        }
        if (!validPos)
        {
            fruitX = rand() % width;
            fruitY = rand() % height;
        }
    }
    
    score = 0; 
    speedLevel = 1; 
    gameSpeed = originalGameSpeed; 
    frameCount = 0;
    lastTailX = -1; 
    lastTailY = -1;
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

    if (lastTailX != -1)
    {
        GotoXY(lastTailX + 1, lastTailY + 1); 
        cout << " "; 
        lastTailX = -1;
    }
    GotoXY(headX + 1, headY + 1);
    char headChar;
    if (dir == UP) headChar = '^';
    else if (dir == DOWN) headChar = 'v';
    else if (dir == LEFT) headChar = '<';
    else if (dir == RIGHT) headChar = '>';
    else headChar = '@';
    if (frameCount % 2 == 0) SetColor(GREEN, BLACK);
    else if (isSlowTimeActive) SetColor(CYAN, BLACK);
    else SetColor(YELLOW, BLACK);
    cout << headChar; 
    frameCount++;
    if (nTail > 0)
    {
        for (int i = 0; i < nTail; i++)
        {
            GotoXY(tailX[i] + 1, tailY[i] + 1);
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
    SetColor(YELLOW);
    cout << "Score: " << score << " | High Score: " << highScore;
    cout << " | Speed Level: " << speedLevel;
    if (isSlowTimeActive)
    { 
        SetColor(CYAN); 
        cout << " | STATUS: SLOW TIME (" << slowTimeDuration / (1000 / originalGameSpeed) << "s)"; 
    }
    SetColor(GREY); 
    cout << " | Music: " << (musicEnabled ? "ON" : "OFF") << "  "; 
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
            case 72: if (lastDir != DOWN) dir = UP; break;
            case 80: if (lastDir != UP) dir = DOWN; break;
            case 75: if (lastDir != RIGHT) dir = LEFT; break;
            case 77: if (lastDir != LEFT) dir = RIGHT; break;
            }
        }
        else
        {
            char lowerKey = tolower(key);
            switch (lowerKey)
            {
            case 'w': if (lastDir != DOWN) dir = UP; break;
            case 's': if (lastDir != UP) dir = DOWN; break;
            case 'a': if (lastDir != RIGHT) dir = LEFT; break;
            case 'd': if (lastDir != LEFT) dir = RIGHT; break;
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
            if ((specialFruitX == headX && specialFruitY == headY) ||
                (specialFruitX == fruitX && specialFruitY == fruitY))
                validPos = false;
            for (int i = 0; i < nTail; i++)
                if (tailX[i] == specialFruitX && tailY[i] == specialFruitY)
                    validPos = false;
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
    lastDir = dir;
    if (isSlowTimeActive)
    {
        slowTimeDuration--;
        if (slowTimeDuration <= 0)
        {
            isSlowTimeActive = false;
            gameSpeed = originalGameSpeed;
        }
    }
    originalGameSpeed = max(120 - (score / 50) * 10, 10);
    if (!isSlowTimeActive) gameSpeed = originalGameSpeed;
    if (nTail > 0) 
    { 
        lastTailX = tailX[nTail - 1]; 
        lastTailY = tailY[nTail - 1]; 
    }
    else 
    { 
        lastTailX = headX; 
        lastTailY = headY; 
    }
    for (int i = nTail - 1; i > 0; i--) 
    { 
        tailX[i] = tailX[i - 1]; 
        tailY[i] = tailY[i - 1]; 
    }
    if (nTail > 0) 
    { 
        tailX[0] = headX; 
        tailY[0] = headY; 
    }
    
    // Store previous position for diagonal movement normalization
    int prevHeadX = headX;
    int prevHeadY = headY;
    
    switch (dir)
    {
        case LEFT: headX--; break;
        case RIGHT: headX++; break;
        case UP: headY--; break;
        case DOWN: headY++; break;
        case STOP: lastTailX = -1; lastTailY = -1; break;
    }
    
    bool collided = false;
    if (borderWrap)
    {
        if (headX < 0) headX = width - 1;
        else if (headX >= width) headX = 0;
        if (headY < 0) headY = height - 1;
        else if (headY >= height) headY = 0;
    }
    else
    {
        if (headX < 0 || headX >= width || headY < 0 || headY >= height) collided = true;
    }
    for (int i = 0; i < nTail; i++)
    {
        if (tailX[i] == headX && tailY[i] == headY) collided = true;
    }
    if (collided)
    {
        gameOver = true;
        if (score > highScore)
        {
            highScorePlayerName = GetPlayerName();
            highScore = score;
            SaveHighScore();
        }
    }
    if (headX == fruitX && headY == fruitY)
    {
        score += 10; 
        nTail++; 
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
            if (fruitX == headX && fruitY == headY) validPos = false;
            for (int i = 0; i < nTail; i++)
            {
                if (tailX[i] == fruitX && tailY[i] == fruitY)
                {
                    validPos = false;
                    break;
                }
            }
        } while (!validPos);
    }
    if (isSpecialFruitOnScreen && headX == specialFruitX && headY == specialFruitY)
    {
        isSpecialFruitOnScreen = false;
        ApplySlowTimeEffect();
        GotoXY(specialFruitX + 1, specialFruitY + 1);
        cout << " ";
        specialFruitX = -1;
        specialFruitY = -1;
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
        case 0: // Start Game
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
            cout << "Final Score: " << score;
            if (score == highScore && score > 0)
            {
                SetColor(GREEN);
                GotoXY(width / 2 - 12, height / 2 + 1);
                cout << "NEW HIGH SCORE: " << highScore;
                GotoXY(width / 2 - 12, height / 2 + 2);
                cout << "Player: " << highScorePlayerName;
            }
            else
            {
                SetColor(CYAN);
                GotoXY(width / 2 - 12, height / 2 + 1);
                cout << "Current High Score: " << highScore;
            }
            SetColor(GREY);
            GotoXY(width / 2 - 12, height / 2 + 3);
            cout << "Press any key to continue...";
            ResetColor();
            getch();
            break;
        case 1: ShowHelpPage(); break;
        case 2: ShowSettingsPage(); break;
        case 3: ShowHighScorePage(); break;
        case 4: StopAllMusic(); return 0;
        }
    }
    return 0;
}
