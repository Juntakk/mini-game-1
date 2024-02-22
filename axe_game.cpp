#include "raylib.h"
#include "raymath.h"

struct Player
{
    Vector2 position;
    float radius;
    Color color;
};

struct Axe
{
    Rectangle rect;
    Vector2 speed;
    Color color;
};

struct Bullet
{
    Vector2 position;
    Vector2 speed;
    Color color;
    bool active;
};

// Window dimensions
const int width{1800};
const int height{900};

// Main functions
void UpdatePlayer(Player &player, Bullet bullets[], int maxBullets);
void UpdateAxes(Axe &axe);
bool CheckCollision(Player &player, Axe &axe);
void UpdateBullets(Bullet bullets[], int maxBullets);
void DrawBullets(const Bullet bullets[], int maxBullets);
void SpawnBullets(Bullet bullets[], int maxBullets, float &bulletTimer, float bulletInterval, Player player);

int main()
{
    InitWindow(width, height, "Avoid the obstacles");
    Vector2 playerPos = {static_cast<float>(height) / 2, static_cast<float>(width) / 10};

    Player player = {
        playerPos,
        20,
        BLACK};

    const int num_axes = 40;
    Axe axes[num_axes];
    const int maxBullets = 4;
    Bullet bullets[maxBullets] = {0};
    float bulletTimer = 0.0f;
    float bulletInterval = 0.8f;

    for (int i = 0; i < num_axes; ++i)
    {
        axes[i].rect.width = 40;
        axes[i].rect.height = 40;
        axes[i].rect.x = GetRandomValue(0, width);
        axes[i].rect.y = GetRandomValue(0, height);
        axes[i].speed.x = GetRandomValue(-3, 3);
        axes[i].speed.y = GetRandomValue(-3, 3);
        axes[i].color = RED;
    }

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        UpdatePlayer(player, bullets, maxBullets);
        DrawBullets(bullets, maxBullets);
        UpdateBullets(bullets, maxBullets);
        SpawnBullets(bullets, maxBullets, bulletTimer, bulletInterval, player);

        for (int i = 0; i < num_axes; i++)
        {
            UpdateAxes(axes[i]);

            if (CheckCollision(player, axes[i]))
            {
                player.position = playerPos;
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Draw player
        DrawCircleV(player.position, player.radius, player.color);

        // Draw axes
        for (int i = 0; i < num_axes; i++)
        {
            DrawRectangleRec(axes[i].rect, axes[i].color);
        }

        EndDrawing();
    }
}

void UpdatePlayer(Player &player, Bullet bullets[], int maxBullets)
{
    // Move player with arrow keys
    if (IsKeyDown(KEY_RIGHT))
        player.position.x += 5;
    if (IsKeyDown(KEY_LEFT))
        player.position.x -= 5;
    if (IsKeyDown(KEY_DOWN))
        player.position.y += 5;
    if (IsKeyDown(KEY_UP))
        player.position.y -= 5;

    // Ensure player stays within the screen bounds
    player.position.x = Clamp(player.position.x, player.radius, width - player.radius);
    player.position.y = Clamp(player.position.y, player.radius, height - player.radius);
}

void UpdateAxes(Axe &axe)
{
    // Move the Axes
    axe.rect.x += axe.speed.x;
    axe.rect.y += axe.speed.y;

    // Bounce off the screen edges
    if ((axe.rect.x <= 0) || (axe.rect.x >= width - axe.rect.width))
        axe.speed.x *= -1;
    if ((axe.rect.y <= 0) || (axe.rect.y >= height - axe.rect.height))
        axe.speed.y *= -1;
}

bool CheckCollision(Player &player, Axe &axe)
{
    Vector2 closestPoint;
    closestPoint.x = Clamp(player.position.x, axe.rect.x, axe.rect.x + axe.rect.width);
    closestPoint.y = Clamp(player.position.y, axe.rect.y, axe.rect.y + axe.rect.height);

    float distance = Vector2Distance(player.position, closestPoint);

    return distance < player.radius;
}

void UpdateBullets(Bullet bullets[], int maxBullets)
{
    for (int i = 0; i < maxBullets; ++i)
    {
        if (bullets[i].active)
        {
            bullets[i].position.x += bullets[i].speed.x;
            bullets[i].position.y += bullets[i].speed.y;

            // Deactivate bullet if it goes out of screen bounds
            if (bullets[i].position.y < 0 || bullets[i].position.y > height ||
                bullets[i].position.x < 0 || bullets[i].position.x > width)
            {
                bullets[i].active = false;
            }
        }
    }
}

void DrawBullets(const Bullet bullets[], int maxBullets)
{
    for (int i = 0; i < maxBullets; ++i)
    {
        if (bullets[i].active)
        {
            DrawCircleV(bullets[i].position, 10, bullets[i].color);
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
            bullets[0].speed = {0, -10}; // Up
            bullets[1].speed = {0, 10};  // Down
            bullets[2].speed = {-10, 0}; // Left
            bullets[3].speed = {10, 0};  // Right
        }

        bulletTimer = 0.0f;
    }
}
