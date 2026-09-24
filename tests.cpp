#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#define main snake_main
#include "snake.cpp"
#undef main

// Helper to reset all game globals to a clean known state before each test
void ResetGameState()
{
    gameOver = false;
    dir = RIGHT;
    lastDir = RIGHT;
    headX = 10;
    headY = 10;
    nTail = 0;
    score = 0;
    highScore = 50; // default higher than score to prevent triggering high score logic unexpectedly
    highScorePlayerName = "Default";
    fruitX = 30;
    fruitY = 15;
    isSpecialFruitOnScreen = false;
    specialFruitX = -1;
    specialFruitY = -1;
    isSlowTimeActive = false;
    slowTimeDuration = 0;
    fruitsEaten = 0;
    speedLevel = 1;
    borderWrap = false;
    gameSpeed = 130;
    originalGameSpeed = 110;
    frameCount = 0;
    lastTailX = -1;
    lastTailY = -1;
    inputCharProvider = _getch;
    highScoreFilename = "highscore.txt";
}

// Test Stub: Sequence of scripted keystrokes for GetPlayerName
static std::vector<char> stubbedKeySequence;
static size_t stubbedKeyIndex = 0;

int TestStub_KeyReader()
{
    if (stubbedKeyIndex < stubbedKeySequence.size())
    {
        return stubbedKeySequence[stubbedKeyIndex++];
    }
    return 13; // default Enter key
}

void SetStubbedKeys(const std::string& input)
{
    stubbedKeySequence.clear();
    for (size_t i = 0; i < input.length(); ++i)
    {
        stubbedKeySequence.push_back(input[i]);
    }
    stubbedKeySequence.push_back(13); // Enter key to submit
    stubbedKeyIndex = 0;
}

// ==========================================
// TEST CASES FOR THE 5 BEHAVIORAL RULES
// ==========================================

// Rule 1: Eating a regular fruit increases score by 10 and increases tail length by 1
void test_rule1_eating_fruit_increases_score_and_tail()
{
    ResetGameState();
    headX = 5;
    headY = 5;
    fruitX = 6;
    fruitY = 5;
    dir = RIGHT;
    nTail = 2;
    tailX[0] = 4; tailY[0] = 5;
    tailX[1] = 3; tailY[1] = 5;
    int initialScore = score;
    int initialTail = nTail;

    Logic();

    assert(headX == 6 && headY == 5);
    assert(score == initialScore + 10);
    assert(nTail == initialTail + 1);
    std::cout << "[PASS] Test 1: Rule 1 - Eating fruit increases score by 10 and tail length by 1\n";
}

// Rule 2: Snake dies (gameOver = true) when head enters boundary wall (with borderWrap = false)
void test_rule2_wall_collision_causes_game_over()
{
    ResetGameState();
    borderWrap = false;
    headX = width - 1; // 39
    headY = 10;
    dir = RIGHT;

    Logic();

    assert(headX == width);
    assert(gameOver == true);
    std::cout << "[PASS] Test 2: Rule 2 - Wall collision causes game over\n";
}

// Rule 3: Snake dies (gameOver = true) when head enters its own body
void test_rule3_self_collision_causes_game_over()
{
    ResetGameState();
    nTail = 4;
    headX = 10;
    headY = 10;
    tailX[0] = 10; tailY[0] = 9;  // Segment above
    tailX[1] = 9;  tailY[1] = 9;
    tailX[2] = 9;  tailY[2] = 10;
    tailX[3] = 10; tailY[3] = 10;
    dir = UP; // Head moves to (10, 9), colliding with tail[0]

    Logic();

    assert(gameOver == true);
    std::cout << "[PASS] Test 3: Rule 3 - Self collision causes game over\n";
}

// Rule 4: Border wrap teleports snake head to opposite boundary when crossing edge
void test_rule4_border_wrap_teleports_head()
{
    ResetGameState();
    borderWrap = true;
    headX = width - 1; // 39
    headY = 10;
    dir = RIGHT;

    Logic();

    assert(headX == 0);
    assert(headY == 10);
    assert(gameOver == false);
    std::cout << "[PASS] Test 4: Rule 4 - Border wrap teleports head\n";
}

// Rule 5: When new high score is achieved, prompts player for name and persists score
void test_rule5_high_score_prompt_and_persistence_via_seam()
{
    ResetGameState();
    const std::string testFile = "test_highscore_tmp.txt";
    highScoreFilename = testFile;
    highScore = 20;
    score = 40; // New high score
    headX = width - 1;
    headY = 5;
    dir = RIGHT;
    borderWrap = false;

    // Use seam to provide scripted player name "Champion"
    SetStubbedKeys("Champion");
    inputCharProvider = TestStub_KeyReader;

    Logic(); // Hits wall -> gameOver -> score (40) > highScore (20) -> prompts name & saves

    assert(gameOver == true);
    assert(highScore == 40);
    assert(highScorePlayerName == "Champion");

    // Verify the persisted file on disk via seam
    std::ifstream in(testFile.c_str());
    assert(in.is_open());
    int savedScore = 0;
    std::string savedName;
    in >> savedScore;
    in.ignore();
    std::getline(in, savedName);
    in.close();

    assert(savedScore == 40);
    assert(savedName == "Champion");
    remove(testFile.c_str()); // Clean up test artifact

    std::cout << "[PASS] Test 5: Rule 5 - High score name prompt and persistence via seam\n";
}

// ==========================================
// ADDITIONAL UNIT TESTS FOR GAME COVERAGE
// ==========================================

void test_direct_get_player_name_with_backspace()
{
    ResetGameState();
    // Simulate typing 'A', 'b', backspace (8), 'l', 'e', 'x', Enter
    stubbedKeySequence.clear();
    stubbedKeySequence.push_back('A');
    stubbedKeySequence.push_back('b');
    stubbedKeySequence.push_back(8); // Backspace
    stubbedKeySequence.push_back('l');
    stubbedKeySequence.push_back('e');
    stubbedKeySequence.push_back('x');
    stubbedKeySequence.push_back(13); // Enter
    stubbedKeyIndex = 0;

    std::string result = GetPlayerName(TestStub_KeyReader);
    assert(result == "Alex");
    std::cout << "[PASS] Test 6: GetPlayerName with backspace handling\n";
}

void test_load_and_save_high_score_seam()
{
    ResetGameState();
    const std::string testFile = "test_save_load.txt";
    highScore = 150;
    highScorePlayerName = "Speedy";
    SaveHighScore(testFile);

    // Reset values in memory
    highScore = 0;
    highScorePlayerName = "None";

    LoadHighScore(testFile);
    assert(highScore == 150);
    assert(highScorePlayerName == "Speedy");
    remove(testFile.c_str());
    std::cout << "[PASS] Test 7: SaveHighScore and LoadHighScore file seam\n";
}

void test_slow_time_effect()
{
    ResetGameState();
    ApplySlowTimeEffect();
    assert(isSlowTimeActive == true);
    assert(slowTimeDuration == SLOW_TIME_FRAMES);

    // Run logic steps to observe countdown
    headX = 10; headY = 10; dir = RIGHT;
    Logic();
    assert(slowTimeDuration == SLOW_TIME_FRAMES - 1);
    std::cout << "[PASS] Test 8: ApplySlowTimeEffect and duration countdown\n";
}

void test_border_wrap_top_and_left()
{
    ResetGameState();
    borderWrap = true;
    // Test moving left past border (headX = 0, dir = LEFT -> headX = width - 1)
    headX = 0; headY = 10; dir = LEFT;
    Logic();
    assert(headX == width - 1);

    // Test moving up past border (headY = 0, dir = UP -> headY = height - 1)
    headX = 10; headY = 0; dir = UP;
    Logic();
    assert(headY == height - 1);
    std::cout << "[PASS] Test 9: Border wrap for left and top edges\n";
}

void test_speed_level_scaling_after_five_fruits()
{
    ResetGameState();
    fruitsEaten = 4;
    headX = 5; headY = 5;
    fruitX = 6; fruitY = 5;
    dir = RIGHT;
    int prevSpeedLevel = speedLevel;

    Logic(); // Eats 5th fruit: fruitsEaten becomes 5, speedLevel increments
    assert(fruitsEaten == 5);
    assert(originalGameSpeed == 110);
    assert(speedLevel == prevSpeedLevel + 1);
    std::cout << "[PASS] Test 10: Speed level increases after 5 fruits eaten\n";
}

int main()
{
    std::cout << "========================================\n";
    std::cout << "Running Snake Game Full Test Suite\n";
    std::cout << "========================================\n";

    test_rule1_eating_fruit_increases_score_and_tail();
    test_rule2_wall_collision_causes_game_over();
    test_rule3_self_collision_causes_game_over();
    test_rule4_border_wrap_teleports_head();
    test_rule5_high_score_prompt_and_persistence_via_seam();
    test_direct_get_player_name_with_backspace();
    test_load_and_save_high_score_seam();
    test_slow_time_effect();
    test_border_wrap_top_and_left();
    test_speed_level_scaling_after_five_fruits();

    std::cout << "========================================\n";
    std::cout << "All 10 tests passed successfully!\n";
    std::cout << "========================================\n";
    return 0;
}
