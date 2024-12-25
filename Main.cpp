#include "raylib.h"
#include "raymath.h"
#include <chrono>

// Global or player-specific timer
std::chrono::steady_clock::time_point lastCollisionTime = std::chrono::steady_clock::now();

struct Player
{
    Vector2 position;
    Vector2 speed;
    float radius;
    Color color;
    int hp;
    int lvl;
    int exp;
};

struct Enemy
{
    Vector2 position;
    float radius;
    Color color;
    Vector2 speed;
    bool isActive;
};

struct Bullet
{
    Vector2 position;
    Vector2 speed;
    int radius;
    Color color;
    bool active;
};

// Window dimensions
const int screenWidth{1800};
const int screenHeight{900};

// Main functions
void UpdatePlayer(Player &player, Bullet bullets[], int maxBullets, float dashDistance);
void UpdateBullets(Bullet bullets[], int maxBullets);
void DrawBullets(const Bullet bullets[], int maxBullets, int bulletRadius);
void SpawnBullets(Bullet bullets[], int maxBullets, float &bulletTimer, float bulletInterval, Player player);
void PlayerDash(Player &player, float distance, Vector2 dashDirection);
void DrawEnemies(Enemy enemies[], int maxEnemies);
void InitiateEnemies(Enemy enemies[], int maxEnemies);
void DrawPlayer(Player &player);
void UpdateEnemies(Enemy enemies[], int maxEnemies, Player &player, Bullet bullets[], int maxBullets);
bool CheckBulletEnemyCollision(const Bullet &bullet, const Enemy &enemy);
void HandleBulletEnemyCollision(Bullet &bullet, Enemy &enemy);
void TriggerBulletEnemyCollision(Player &player, Bullet bullets[], Enemy enemies[], int maxBullets, int maxEnemies);
void HandleExp(Player &player);
bool CheckPlayerEnemyCollision(Player &player, const Enemy &enemy);
void TriggerPlayerEnemyCollision(Player &player, Enemy enemies[], size_t enemyCount);
void HandleEnemyPlayerCollision(Player &player, Enemy enemies[]);
void ResetGame(Player &player, Enemy enemies[], int maxEnemies, Bullet bullets[], int maxBullets, int &currentLevel);

// MAIN
int main()
{
    InitWindow(screenWidth, screenHeight, "Game");
    InitAudioDevice();

    // VARIABLES
    // Player
    Vector2 playerPos = {screenWidth / 2, screenHeight / 2};
    Vector2 playerSpeed = {0, 0};
    float dashDistance = 44.0f;

    // Sound
    Sound sound = LoadSound("sounds/level_up.wav");

    // Enemies
    const int maxEnemies = 30;
    Enemy enemies[maxEnemies];

    // State
    Color backgroundColor = GRAY;
    int currentLevel = 1;

    // Bullets
    const int maxBullets = 8;
    Bullet bullets[maxBullets] = {0};
    const int bulletRadius = 10;
    float bulletTimer = 0.0f;
    float bulletInterval = 0.7f;

    // Initiate player
    Player player = {
        playerPos,
        playerSpeed,
        20,
        BLACK,
        100,
        1,
        0};

    // Initiate enemies
    InitiateEnemies(enemies, maxEnemies);

    // MAIN LOOP
    SetTargetFPS(60);
    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(backgroundColor);

        // Bullets
        DrawBullets(bullets, maxBullets, bulletRadius);
        // SpawnBullets(bullets, maxBullets, bulletTimer, bulletInterval, player);
        UpdateBullets(bullets, maxBullets);

        // Player
        DrawPlayer(player);
        HandleExp(player);
        UpdatePlayer(player, bullets, maxBullets, dashDistance);

        // Enemies
        DrawEnemies(enemies, maxEnemies);
        UpdateEnemies(enemies, maxEnemies, player, bullets, maxBullets);

        // Collisions
        TriggerBulletEnemyCollision(player, bullets, enemies, maxBullets, maxEnemies);
        TriggerPlayerEnemyCollision(player, enemies, maxEnemies);

        // Draw UI
        DrawText(TextFormat("Level: %d", player.lvl), 10, 10, 30, ORANGE);
        DrawText(TextFormat("Experience: %d", player.exp), 10, 40, 30, ORANGE);
        DrawText(TextFormat("HP: %d", player.hp), 10, 70, 30, ORANGE);
        DrawText(TextFormat("Stage: %d", currentLevel), 800, 40, 40, BLACK);

        if (player.hp <= 0)
        {
            DrawText("GAME OVER", screenWidth / 2 - 150, screenHeight / 2 - 30, 60, RED);
            DrawText("Press R to Restart", screenWidth / 2 - 200, screenHeight / 2 + 40, 40, DARKGRAY);

            if (IsKeyPressed(KEY_R))
            {
                ResetGame(player, enemies, maxEnemies, bullets, maxBullets, currentLevel);
            }
        }

        EndDrawing();
    }

    UnloadSound(sound);
    CloseAudioDevice();
    CloseWindow();
}

// Player

void UpdatePlayer(Player &player, Bullet bullets[], int maxBullets, float dashDistance)
{
    Vector2 movementInput = {0, 0};

    if (IsKeyDown(KEY_RIGHT))
    {
        movementInput.x += 1;
    }

    if (IsKeyDown(KEY_LEFT))
    {
        movementInput.x -= 1;
    }

    if (IsKeyDown(KEY_DOWN))
    {
        movementInput.y += 1;
    }

    if (IsKeyDown(KEY_UP))
    {
        movementInput.y -= 1;
    }

    // Dash when space bar is pressed
    if (IsKeyPressed(KEY_SPACE))
    {
        // Use the movement input as the dash direction
        PlayerDash(player, 50.0f, movementInput);
    }

    // Normalize the movement input to ensure consistent speed
    movementInput = Vector2Normalize(movementInput);

    // Update player speed based on the movement input
    player.speed = Vector2Scale(movementInput, 5); // Adjust speed as needed

    // Update player position based on the speed vector
    player.position.x += player.speed.x;
    player.position.y += player.speed.y;

    // Ensure player stays within the screen bounds
    player.position.x = Clamp(player.position.x, player.radius, screenWidth - player.radius);
    player.position.y = Clamp(player.position.y, player.radius, screenHeight - player.radius);
}
void PlayerDash(Player &player, float dashDistance, Vector2 dashDirection)
{
    // Store the current speed for later restoration
    Vector2 originalSpeed = player.speed;

    // Normalize the dash direction to ensure consistent dash distance
    dashDirection = Vector2Normalize(dashDirection);

    // Calculate the dash vector based on the normalized dash direction
    Vector2 dashVector = Vector2Scale(dashDirection, dashDistance);

    // Apply the dash vector to the player's position
    player.position = Vector2Add(player.position, dashVector);

    // Restore the original speed
    player.speed = originalSpeed;
}
void DrawPlayer(Player &player)
{
    DrawCircleV(player.position, player.radius, player.color);
}
void HandleExp(Player &player)
{
    if (player.exp == 100)
    {
        player.lvl++;
        player.exp = 0;
    }
}

// Enemies
void InitiateEnemies(Enemy enemies[], int maxEnemies)
{
    for (int i = 0; i < maxEnemies; ++i)
    {
        enemies[i].position = {GetRandomValue(0, screenWidth), GetRandomValue(0, screenHeight)};
        enemies[i].radius = 30;
        enemies[i].color = {GetRandomValue(0, 255), GetRandomValue(0, 255), GetRandomValue(0, 255), 255};
        enemies[i].speed = {GetRandomValue(-4, 4),
                            GetRandomValue(-4, 4)};
    }
}
void DrawEnemies(Enemy enemies[], int maxEnemies)
{
    for (int i = 0; i < maxEnemies; i++)
    {
        DrawCircleV(enemies[i].position, enemies[i].radius, enemies[i].color);
    }
}
void UpdateEnemies(Enemy enemies[], int maxEnemies, Player &player, Bullet bullets[], int maxBullets)
{
    int activeEnemies = 0;

    // Enemy movement
    for (int i = 0; i < maxEnemies; i++)
    {
        activeEnemies++;
        enemies[i].position.x += enemies[i].speed.x;
        enemies[i].position.y += enemies[i].speed.y;

        // Bouncing on Walls
        if (enemies[i].position.x <= 0 + enemies[i].radius || enemies[i].position.x >= screenWidth - enemies[i].radius)
        {
            enemies[i].speed.x *= -1;
        }
        if (enemies[i].position.y >= screenHeight - enemies[i].radius || enemies[i].position.y <= 0 + enemies[i].radius)
        {
            enemies[i].speed.y *= -1;
        }
    }
}

// Collisions
bool CheckBulletEnemyCollision(const Bullet &bullet, const Enemy &enemy)
{
    return (CheckCollisionCircles(bullet.position, bullet.radius, enemy.position, enemy.radius));
}
void HandleBulletEnemyCollision(Bullet &bullet, Enemy &enemy)
{
    bullet.active = false;
    enemy.position.x = -100;
}
void TriggerBulletEnemyCollision(Player &player, Bullet bullets[], Enemy enemies[], int maxBullets, int maxEnemies)
{
    for (int i = 0; i < maxBullets; ++i)
    {
        if (bullets[i].active)
        {
            for (int j = 0; j < maxEnemies; ++j)
            {
                if (CheckBulletEnemyCollision(bullets[i], enemies[j]))
                {
                    HandleBulletEnemyCollision(bullets[i], enemies[j]);
                    player.exp += 10;
                }
            }
        }
    }
}
bool CheckPlayerEnemyCollision(Player &player, const Enemy &enemy)
{
    return (CheckCollisionCircles(player.position, player.radius, enemy.position, enemy.radius));
}
void TriggerPlayerEnemyCollision(Player &player, Enemy enemies[], size_t enemyCount)
{
    auto currentTime = std::chrono::steady_clock::now();
    auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastCollisionTime).count();

    if (elapsedTime >= 1000) // 1000 ms = 1 second
    {
        for (size_t i = 0; i < enemyCount; ++i)
        {
            if (CheckPlayerEnemyCollision(player, enemies[i]))
            {
                HandleEnemyPlayerCollision(player, enemies);

                // Update the last collision time
                lastCollisionTime = currentTime;
                break; // Exit after handling one collision
            }
        }
    }
}

void HandleEnemyPlayerCollision(Player &player, Enemy enemies[])
{
    player.hp -= 50;
    if (player.hp <= 0)
    {
        player.hp = 0;
    }
}
// Bullets
void UpdateBullets(Bullet bullets[], int maxBullets)
{
    for (int i = 0; i < maxBullets; ++i)
    {
        if (bullets[i].active)
        {
            bullets[i].position.x += bullets[i].speed.x;
            bullets[i].position.y += bullets[i].speed.y;

            // Deactivate bullet if it goes out of screen bounds
            if (bullets[i].position.y < 0 || bullets[i].position.y > screenHeight ||
                bullets[i].position.x < 0 || bullets[i].position.x > screenWidth)
            {
                bullets[i].active = false;
            }
        }
    }
}
void DrawBullets(const Bullet bullets[], int maxBullets, int bulletRadius)
{
    for (int i = 0; i < maxBullets; ++i)
    {
        if (bullets[i].active)
        {
            DrawCircleV(bullets[i].position, bulletRadius, bullets[i].color);
        }
    }
}
void SpawnBullets(Bullet bullets[], int maxBullets, float &bulletTimer, float bulletInterval, Player player)
{
    bulletTimer += GetFrameTime();
    if (bulletTimer >= bulletInterval)
    {
        // Initialize bullets with different directions and mark them as active
        for (int i = 0; i < maxBullets; ++i)
        {
            bullets[i].active = true;
            bullets[i].position = player.position;
            bullets[i].color = DARKGRAY; // Adjust bullet color as needed

            // Initialize bullets with different directions
            bullets[0].speed = {0, -10};   // Up
            bullets[1].speed = {0, 10};    // Down
            bullets[2].speed = {-10, 0};   // Left
            bullets[3].speed = {10, 0};    // Right
            bullets[4].speed = {10, -10};  // Up/Right
            bullets[5].speed = {10, 10};   // Down/Right
            bullets[6].speed = {-10, -10}; // Up/Left
            bullets[7].speed = {-10, 10};  // Down/Left
        }

        bulletTimer = 0.0f;
    }
}

void ResetGame(Player &player, Enemy enemies[], int maxEnemies, Bullet bullets[], int maxBullets, int &currentLevel)
{
    player.hp = 100;
    player.exp = 0;
    player.lvl = 1;
    player.position = {screenWidth / 2, screenHeight / 2};

    InitiateEnemies(enemies, maxEnemies);

    for (int i = 0; i < maxBullets; ++i)
    {
        bullets[i].active = false;
    }

    currentLevel = 1;
}