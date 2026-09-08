#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>

#define WIDTH 800
#define HEIGHT 600
#define NUM_FRUITS 5
#define MAX_FRUITS 5

// Random value generate
float GetRandomFloat(float min, float max)
{
    return ((float)rand() / (float)RAND_MAX) * (max - min) + min;
}

// Fruit & Moving_Fruits structure

typedef struct
{
    Texture2D Frame[3];

} Fruit;

Fruit Fruits[NUM_FRUITS];

typedef struct
{
    Vector2 position;
    Vector2 velocity;
    Vector2 slice1position;
    Vector2 slice2position;
    Vector2 slice1velocity;
    Vector2 slice2velocity;
    int type;
    bool active;
    bool sliced;

} Moving_Fruits;

Moving_Fruits moving_Fruits[MAX_FRUITS];

// main function

int main(void)
{
    InitWindow(800, 600, "Fruit Ninja");
    SetTargetFPS(60);
    Texture2D background = LoadTexture("assets/fruit_Ninja_Bg.png");

    // --------------Texture Load----------

    for (int i = 0; i < NUM_FRUITS; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            char filename[50];
            sprintf(filename, "assets/%d%d.png", i + 1, j);
            Fruits[i].Frame[j] = LoadTexture(filename);
        }
    }

    for (int i = 0; i < MAX_FRUITS; i++)
    {
        moving_Fruits[i].active = false;
        moving_Fruits[i].sliced = false;
    }

    // score
    int score = 0, maxScore = 0, life = 3;

    float timer = 0, time = 1;

    while (!WindowShouldClose())
    {
        timer += GetFrameTime();
        if (life > 0)
        {

            // Fruits active and Initial value add
            if (timer >= time)
            {
                timer = 0;
                for (int i = 0; i < MAX_FRUITS; i++)
                {
                    if (!moving_Fruits[i].active)
                    {
                        moving_Fruits[i].type = GetRandomValue(0, NUM_FRUITS - 1);
                        moving_Fruits[i].active = true;
                        moving_Fruits[i].sliced = false;
                        moving_Fruits[i].position.x = GetRandomValue(0, 600);
                        moving_Fruits[i].position.y = HEIGHT;
                        if (moving_Fruits[i].position.x < 300)
                        {
                            moving_Fruits[i].velocity.x = GetRandomFloat(2.0, (4.0 - moving_Fruits[i].position.x / 150));
                        }
                        else
                        {
                            moving_Fruits[i].velocity.x = GetRandomFloat((-moving_Fruits[i].position.x / 150), -2.0);
                        }
                        moving_Fruits[i].velocity.y = GetRandomFloat(-6.2, -5.5);

                        break;
                    }
                }
            }

            // slice check
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
            {
                Vector2 mouse = GetMousePosition();
                for (int i = 0; i < MAX_FRUITS; i++)
                {
                    if (!moving_Fruits[i].active)
                        continue;
                    if (moving_Fruits[i].sliced)
                        continue;

                    int type = moving_Fruits[i].type;
                    Texture2D texture = Fruits[type].Frame[0];

                    Rectangle fruitRect ={moving_Fruits[i].position.x,moving_Fruits[i].position.y,texture.width,texture.height};

                    if (CheckCollisionPointRec(mouse, fruitRect))
                    {
                        moving_Fruits[i].sliced = true;
                        score += 10;
                        if (score > maxScore)
                        {
                            maxScore = score;
                        }

                        moving_Fruits[i].slice1position = moving_Fruits[i].position;

                        moving_Fruits[i].slice1velocity = moving_Fruits[i].velocity;

                        moving_Fruits[i].slice2position = moving_Fruits[i].position;

                        moving_Fruits[i].slice2velocity = moving_Fruits[i].velocity;
                    }
                }
            }

            // moving value update
            for (int i = 0; i < MAX_FRUITS; i++)
            {

                if (moving_Fruits[i].active)
                {
                    if (!moving_Fruits[i].sliced)
                    {
                        // Move
                        moving_Fruits[i].position.x += moving_Fruits[i].velocity.x;
                        moving_Fruits[i].position.y += moving_Fruits[i].velocity.y * 2.5;
                        moving_Fruits[i].velocity.y += GetFrameTime() * 5.0;

                        // Reached bottom again

                        if (moving_Fruits[i].position.y > HEIGHT)
                        {
                            moving_Fruits[i].active = false;
                            life--;
                        }
                    }
                    else
                    {
                        // Move sliced fruit pieces
                        if (moving_Fruits[i].velocity.x > 0)
                        {
                            moving_Fruits[i].slice1position.x -= moving_Fruits[i].slice1velocity.x;
                            moving_Fruits[i].slice1position.y += moving_Fruits[i].slice1velocity.y * 2.5;
                            moving_Fruits[i].slice1velocity.y += GetFrameTime() * 5.0;

                            moving_Fruits[i].slice2position.x += moving_Fruits[i].slice2velocity.x;
                            moving_Fruits[i].slice2position.y += moving_Fruits[i].slice2velocity.y * 2.5;
                            moving_Fruits[i].slice2velocity.y += GetFrameTime() * 5.0;
                        }
                        else
                        {
                            moving_Fruits[i].slice1position.x += moving_Fruits[i].slice1velocity.x;
                            moving_Fruits[i].slice1position.y += moving_Fruits[i].slice1velocity.y * 2.5;
                            moving_Fruits[i].slice1velocity.y += GetFrameTime() * 5.0;

                            moving_Fruits[i].slice2position.x -= moving_Fruits[i].slice2velocity.x;
                            moving_Fruits[i].slice2position.y += moving_Fruits[i].slice2velocity.y * 2.5;
                            moving_Fruits[i].slice2velocity.y += GetFrameTime() * 5.0;
                        }

                        if (moving_Fruits[i].slice1position.y > HEIGHT || moving_Fruits[i].slice2position.y > HEIGHT)
                        {
                            moving_Fruits[i].active = false;
                        }
                    }
                }
            }
        }

        // DrawTexture
        BeginDrawing();

        ClearBackground(RAYWHITE);

        DrawTexture(background, 0, 0, WHITE);



        for (int i = 0; i < MAX_FRUITS; i++)
        {

            if (moving_Fruits[i].active)
            {
                if (!moving_Fruits[i].sliced)
                {

                    DrawTexture(Fruits[moving_Fruits[i].type].Frame[0], moving_Fruits[i].position.x, moving_Fruits[i].position.y, WHITE);
                }

                else
                {

                    DrawTexture(Fruits[moving_Fruits[i].type].Frame[1], moving_Fruits[i].slice1position.x, moving_Fruits[i].slice1position.y, WHITE);
                    DrawTexture(Fruits[moving_Fruits[i].type].Frame[2], moving_Fruits[i].slice2position.x, moving_Fruits[i].slice2position.y, WHITE);
                }
            }
        }

        DrawText(TextFormat("Score: %d", score), 20, 20, 25, YELLOW);
        DrawText(TextFormat("High Score: %d", maxScore), 300, 20, 25, YELLOW);
        DrawText(TextFormat("Life: %d", life), 650, 20, 25, YELLOW);
        
        // Game Over

        if (life <= 0)
        {

            DrawText("Game Over!", WIDTH / 2 - 50, HEIGHT / 2 - 10, 20, WHITE);
            DrawText(TextFormat("Final Score: %d", score), WIDTH / 2 - 50, HEIGHT / 2 + 20, 20, WHITE);
            DrawText("Press R to Restart", WIDTH / 2 - 50, HEIGHT / 2 + 50, 20, WHITE);
            if (IsKeyPressed(KEY_R))
            {
                score = 0;
                life = 3;
                for (int i = 0; i < MAX_FRUITS; i++)
                {
                    moving_Fruits[i].active = false;
                    moving_Fruits[i].sliced = false;
                }
            }
        }

        EndDrawing();
    }

    UnloadTexture(background);
    for (int i = 0; i < NUM_FRUITS; i++)
    {

        for (int j = 0; j < 3; j++)
        {
            UnloadTexture(Fruits[i].Frame[j]);
        }
    }
    CloseWindow();
    return 0;
}