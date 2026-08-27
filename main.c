#include "raylib.h"
#include<stdio.h>
#define WIDTH 800
#define HEIGHT 600
#define NUM_FRUITS 5
#define MAX_FRUITS 5
typedef struct{
    Texture2D Frame[3];
}Fruit;
Fruit Fruits[NUM_FRUITS];
typedef struct{
    Vector2 position;
    Vector2 velocity;
    int type;
    bool active;

}Moving_Fruits;
Moving_Fruits moving_Fruits[MAX_FRUITS];

int main(void)
{
    InitWindow(800, 600, "Fruit Ninja");
    SetTargetFPS(60);
    Texture2D background = LoadTexture("assets/fruit_Ninja_Bg.png");
    for(int i=0;i<NUM_FRUITS;i++){
        for(int j=0;j<3;j++){
            char filename[50];
            sprintf(filename,"assets/%d%d.png",i+1,j);
            Fruits[i].Frame[j]=LoadTexture(filename);
        }
    }
    for(int i=0;i<MAX_FRUITS;i++){
        moving_Fruits[i].active = false;
    }
    float timer=0,time=1;

    while (!WindowShouldClose())
    {
        timer+=GetFrameTime();
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawTexture(background, 0, 0, WHITE);
        if(timer>time){
            timer=0;
            for(int i=0;i<MAX_FRUITS;i++){
                if(!moving_Fruits[i].active){
                    moving_Fruits[i].type=GetRandomValue(0,NUM_FRUITS-1);
                    moving_Fruits[i].active=true;
                    moving_Fruits[i].position.x=GetRandomValue(80,WIDTH-150);
                    moving_Fruits[i].position.y=HEIGHT+20;
                    moving_Fruits[i].velocity.x=GetRandomValue(-2,2);
                    moving_Fruits[i].velocity.y=5;
                    if(moving_Fruits[i].velocity.x==0)moving_Fruits[i].velocity.x=-2;
                    break;
                }
            }
        }
        for(int i=0;i<MAX_FRUITS;i++){
            if(moving_Fruits[i].active){
                // Move
                moving_Fruits[i].position.x +=moving_Fruits[i].velocity.x;

                moving_Fruits[i].position.y -=moving_Fruits[i].velocity.y;


                // Left / Right boundary
                if(moving_Fruits[i].position.x > WIDTH-130||moving_Fruits[i].position.x < -50){
                    moving_Fruits[i].velocity.x *= -1;
                }


                // Reached top
                if(moving_Fruits[i].position.y < -50){
                    moving_Fruits[i].velocity.y *= -1;
                }


                // Reached bottom again
                if(moving_Fruits[i].position.y > HEIGHT + 50){
                    moving_Fruits[i].active = false;
                }
            }
           
        }

        //DrawTexture 
        for(int i=0;i<MAX_FRUITS;i++){
            if(moving_Fruits[i].active){
                DrawTexture(Fruits[moving_Fruits[i].type].Frame[0],moving_Fruits[i].position.x,moving_Fruits[i].position.y,WHITE);
            }
        }

        EndDrawing();
    }
    UnloadTexture(background);
        for(int i = 0; i < NUM_FRUITS; i++){
            for(int j = 0; j < 3; j++){
                UnloadTexture(Fruits[i].Frame[j]);
            }
        }

    CloseWindow();

    return 0;
}