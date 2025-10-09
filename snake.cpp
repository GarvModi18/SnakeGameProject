#include <iostream>
#include <conio.h>
#include <windows.h>
#include <vector>
#include <cstdlib>
#include <ctime>
using namespace std;

const int width = 40;
const int height = 20;

struct Point
{
    int x, y;
};

enum Direction
{
    STOP = 0,
    LEFT,
    RIGHT,
    UP,
    DOWN
};
Direction dir;

vector<Point> snake; // Snake body
Point fruit;
int score = 0;
bool gameOver = false;

// Function to check if a point is on the snake
bool isOnSnake(int x, int y)
{
    for (auto s : snake)
    {
        if (s.x == x && s.y == y)
            return true;
    }
    return false;
}

// Improved fruit spawning algorithm
void spawnFruit()
{
    do
    {
        fruit.x = rand() % width;
        fruit.y = rand() % height;
    } while (isOnSnake(fruit.x, fruit.y));
}

// Draw the board
void draw()
{
    system("cls");
    for (int i = 0; i < width + 2; i++)
        cout << "#";
    cout << endl;

    for (int y = 0; y < height; y++)
    {
        cout << "#";
        for (int x = 0; x < width; x++)
        {
            if (x == snake[0].x && y == snake[0].y)
                cout << "O"; // Head
            else if (x == fruit.x && y == fruit.y)
                cout << "F"; // Fruit
            else
            {
                bool printed = false;
                for (size_t k = 1; k < snake.size(); k++)
                {
                    if (snake[k].x == x && snake[k].y == y)
                    {
                        cout << "o"; // Body
                        printed = true;
                        break;
                    }
                }
                if (!printed)
                    cout << " ";
            }
        }
        cout << "#" << endl;
    }

    for (int i = 0; i < width + 2; i++)
        cout << "#";
    cout << "\nScore: " << score << endl;
}

// Input handling
void input()
{
    if (_kbhit())
    {
        switch (_getch())
        {
        case 'a':
            dir = LEFT;
            break;
        case 'd':
            dir = RIGHT;
            break;
        case 'w':
            dir = UP;
            break;
        case 's':
            dir = DOWN;
            break;
        case 'x':
            gameOver = true;
            break;
        }
    }
}

// Logic update
void logic()
{
    Point prev = snake[0];
    Point prev2;

    // Move head
    switch (dir)
    {
    case LEFT:
        snake[0].x--;
        break;
    case RIGHT:
        snake[0].x++;
        break;
    case UP:
        snake[0].y--;
        break;
    case DOWN:
        snake[0].y++;
        break;
    default:
        break;
    }

    // Move body
    for (size_t i = 1; i < snake.size(); i++)
    {
        prev2 = snake[i];
        snake[i] = prev;
        prev = prev2;
    }

    // Collision with walls
    if (snake[0].x < 0 || snake[0].x >= width || snake[0].y < 0 || snake[0].y >= height)
        gameOver = true;

    // Collision with self
    for (size_t i = 1; i < snake.size(); i++)
    {
        if (snake[i].x == snake[0].x && snake[i].y == snake[0].y)
            gameOver = true;
    }

    // Eating fruit
    if (snake[0].x == fruit.x && snake[0].y == fruit.y)
    {
        score += 10;
        snake.push_back({-1, -1}); // Extend snake
        spawnFruit();
    }
}

int main()
{
    srand(time(0));
    dir = STOP;
    snake.push_back({width / 2, height / 2});
    spawnFruit();

    while (!gameOver)
    {
        draw();
        input();
        logic();
        Sleep(100); // Control speed
    }

    cout << "Game Over! Final Score: " << score << endl;
    return 0;
}
