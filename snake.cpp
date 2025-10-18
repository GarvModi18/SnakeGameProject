#include <iostream>
#include <vector>
#include <windows.h>    // For Sleep(), SetConsoleCursorPosition, SetConsoleTextAttribute
#include <ctime>        // For time() to seed rand()
#include <cstdio>       // For standard I/O
#include <conio.h>      // For kbhit() and getch()
#include <fstream>      // NEW: For file operations (reading/writing high score)
#include <string>       // For string manipulation (optional but good practice)

using namespace std;

// --- Console Color Definitions ---
enum ConsoleColor {
    BLACK = 0, DARKBLUE = 1, DARKGREEN = 2, DARKCYAN = 3, DARKRED = 4, DARKMAGENTA = 5,
    DARKYELLOW = 6, GREY = 7, DARKGREY = 8, BLUE = 9, GREEN = 10, CYAN = 11,
    RED = 12, MAGENTA = 13, YELLOW = 14, WHITE = 15
};

// --- Console Utility Functions ---
void SetColor(ConsoleColor textColor, ConsoleColor bgColor = BLACK) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (WORD)((bgColor << 4) | textColor));
}
void ResetColor() { SetColor(WHITE, BLACK); }
void GotoXY(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// --- Global Game Variables ---
bool gameOver;
const int width = 40; 
const int height = 20;
int headX, headY, fruitX, fruitY, score;
int tailX[1000], tailY[1000];
int nTail;
int gameSpeed = 120;
int frameCount = 0; 
int highScore = 0; // NEW: Global High Score variable

enum eDirection { STOP = 0, LEFT, RIGHT, UP, DOWN };
eDirection dir;
eDirection lastDir; 
int lastTailX, lastTailY;

// --- FILE I/O FUNCTIONS ---

void LoadHighScore() {
    ifstream fileIn("highscore.txt");
    if (fileIn.is_open()) {
        fileIn >> highScore;
        fileIn.close();
    } else {
        // If file doesn't exist, high score remains 0 (initialized globally)
        highScore = 0;
    }
}

void SaveHighScore() {
    ofstream fileOut("highscore.txt");
    if (fileOut.is_open()) {
        fileOut << highScore; // Save the updated high score
        fileOut.close();
    }
}


// --- Game Logic and Drawing ---

void Setup() {
    SetConsoleOutputCP(65001); 
    LoadHighScore(); // NEW: Load high score before starting the game
    
    srand(static_cast<unsigned int>(time(0))); 
    gameOver = false;
    dir = RIGHT; 
    lastDir = RIGHT; 
    headX = width / 2;
    headY = height / 2;
    fruitX = rand() % width;
    fruitY = rand() % height;
    score = 0;
    nTail = 0;
    gameSpeed = 120; 
    frameCount = 0; 
    lastTailX = -1; 
    lastTailY = -1;

    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);

    system("cls"); 
    SetColor(DARKGREY);
    for (int i = 0; i < width + 2; i++) {
        GotoXY(i, 0); cout << "█"; 
        GotoXY(i, height + 1); cout << "█";
    }
    for (int i = 0; i < height + 2; i++) {
        GotoXY(0, i); cout << "█";
        GotoXY(width + 1, i); cout << "█";
    }
    ResetColor();
}

void Draw() {
    // 1. Erase the very last position of the snake
    if (lastTailX != -1) {
        GotoXY(lastTailX + 1, lastTailY + 1); 
        cout << " ";
        lastTailX = -1; 
    }
    
    // 2. Draw Head with directional animation and color change
    GotoXY(headX + 1, headY + 1); 
    
    char headChar;
    if (dir == UP) headChar = '^'; 
    else if (dir == DOWN) headChar = 'v'; 
    else if (dir == LEFT) headChar = '<'; 
    else if (dir == RIGHT) headChar = '>'; 
    else headChar = '@'; 

    if (frameCount % 2 == 0) { 
        SetColor(GREEN, BLACK); 
    } else {
        SetColor(YELLOW, BLACK); 
    }
    cout << headChar;
    frameCount++; 

    // 3. Draw new segment of Tail
    if (nTail > 0) {
        GotoXY(tailX[0] + 1, tailY[0] + 1);
        SetColor(DARKGREEN);
        cout << "o"; 
    }
    ResetColor();

    // 4. Draw real food (Heart symbol)
    GotoXY(fruitX + 1, fruitY + 1); 
    SetColor(RED);
    cout << "♥"; 
    ResetColor();

    // 5. Update Score and High Score
    GotoXY(0, height + 2); 
    SetColor(YELLOW);
    cout << "Score: " << score << " | High Score: " << highScore << " | Speed: " << (150 - gameSpeed) / 10 + 1 << "  "; 
    ResetColor();
}

void Input() {
    if (kbhit()) { 
        char key = getch();
        
        if (key == 0 || key == -32) { 
            key = getch(); 
            switch (key) {
            case 72: if (lastDir != DOWN) dir = UP; break; 
            case 80: if (lastDir != UP) dir = DOWN; break; 
            case 75: if (lastDir != RIGHT) dir = LEFT; break; 
            case 77: if (lastDir != LEFT) dir = RIGHT; break; 
            }
        } 
        else {
            switch (key) {
            case 'x': case 'X': 
                gameOver = true; 
                break;
            case 'p': case 'P': 
                SetColor(CYAN);
                GotoXY(width / 2 - 2, height / 2 + 1);
                cout << "PAUSED";
                getch(); 
                GotoXY(width / 2 - 2, height / 2 + 1);
                cout << "      "; 
                ResetColor();
                break;
            }
        }
    }
}

void Logic() {
    lastDir = dir; 

    if (nTail > 0) {
        lastTailX = tailX[nTail - 1];
        lastTailY = tailY[nTail - 1];
    } else {
        lastTailX = headX; 
        lastTailY = headY;
    }

    // Shift the tail segments forward 
    for (int i = nTail - 1; i > 0; i--) {
        tailX[i] = tailX[i - 1];
        tailY[i] = tailY[i - 1];
    }
    if (nTail > 0) {
        tailX[0] = headX;
        tailY[0] = headY;
    }

    // Move the snake head
    switch (dir) {
    case LEFT: headX--; break;
    case RIGHT: headX++; break;
    case UP: headY--; break;
    case DOWN: headY++; break;
    case STOP: 
        lastTailX = -1; 
        lastTailY = -1; 
        break;
    }
    
    // Check for game over conditions
    bool collided = false;
    if (headX < 0 || headX >= width || headY < 0 || headY >= height)
        collided = true;

    for (int i = 0; i < nTail; i++) {
        if (tailX[i] == headX && tailY[i] == headY)
            collided = true;
    }

    if (collided) {
        gameOver = true;
        // NEW: Check and save high score upon death
        if (score > highScore) {
            highScore = score;
            SaveHighScore();
        }
    }

    // Fruit Check
    if (headX == fruitX && headY == fruitY) {
        score += 10;
        nTail++; 
        if (score % 50 == 0 && gameSpeed > 10) { 
            gameSpeed -= 10; 
        }
        
        bool validFruitPos;
        do {
            validFruitPos = true;
            fruitX = rand() % width;
            fruitY = rand() % height;
            if (fruitX == headX && fruitY == headY) validFruitPos = false;
            for (int i = 0; i < nTail; i++) {
                if (tailX[i] == fruitX && tailY[i] == fruitY) {
                    validFruitPos = false;
                    break;
                }
            }
        } while (!validFruitPos);
    }
}

// --- Main Program ---
int main() {
    Setup();
    while (!gameOver) {
        Input(); 
        Logic(); 
        Draw();  
        Sleep(gameSpeed); 
    }

    // Game Over screen
    system("cls"); 
    SetColor(RED);
    GotoXY(width / 2 - 5, height / 2 - 2);
    cout << "GAME OVER!";
    
    SetColor(YELLOW);
    GotoXY(width / 2 - 8, height / 2);
    cout << "Final Score: " << score;
    
    // NEW: Displaying the FINAL High Score
    if (score == highScore && score > 0) {
        SetColor(GREEN);
        GotoXY(width / 2 - 12, height / 2 + 1);
        cout << "NEW HIGH SCORE: " << highScore;
    } else {
        SetColor(CYAN);
        GotoXY(width / 2 - 12, height / 2 + 1);
        cout << "Current High Score: " << highScore;
    }
    
    SetColor(GREY);
    GotoXY(width / 2 - 12, height / 2 + 3);
    cout << "Press any key to exit...";
    ResetColor();

    getch(); 

    return 0;
}