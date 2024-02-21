#include "raylib.h"
#include "raymath.h"

struct Player {
    Vector2 position;
    float radius;
    Color color;
};

struct Axe {
    Rectangle rect;
    Vector2 speed;
    Color color;
};

// Window dimensions
    const int width{1800};
    const int height{900};

//Main functions
void UpdatePlayer(Player& player);
void UpdateAxes(Axe& axe);
bool CheckCollision(Player& player, Axe& Axe);

int main()
{
    InitWindow(width, height, "Avoid the obstacles");
    Vector2 playerPos = {static_cast<float>(height)/2, static_cast<float>(width)/10};

    Player player = {
        playerPos,
        30,
        BLACK
    };

    const int num_axes = 4;
    Axe axes[num_axes];

    for (int i = 0; i < num_axes; ++i) {
        axes[i].rect.width = 80;
        axes[i].rect.height = 20;
        axes[i].rect.x = GetRandomValue(0, width - axes[i].rect.width);
        axes[i].rect.y = GetRandomValue(0, height - axes[i].rect.height);
        axes[i].speed.x = GetRandomValue(-3, 3);
        axes[i].speed.y = GetRandomValue(-3, 3);
        axes[i].color = RED;
    }

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        UpdatePlayer(player);

        for (int i = 0; i < num_axes; i++)
        {
            UpdateAxes(axes[i]);

            if(CheckCollision(player, axes[i])){
                player.position = playerPos;
            }
        }
        
        BeginDrawing();
        ClearBackground(RAYWHITE);

        //Draw player
        DrawCircleV(player.position,player.radius, player.color);

        //Draw axes
        for (int i = 0; i < num_axes; i++)
        {
            DrawRectangleRec(axes[i].rect, axes[i].color);
        }

        EndDrawing();
    }
}

void UpdatePlayer(Player& player){

    //Move player
    if (IsKeyDown(KEY_RIGHT)) player.position.x += 5;
    if (IsKeyDown(KEY_LEFT)) player.position.x -= 5;
    if (IsKeyDown(KEY_DOWN)) player.position.y += 5;
    if (IsKeyDown(KEY_UP)) player.position.y -= 5;

    // Ensure player stays within the screen bounds
    player.position.x = Clamp(player.position.x, player.radius, width - player.radius);
    player.position.y = Clamp(player.position.y, player.radius, height - player.radius);
}

void UpdateAxes(Axe& Axe){
    // Move the Axes
    Axe.rect.x += Axe.speed.x;
    Axe.rect.y += Axe.speed.y;

    // Bounce off the screen edges
    if ((Axe.rect.x <= 0) || (Axe.rect.x >= width - Axe.rect.width)) Axe.speed.x *= -1;
    if ((Axe.rect.y <= 0) || (Axe.rect.y >= height - Axe.rect.height)) Axe.speed.y *= -1;
}

bool CheckCollision(const Player& player, const Axe& axe){
    Vector2 closestPoint;
    closestPoint.x = Clamp(player.position.x, axe.rect.x, axe.rect.x + axe.rect.width);
    closestPoint.y = Clamp(player.position.y, axe.rect.y, axe.rect.y + axe.rect.height);

    float distance = Vector2Distance(player.position, closestPoint);

    return distance < player.radius;
}