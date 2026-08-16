#include "raylib.h"

int main(void)
{
    InitWindow(800, 600, "Fruit Ninja");
    SetTargetFPS(60);
    Texture2D background = LoadTexture("assets/fruit_Ninja_Bg.png");
    float x=390, y=600;
    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        if(IsKeyPressed(KEY_SPACE)) y-=10;
        DrawTexture(background, 0, 0, WHITE);
        DrawCircle(x, y, 20, RED);
        EndDrawing();
    }
    UnloadTexture(background);

    CloseWindow();

    return 0;
}
