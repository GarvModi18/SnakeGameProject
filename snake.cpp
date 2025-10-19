#include <iostream>
#include <vector>
#include <windows.h>    
#include <ctime>        
#include <cstdio>       
#include <conio.h>      
#include <fstream>      
#include <string>       

using namespace std;

// --- Console Color Definitions ---
enum ConsoleColor {
    BLACK = 0, DARKBLUE = 1, DARKGREEN = 2, DARKCYAN = 3, DARKRED = 4, DARKMAGENTA = 5,
    DARKYELLOW = 6, GREY = 7, DARKGREY = 8, BLUE = 9, GREEN = 10, CYAN = 11,
    RED = 12, MAGENTA = 13, YELLOW = 14, WHITE = 15
};

// --- Console Utility Functions (Unchanged) ---
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
// NEW SPECIAL FRUIT VARIABLES
int specialFruitX, specialFruitY; // Coordinates for the Special Fruit (Blue Diamond)
bool isSpecialFruitOnScreen = false;
const int SPECIAL_FRUIT_SPAWN_CHANCE = 10; // 1 in 10 chance on fruit consumption

// NEW EFFECT VARIABLES
bool isSlowTimeActive = false;
int slowTimeDuration = 0; // Frames remaining for slow time
int originalGameSpeed = 120; // Store the base speed
const int SLOW_TIME_FRAMES = 150; // Effect lasts for 150 frames (approx 18 seconds)

int tailX[1000], tailY[1000];
int nTail;
int gameSpeed = 120;
int frameCount = 0; 
int highScore = 0; 

enum eDirection { STOP = 0, LEFT, RIGHT, UP, DOWN };
eDirection dir;
eDirection lastDir; 
int lastTailX, lastTailY;

// --- FILE I/O FUNCTIONS (Unchanged) ---
void LoadHighScore() {
    ifstream fileIn("highscore.txt");
    if (fileIn.is_open()) { fileIn >> highScore; fileIn.close(); } else { highScore = 0; }
}

void SaveHighScore() {
    ofstream fileOut("highscore.txt");
    if (fileOut.is_open()) { fileOut << highScore; fileOut.close(); }
}

// --- Game Logic and Drawing ---

void Setup() {
    SetConsoleOutputCP(65001); 
    LoadHighScore(); 
    
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
    gameSpeed = originalGameSpeed; // Start at base speed
    frameCount = 0; 
    lastTailX = -1; 
    lastTailY = -1;

    // Reset special effects
    isSpecialFruitOnScreen = false;
    isSlowTimeActive = false;
    slowTimeDuration = 0;

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
        // Change to CYAN when slow time is active for visual feedback
        if (isSlowTimeActive) SetColor(CYAN, BLACK); else SetColor(YELLOW, BLACK);
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

    // 4. Draw Standard Food (Heart symbol)
    GotoXY(fruitX + 1, fruitY + 1); 
    SetColor(RED);
    cout << "♥"; 
    ResetColor();

    // 5. Draw Special Food (Blue Diamond)
    if (isSpecialFruitOnScreen) {
        GotoXY(specialFruitX + 1, specialFruitY + 1);
        SetColor(BLUE);
        cout << "♦"; // Blue Diamond for special fruit
        ResetColor();
    }

    // 6. Update Score and High Score & Effect Status
    GotoXY(0, height + 2); 
    SetColor(YELLOW);
    cout << "Score: " << score << " | High Score: " << highScore;
    cout << " | Speed: " << (originalGameSpeed - gameSpeed) / 10 + 1;

    // Display Status of Active Effect
    if (isSlowTimeActive) {
        SetColor(CYAN);
        cout << " | STATUS: SLOW TIME (" << slowTimeDuration / (1000 / originalGameSpeed) << "s)";
    }
    cout << "  "; // Padding
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

void SpawnSpecialFruit() {
    if (isSpecialFruitOnScreen) return; // Only one special fruit at a time

    // 1 in SPECIAL_FRUIT_SPAWN_CHANCE chance to spawn
    if (rand() % SPECIAL_FRUIT_SPAWN_CHANCE == 0) { 
        isSpecialFruitOnScreen = true;
        bool validPos;
        do {
            validPos = true;
            specialFruitX = rand() % width;
            specialFruitY = rand() % height;
            // Check collision with head, body, and regular fruit
            if ((specialFruitX == headX && specialFruitY == headY) ||
                (specialFruitX == fruitX && specialFruitY == fruitY)) 
                validPos = false;
            for (int i = 0; i < nTail; i++) {
                if (tailX[i] == specialFruitX && tailY[i] == specialFruitY) {
                    validPos = false;
                    break;
                }
            }
        } while (!validPos);
    }
}

void ApplySlowTimeEffect() {
    isSlowTimeActive = true;
    slowTimeDuration = SLOW_TIME_FRAMES;
    gameSpeed = originalGameSpeed + 80; // Significantly slow down the game
}

void Logic() {
    lastDir = dir; 

    // --- EFFECT MANAGEMENT ---
    if (isSlowTimeActive) {
        slowTimeDuration--;
        if (slowTimeDuration <= 0) {
            isSlowTimeActive = false;
            gameSpeed = originalGameSpeed; // Reset to original speed
        }
    }
    // Update originalGameSpeed based on current score (for non-effect base speed)
    originalGameSpeed = max(120 - (score / 50) * 10, 10);
    if (!isSlowTimeActive) {
        gameSpeed = originalGameSpeed; // Apply normal speed if effect is off
    }
    
    // --- MOVEMENT & COLLISION (Unchanged) ---
    // ... (Tail shifting logic remains the same) ...
    if (nTail > 0) {
        lastTailX = tailX[nTail - 1];
        lastTailY = tailY[nTail - 1];
    } else {
        lastTailX = headX; 
        lastTailY = headY;
    }
    for (int i = nTail - 1; i > 0; i--) {
        tailX[i] = tailX[i - 1];
        tailY[i] = tailY[i - 1];
    }
    if (nTail > 0) {
        tailX[0] = headX;
        tailY[0] = headY;
    }

    switch (dir) {
    case LEFT: headX--; break; case RIGHT: headX++; break; 
    case UP: headY--; break; case DOWN: headY++; break;
    case STOP: lastTailX = -1; lastTailY = -1; break;
    }
    
    bool collided = false;
    if (headX < 0 || headX >= width || headY < 0 || headY >= height)
        collided = true;
    for (int i = 0; i < nTail; i++) {
        if (tailX[i] == headX && tailY[i] == headY) collided = true;
    }

    if (collided) {
        gameOver = true;
        if (score > highScore) { highScore = score; SaveHighScore(); }
    }

    // --- FRUIT CHECKS ---
    
    // 1. Regular Fruit Check
    if (headX == fruitX && headY == fruitY) {
        score += 10;
        nTail++; 
        
        // Try to spawn special fruit
        SpawnSpecialFruit();

        // Spawn new regular fruit
        bool validPos;
        do {
            validPos = true;
            fruitX = rand() % width;
            fruitY = rand() % height;
            // Also check collision with special fruit position
            if ((fruitX == specialFruitX && fruitY == specialFruitY) && isSpecialFruitOnScreen) validPos = false;
            if (fruitX == headX && fruitY == headY) validPos = false;
            for (int i = 0; i < nTail; i++) {
                if (tailX[i] == fruitX && tailY[i] == fruitY) { validPos = false; break; }
            }
        } while (!validPos);
    }

    // 2. Special Fruit Check
    if (isSpecialFruitOnScreen && headX == specialFruitX && headY == specialFruitY) {
        // Remove the fruit and apply effect
        isSpecialFruitOnScreen = false;
        ApplySlowTimeEffect();

        // Erase the fruit from the screen immediately (important for smooth removal)
        GotoXY(specialFruitX + 1, specialFruitY + 1);
        cout << " "; 
        
        // Reset coordinates to avoid accidental double-eating
        specialFruitX = -1;
        specialFruitY = -1;
    }
}

// --- Main Program (Unchanged) ---
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