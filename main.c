#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#define WIDTH 800
#define HEIGHT 600
#define NUM_FRUITS 9
#define MAX_FRUITS 9
#define BOMB_RESPAWN_MIN 3.0f
#define BOMB_RESPAWN_MAX 7.0f
#define USE_JUICE_EFFECT 1
// 1 means true bujhasse
#define MAX_PLAYER_NAME 20
#define MAX_PLAYERS 50
#define SCORE_FILE "scores.txt"
#define MAX_JUICE_PARTICLES 200
#define TRAIL_POINTS 10
#define COMBO_TIME 0.25f
#define COMBO_DISPLAY_TIME 0.8f
#define CRITICAL_DISPLAY_TIME 1.0f
#define CRITICAL_CHANCE 10
// Random value generate
float GetRandomFloat(float min, float max)
{
    return ((float)rand() / (float)RAND_MAX) * (max - min) + min;
}
// UI Button helper
typedef enum
{
    MENU_MAIN = 0,
    MENU_PLAYER,
    MENU_HOW_TO_PLAY,
    MENU_LEADERBOARD,
    MENU_CREDITS,
    MENU_SETTINGS
} MenuScreen;
bool DrawMenuButton(const char *text, Rectangle bounds)
{
    Vector2 mouse = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mouse, bounds);
    Color normal = (Color){105, 57, 34, 255};
    Color hover = (Color){124, 69, 40, 255};
    Color border = (Color){177, 94, 45, 255};
    DrawRectangleRounded(bounds, 0.10f, 8, hovered ? hover : normal);
    DrawRectangleRoundedLinesEx(bounds, 0.10f, 8, 2.0f, hovered ? GOLD : border);
    int fontSize = 24;
    int textWidth = MeasureText(text, fontSize);
    DrawText(text, (int)(bounds.x + (bounds.width - textWidth) / 2), (int)(bounds.y + (bounds.height - fontSize) / 2), fontSize, hovered ? WHITE : (Color){245, 238, 228, 255});
    return hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}
void DrawMenuHeader(const char *title, const char *subtitle)
{
    int titleSize = 42;
    int titleWidth = MeasureText(title, titleSize);
    int subSize = 18;
    int subWidth = 0;
    if (subtitle != NULL)
        subWidth = MeasureText(subtitle, subSize);//kebol width mape
    int paddingX = 25;
    int paddingY = 15;
    int contentWidth = titleWidth;
    if (subtitle != NULL && subWidth > contentWidth)
        contentWidth = subWidth;
    int boxWidth = contentWidth + paddingX * 2;
    int boxHeight = subtitle != NULL ? 95 : 65;
    int boxX = (WIDTH - boxWidth) / 2;
    int boxY = 60;
    // Rounded black background
    DrawRectangleRounded((Rectangle){boxX, boxY, boxWidth, boxHeight}, 0.18f, 12, (Color){0, 0, 0, 155});
    // Subtle border
    DrawRectangleRoundedLinesEx((Rectangle){boxX, boxY, boxWidth, boxHeight}, 0.18f, 12, 1.5f, (Color){255, 255, 255, 70});
    // Title
    DrawText(title, (WIDTH - titleWidth) / 2, 72, titleSize, LIGHTGRAY);
    // Subtitle
    if (subtitle != NULL)
    {
        DrawText(subtitle, (WIDTH - subWidth) / 2, 122, subSize, (Color){210, 205, 198, 255});
    }
}
bool DrawSmallTopButton(const char *text, Rectangle bounds)
{
    Vector2 mouse = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mouse, bounds);
    DrawRectangleRounded(bounds, 0.18f, 8, hovered ? (Color){115, 72, 47, 255} : (Color){69, 43, 31, 245});
    DrawRectangleRoundedLinesEx(bounds, 0.18f, 8, 1.5f, hovered ? GOLD : (Color){92, 59, 42, 255});
    int fontSize = 18;
    int textWidth = MeasureText(text, fontSize);
    DrawText(text, (int)(bounds.x + (bounds.width - textWidth) / 2), (int)(bounds.y + (bounds.height - fontSize) / 2), fontSize, WHITE);
    return hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}
void DrawMenuBackground(Texture2D menu)
{
    Rectangle source = {0, 0, (float)menu.width, (float)menu.height};
    Rectangle dest = {0, 0, (float)WIDTH, (float)HEIGHT};
    DrawTexturePro(menu, source, dest, (Vector2){0, 0}, 0.0f, WHITE);
    DrawRectangle(0, 0, WIDTH, HEIGHT, Fade(BLACK, 0.20f));
}
void PlaySFX(Sound sound, bool enabled)
{
    if (enabled)
        PlaySound(sound);
}


// Blade width
static float BladeWidth(float progress, float speedPower)
{
    float width;

    if (progress < 0.25f)
    {
        float t = progress / 0.15f;
        width = 1.0f + t * 6.5f;
    }
    else if (progress < 0.40f)
    {
        width = 7.5f + speedPower * 2.0f;
    }
    else
    {
        float t = (progress - 0.40f) / 0.60f;
        float maxWidth =9.5f;

        width = maxWidth - t * (maxWidth);
    }

    return width;
}

// Trail direction
static Vector2 TrailDirection(Vector2 *trail, int count, int i)
{
    Vector2 direction;

    if (i == 0)
    {
        direction.x = trail[0].x - trail[1].x;
        direction.y = trail[0].y - trail[1].y;
    }
    else if (i == count - 1)
    {
        direction.x = trail[i - 1].x - trail[i].x;
        direction.y = trail[i - 1].y - trail[i].y;
    }
    else
    {
        direction.x = trail[i - 1].x - trail[i + 1].x;
        direction.y = trail[i - 1].y - trail[i + 1].y;
    }

    float length = sqrtf(direction.x * direction.x + direction.y * direction.y);

    if (length < 0.001f)
        length = 1.0f;

    direction.x /= length;
    direction.y /= length;

    return direction;
}

// Draw blade
static void DrawBlade(Vector2 *trail, int count, float speedPower)
{
    Vector2 blade[TRAIL_POINTS * 2];
    Vector2 border[TRAIL_POINTS * 2];
    Vector2 glow[TRAIL_POINTS * 2];

    for (int i = 0; i < count; i++)
    {
        float progress = (float)i / (float)(count - 1);
        float width = BladeWidth(progress, speedPower);

        Vector2 direction = TrailDirection(trail, count, i);
        Vector2 normal = {-direction.y, direction.x};

        blade[i * 2] = (Vector2){trail[i].x + normal.x * width,trail[i].y + normal.y * width};

        blade[i * 2 + 1] = (Vector2){trail[i].x - normal.x * width,trail[i].y - normal.y * width};

        float borderSize = 3.0f;

        border[i * 2] = (Vector2){trail[i].x + normal.x * (width + borderSize),trail[i].y + normal.y * (width + borderSize)};

        border[i * 2 + 1] = (Vector2){trail[i].x - normal.x * (width + borderSize),trail[i].y - normal.y * (width + borderSize)};

        float glowSize = 5.0f;

        glow[i * 2] = (Vector2){trail[i].x + normal.x * (width + glowSize),trail[i].y + normal.y * (width + glowSize)};

        glow[i * 2 + 1] = (Vector2){trail[i].x - normal.x * (width + glowSize),trail[i].y - normal.y * (width + glowSize)};
    }

    DrawTriangleStrip(glow, count * 2, Fade(SKYBLUE, 0.035f));
    DrawTriangleStrip(border, count * 2, Fade(SKYBLUE, 0.32f));
    DrawTriangleStrip(blade, count * 2, Fade(WHITE, 0.94f));
}
// Player score structure
typedef struct
{
    char name[MAX_PLAYER_NAME + 1];
    int highScore;
} PlayerScore;
PlayerScore players[MAX_PLAYERS];
int playerCount = 0;
// Load saved player scores
void LoadScores(void)
{
    FILE *file = fopen(SCORE_FILE, "r");
    if (file == NULL)
        return;
    playerCount = 0;
    char line[256];
    while (playerCount < MAX_PLAYERS && fgets(line, sizeof(line), file))
    {
        char *tab = strrchr(line, '\t');
        if (tab != NULL)
        {
            *tab = '\0';
            char *newline = strchr(tab + 1, '\n');
            if (newline != NULL)
                *newline = '\0';
            if (line[0] != '\0' && strlen(line) <= MAX_PLAYER_NAME)
            {
                strcpy(players[playerCount].name, line);
                players[playerCount].highScore = atoi(tab + 1);
                playerCount++;
            }
        }
        else
        {
            char oldName[MAX_PLAYER_NAME + 1];
            int oldScore;
            if (sscanf(line, "%20s %d", oldName, &oldScore) == 2)
            {
                strcpy(players[playerCount].name, oldName);
                players[playerCount].highScore = oldScore;
                playerCount++;
            }
        }
    }
    fclose(file);
}
// Save all player scores
void SaveScores(void)
{
    FILE *file = fopen(SCORE_FILE, "w");
    if (file == NULL)
        return;
    for (int i = 0; i < playerCount; i++)
    {
        fprintf(file, "%s\t%d\n", players[i].name, players[i].highScore);
    }
    fclose(file);
}
// Find player by name
int FindPlayer(const char *name)
{
    for (int i = 0; i < playerCount; i++)
    {
        if (strcmp(players[i].name, name) == 0)
            return i;
    }
    return -1;
}
// Create player if it does not exist
int GetOrCreatePlayer(const char *name)
{
    int index = FindPlayer(name);
    if (index != -1)
        return index;
    if (playerCount >= MAX_PLAYERS)
    {
        int ranking[MAX_PLAYERS];
        for (int i = 0; i < playerCount; i++)
            ranking[i] = i;
        for (int i = 0; i < playerCount - 1; i++)
        {
            for (int j = i + 1; j < playerCount; j++)
            {
                if (players[ranking[j]].highScore > players[ranking[i]].highScore)
                {
                    int temp = ranking[i];
                    ranking[i] = ranking[j];
                    ranking[j] = temp;
                }
            }
        }
        bool isTopTen[MAX_PLAYERS] = {false};
        int topCount = playerCount < 10 ? playerCount : 10;
        for (int i = 0; i < topCount; i++)
            isTopTen[ranking[i]] = true;
        int removeIndex = -1;
        for (int i = 0; i < playerCount; i++)
        {
            if (!isTopTen[i])
            {
                removeIndex = i;
                break;
            }
        }
        if (removeIndex != -1)
        {
            for (int i = removeIndex; i < playerCount - 1; i++)
                players[i] = players[i + 1];
            playerCount--;
            SaveScores();
        }
    }
    strcpy(players[playerCount].name, name);
    players[playerCount].highScore = 0;
    playerCount++;
    SaveScores();
    return playerCount - 1;
}
// Get overall maximum score
int GetOverallHighScore(void)
{
    int overallHighScore = 0;
    for (int i = 0; i < playerCount; i++)
    {
        if (players[i].highScore > overallHighScore)
            overallHighScore = players[i].highScore;
    }
    return overallHighScore;
}
// Get player who has overall maximum score
int GetOverallHighScorePlayer(void)
{
    int bestPlayer = -1;
    for (int i = 0; i < playerCount; i++)
    {
        if (bestPlayer == -1 || players[i].highScore > players[bestPlayer].highScore)
        {
            bestPlayer = i;
        }
    }
    return bestPlayer;
}
// Fruit structure
typedef struct
{
    Texture2D Frame[4];
} Fruit;
Fruit Fruits[NUM_FRUITS];
// Moving fruit structure
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
    bool bomb;
    float JuiceTimer;
    float rotation;
    float rotationspeed;
} Moving_Fruits;
// NEW
typedef struct
{
    Vector2 position;
    Vector2 velocity;
    float radius;
    float life;
    Color color;
    bool active;
} JuiceParticle;
JuiceParticle juiceParticles[MAX_JUICE_PARTICLES];
// NEW
void SpawnJuiceParticles(Vector2 pos, Color color)
{
    for (int i = 0; i < 35; i++)
    {
        for (int j = 0; j < MAX_JUICE_PARTICLES; j++)
        {
            if (!juiceParticles[j].active)
            {

                juiceParticles[j].active = true;

                juiceParticles[j].position = pos;

                float angle = DEG2RAD * GetRandomFloat(0, 360); // NEW
                cos(angle);
                sin(angle);
                juiceParticles[j].velocity.x =
                    cos(angle) * GetRandomFloat(2, 6);

                juiceParticles[j].velocity.y =
                    sin(angle) * GetRandomFloat(2, 6);

                juiceParticles[j].radius =
                    GetRandomFloat(3, 8);

                juiceParticles[j].life = 1.0f;

                juiceParticles[j].color = color;

                break;
            }
        }
    }
}
// NEW

Moving_Fruits moving_Fruits[MAX_FRUITS];

void ResetGameState(Moving_Fruits moving_Fruits[], int maxFruits, int *score, int *life, float *timer, float *fireTimer, bool *fireActive, float *gameTime, int *fruitAmount, float *fireNextSpawn)
{
    *score = 0;
    *life = 3;
    *timer = 0;
    *fireTimer = 0;
    *fireActive = false;
    *gameTime = 0;
    *fruitAmount = 1;
    *fireNextSpawn = GetRandomFloat(BOMB_RESPAWN_MIN, BOMB_RESPAWN_MAX);
    for (int i = 0; i < maxFruits; i++)
    {
        moving_Fruits[i].active = false;
        moving_Fruits[i].sliced = false;
        moving_Fruits[i].JuiceTimer = 0;
    }
}
int main(void)
{
    InitWindow(WIDTH, HEIGHT, "Fruit Ninja");
    InitAudioDevice();
    SetTargetFPS(60);
    // Music
    Music theme = LoadMusicStream("assets/Themesong.mp3");
    Music bombSound = LoadMusicStream("assets/bomb.wav");
    PlayMusicStream(theme);
    // PlayMusicStream(bombSound);
    SetMusicVolume(theme, 0.5f);
    // Sounds
    Sound sliceSound = LoadSound("assets/slicing.mp3");
    Sound respawnSound = LoadSound("assets/respawn.wav");
    Sound gameoverSound = LoadSound("assets/gameover.wav");
    Sound losePointSound = LoadSound("assets/losingpoint.wav");
    Sound BombBlast = LoadSound("assets/bombBlast.mp3");
    Sound Special = LoadSound("assets/specialFruit.wav");
    Sound comboSound = LoadSound("assets/combo.mp3");
    // Textures
    Texture2D background = LoadTexture("assets/fruit_Ninja_Bg2.png");
    Texture2D Fornt = LoadTexture("assets/fruit_Ninja_Bg2.png");
    Texture2D aftergame = LoadTexture("assets/fruit_Ninja_Bg.png");
    Texture2D aftergametxt = LoadTexture("assets/gameovertxt.png");
    // Texture2D covertxt = LoadTexture("assets/coverpagetxt.jpg");
    Texture2D menu = LoadTexture("assets/menu.png");
    Texture2D LoadingScreen = LoadTexture("assets/load.png");
    Texture2D covertxt = LoadTexture("assets/thumbnail.png");
    // NEW
    Texture2D tushar=LoadTexture("assets/tushar.png");
    Texture2D kaium=LoadTexture("assets/kaium.png");
    Texture2D sir=LoadTexture("assets/sir.png");
    for (int i = 0; i < MAX_JUICE_PARTICLES; i++)
    {
        juiceParticles[i].active = false;
    }
    // NEW
    //  for timer of Drawtext of Special Fruit
    float scorePopupTimer = 0;
    bool showScorePopup = false;
    Vector2 scorePopupPosition;
    // Fruit texture load
    for (int i = 0; i < NUM_FRUITS; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            char filename[50];
            sprintf(filename, "assets/%d%d.png", i + 1, j);
            Fruits[i].Frame[j] = LoadTexture(filename);
        }
    }
    // surutei sob off
    for (int i = 0; i < MAX_FRUITS; i++)
    {
        moving_Fruits[i].active = false;
        moving_Fruits[i].sliced = false;
        moving_Fruits[i].rotation = GetRandomValue(0, 360);           // recheck
        moving_Fruits[i].rotationspeed = GetRandomFloat(-2.0f, 2.0f); // eta speed
        moving_Fruits[i].bomb = false;
        moving_Fruits[i].JuiceTimer = 0;
    }
    int score = 0;
    int maxScore = 0;
    int life = 3;
    bool startgame = false;
    bool gameover = false;
    // Combo system
    int comboCount = 0;
    int comboDisplayCount = 0;
    float comboTimer = 0.0f;
    bool showCombo = false;
    float comboDisplayTimer = 0.0f;
    // Critical hit system
    bool showCritical = false;
    float criticalTimer = 0.0f;
    Vector2 criticalPosition = {0, 0};
    // Loading screen

    bool loading = true;
    float loading_timer = 0.0f;
    const float LOADING_DURATION = 2.8f;
    // Special Fruit
    Texture2D special = LoadTexture("assets/Frenzy_Banana.png");
    Vector2 specialPosition = {-100, 100};
    Vector2 specialVelocity = {0, 0};
    bool specialT = 0;
    float specialTimer = 0;
    float specialNextSpawn = GetRandomFloat(3.0f, 6.0f);
    // Bomb
    Texture2D fire = LoadTexture("assets/fire.png");
    Vector2 firePosition = {-100, -100};
    Vector2 fireVelocity = {0, 0};
    bool fireActive = false;
    float fireTimer = 0;
    float fireNextSpawn = GetRandomFloat(BOMB_RESPAWN_MIN, BOMB_RESPAWN_MAX);
    // Player name input
    char playerName[MAX_PLAYER_NAME + 1] = "";
    int playerNameLength = 0;
    int currentPlayer = -1;
    // Player dropdown
    bool showPlayerScores = false;
    int scoreScroll = 0;
    // Menu state + audio settings
    MenuScreen menuScreen = MENU_MAIN;
    bool musicEnabled = true;
    bool soundEnabled = true;
    // Load previously saved players
    LoadScores();
    float timer = 0;
    float time = 1;
    // timer is resopawn time for the fruits
    float gameTime = 0; // THIS ONE IS FOR LEVEL
    int fruitAmount = 1;
    // Main Game Loop
    while (!WindowShouldClose())
    {
        if (musicEnabled)
            UpdateMusicStream(theme);
        // SPLASH / LOADING SCREEN

        if (loading)
        {
            loading_timer += GetFrameTime();
            float progress = loading_timer / LOADING_DURATION;
            if (progress > 1.0f)
                progress = 1.0f;
            BeginDrawing();
            ClearBackground(BLACK);
            DrawTexturePro(LoadingScreen, (Rectangle){0, 0, (float)LoadingScreen.width, (float)LoadingScreen.height}, (Rectangle){0, 0, (float)WIDTH, (float)HEIGHT}, (Vector2){0, 0}, 0.0f, WHITE);
            DrawRectangle(0, HEIGHT - 105, WIDTH, 105, Fade(BLACK, 0.28f));
            // Loading text.
            const char *loadingText = "LOADING...";
            int loadingFontSize = 28;
            int loadingTextWidth = MeasureText(loadingText, loadingFontSize);
            DrawText(loadingText, (WIDTH - loadingTextWidth) / 2, HEIGHT - 82, loadingFontSize, WHITE);
            float barWidth = WIDTH * 0.39f;
            float barHeight = 24.0f;
            float barX = (WIDTH - barWidth) / 2.0f;
            float barY = HEIGHT - 48.0f;
            Rectangle barOuter = {barX, barY, barWidth, barHeight};
            Rectangle barInner = {barX + 3, barY + 3, (barWidth - 6) * progress, barHeight - 6};
            // Empty bar.
            DrawRectangleRounded(barOuter, 0.35f, 12, Fade(BLACK, 0.78f));
            DrawRectangleRoundedLinesEx(barOuter, 0.35f, 12, 2.0f, WHITE);
            // Animated fill. It grows from left to right until 100%.
            if (barInner.width > 0.0f)
            {
                DrawRectangleRounded(barInner, 0.30f, 12, GOLD);
                float shineX = barX + 3 + barInner.width - 3;
                DrawCircle((int)shineX, (int)(barY + barHeight / 2), 4, WHITE);
            }
            // Percentage under the bar.
            char percentText[16];
            sprintf(percentText, "%d%%", (int)(progress * 100.0f));
            int percentWidth = MeasureText(percentText, 16);
            DrawText(percentText, (WIDTH - percentWidth) / 2, HEIGHT - 22,
                     16, WHITE);
            EndDrawing();
            // Once the bar is completely filled, move directly to the
            // interactive main menu.
            if (progress >= 1.0f)
            {
                loading = false;
                loading_timer = LOADING_DURATION;
                menuScreen = MENU_MAIN;
            }
            continue;
        }
        if (!startgame)
        {
            // MENU SYSTEM

            BeginDrawing();
            DrawMenuBackground(menu);
            Vector2 mouse = GetMousePosition();
            Rectangle helpTop = (Rectangle){WIDTH - 142, 20, 48, 42};
            Rectangle musicTop = (Rectangle){WIDTH - 88, 20, 80, 42};
            if (DrawSmallTopButton("?", helpTop))
            {
                menuScreen = MENU_HOW_TO_PLAY;
            }
            if (DrawSmallTopButton(musicEnabled ? "MUSIC" : "MUTE", musicTop))
            {
                musicEnabled = !musicEnabled;
                if (musicEnabled)
                    PlayMusicStream(theme);
                else
                {
                    StopMusicStream(theme);
                    StopMusicStream(bombSound);
                }
            }
            // MAIN MENU
            if (menuScreen == MENU_MAIN)
            {
                // Small sword slash beside the title.
                DrawLineEx((Vector2){475, 115}, (Vector2){530, 60},
                           5.0f, (Color){220, 235, 245, 255});
                DrawLineEx((Vector2){478, 119}, (Vector2){533, 64},
                           2.0f, Fade(WHITE, 0.8f));
                Rectangle playButton = {280, 205, 240, 54};
                Rectangle howButton = {280, 267, 240, 54};
                Rectangle leaderboardButton = {280, 329, 240, 54};
                Rectangle creditsButton = {280, 391, 240, 54};
                Rectangle settingsButton = {280, 453, 240, 54};
                if (DrawMenuButton("PLAY", playButton))
                    menuScreen = MENU_PLAYER;
                if (DrawMenuButton("HOW TO PLAY", howButton))
                    menuScreen = MENU_HOW_TO_PLAY;
                if (DrawMenuButton("LEADERBOARD", leaderboardButton))
                    menuScreen = MENU_LEADERBOARD;
                if (DrawMenuButton("CREDITS", creditsButton))
                    menuScreen = MENU_CREDITS;
                if (DrawMenuButton("SETTINGS", settingsButton))
                    menuScreen = MENU_SETTINGS;
                DrawText("SLICE. SCORE. BE THE BEST.", 40, 572, 17,
                         (Color){190, 184, 178, 255});
                // Quick SFX toggle at the bottom-right.
                Rectangle sfxButton = {WIDTH - 150, 552, 118, 34};
                if (DrawSmallTopButton(soundEnabled ? "SFX ON" : "SFX OFF", sfxButton))
                    soundEnabled = !soundEnabled;
            }
            // PLAYER NAME / START SCREEN
            else if (menuScreen == MENU_PLAYER)
            {
                DrawMenuHeader("READY TO SLICE", "Enter your player name");
                Rectangle nameBox = {205, 190, 390, 56};
                bool nameHover = CheckCollisionPointRec(mouse, nameBox);
                DrawRectangleRounded(nameBox, 0.10f, 8,
                                     Fade(BLACK, 0.55f));
                DrawRectangleRoundedLinesEx(nameBox, 0.10f, 8, 2,
                                            nameHover ? GOLD : Fade(WHITE, 0.55f));
                if (playerNameLength > 0)
                    DrawText(playerName, 225, 207, 23, WHITE);
                else
                    DrawText("Type your name...", 225, 207, 22, LIGHTGRAY);
                // Keyboard input
                int key = GetCharPressed();
                while (key > 0)
                {
                    if (key >= 32 && key <= 125 &&
                        playerNameLength < MAX_PLAYER_NAME)
                    {
                        playerName[playerNameLength] = (char)key;
                        playerNameLength++;
                        playerName[playerNameLength] = '\0';
                    }
                    key = GetCharPressed();
                }
                if (IsKeyPressed(KEY_BACKSPACE) && playerNameLength > 0)
                {
                    playerNameLength--;
                    playerName[playerNameLength] = '\0';
                }
                Rectangle startButton = {250, 285, 300, 55};
                Rectangle backButton = {250, 355, 300, 50};
                bool startClicked = DrawMenuButton("START GAME", startButton);
                bool backClicked = DrawMenuButton("BACK", backButton);
                if ((startClicked || IsKeyPressed(KEY_ENTER)) && playerNameLength > 0)
                {
                    currentPlayer = GetOrCreatePlayer(playerName);
                    if (currentPlayer != -1)
                    {
                        maxScore = players[currentPlayer].highScore;
                        ResetGameState(moving_Fruits, MAX_FRUITS, &score, &life, &timer, &fireTimer, &fireActive, &gameTime, &fruitAmount, &fireNextSpawn);
                        comboCount = 0;
                        comboTimer = 0.0f;
                        showCombo = false;
                        comboDisplayTimer = 0.0f;
                        showCritical = false;
                        criticalTimer = 0.0f;
                        gameover = false;
                        if (musicEnabled)
                            PlayMusicStream(theme);
                        startgame = true;
                    }
                }
                if (backClicked || IsKeyPressed(KEY_ESCAPE))
                    menuScreen = MENU_MAIN;
                if (playerNameLength == 0)
                    DrawText("Enter a name before starting.", 282, 435, 16, GREEN);
                else
                    DrawText("Press ENTER or click START GAME.", 278, 435, 16, LIGHTGRAY);
            }
            
            // HOW TO PLAY
            
            else if (menuScreen == MENU_HOW_TO_PLAY)
            {
                DrawMenuHeader("HOW TO PLAY", "Become the fastest fruit ninja");
                Rectangle panel = {145, 155, 510, 350};
                DrawRectangleRounded(panel, 0.04f, 8, Fade(BLACK, 0.64f));
                DrawRectangleRoundedLinesEx(panel, 0.04f, 8, 2, Fade(WHITE, 0.35f));
                DrawText("1", 180, 190, 28, GOLD);
                DrawText("Move your mouse over the fruit.", 220, 193, 20, WHITE);
                DrawText("2", 180, 240, 28, GOLD);
                DrawText("Hold LEFT MOUSE and slice the fruit.", 220, 243, 20, WHITE);
                DrawText("3", 180, 290, 28, GOLD);
                DrawText("+10 points for a normal fruit.", 220, 293, 20, WHITE);
                DrawText("4", 180, 340, 28, GOLD);
                DrawText("Special banana gives +20 points.", 220, 343, 20, WHITE);
                DrawText("5", 180, 390, 28, GOLD);
                DrawText("Missing fruit costs one life.", 220, 393, 20, WHITE);
                DrawText("6", 180, 440, 28, RED);
                DrawText("Never slice the bomb!", 220, 443, 20, WHITE);
                Rectangle backButton = {300, 525, 200, 45};
                if (DrawMenuButton("BACK", backButton) || IsKeyPressed(KEY_ESCAPE))
                    menuScreen = MENU_MAIN;
            }
            
            // LEADERBOARD
            else if (menuScreen == MENU_LEADERBOARD)
            {
                DrawMenuHeader("LEADERBOARD", "Top local players");
                Rectangle panel = {160, 150, 480, 370};
                DrawRectangleRounded(panel, 0.04f, 8, Fade(BLACK, 0.68f));
                DrawRectangleRoundedLinesEx(panel, 0.04f, 8, 2, Fade(WHITE, 0.35f));
                DrawText("RANK", 190, 175, 16, LIGHTGRAY);
                DrawText("PLAYER", 275, 175, 16, LIGHTGRAY);
                DrawText("SCORE", 530, 175, 16, LIGHTGRAY);
                // Build ranking indices without changing the saved order.
                int ranking[MAX_PLAYERS];
                for (int i = 0; i < playerCount; i++)
                    ranking[i] = i;
                for (int i = 0; i < playerCount - 1; i++)
                {
                    for (int j = i + 1; j < playerCount; j++)
                    {
                        if (players[ranking[j]].highScore > players[ranking[i]].highScore)
                        {
                            int temp = ranking[i];
                            ranking[i] = ranking[j];
                            ranking[j] = temp;
                        }
                    }
                }
                int visible = playerCount < 10 ? playerCount : 10;
                if (visible == 0)
                {
                    DrawText("No scores saved yet.", 300, 285, 20, LIGHTGRAY);
                }
                else
                {
                    for (int i = 0; i < visible; i++)
                    {
                        int y = 210 + i * 28;
                        if (i == 0)
                            DrawRectangle(180, y - 4, 440, 27, Fade(GOLD, 0.16f));
                        DrawText(TextFormat("%02d", i + 1), 195, y, 18, WHITE);
                        DrawText(players[ranking[i]].name, 275, y, 18, WHITE);
                        DrawText(TextFormat("%d", players[ranking[i]].highScore), 530, y, 18, WHITE);
                    }
                }
                Rectangle backButton = {300, 535, 200, 45};
                if (DrawMenuButton("BACK", backButton) || IsKeyPressed(KEY_ESCAPE))
                    menuScreen = MENU_MAIN;
            }
            // CREDITS
            else if (menuScreen == MENU_CREDITS)
            {
                // DrawMenuHeader("CREDITS", "External resources used by this game");
                
                Rectangle panel = {0, 0, 800, 600};
                DrawRectangleRounded(panel, 0.04f, 8, Fade(BLACK, 0.68f));
                DrawRectangleRoundedLinesEx(panel, 0.04f, 8, 2, Fade(WHITE, 0.35f));
                DrawText("Fruit Ninja",300,10,28,WHITE);
                DrawText("Developed By",320,45,20,GOLD);
                //ekhane sobi
                DrawTexture(tushar,5,60,WHITE);
                DrawTexture(kaium,550,60,WHITE);


                //Nicher lekha
                DrawText("Tawsif Mollik Tushar",10,270,18,WHITE);
                DrawText("Abdul kaium Mia",580,270,18,WHITE);

                DrawText("ID:2505127 ",10,295,18,WHITE);
                DrawText("ID:2505126",580,295,18,WHITE);
                DrawText("Sources:",10,340,18,YELLOW);
                DrawText("Audio+Music : (Khinsider.com)+(mixkit.com)",10,370,18,YELLOW);
                DrawText("Photos : (fruitninja.fandom.com)+(Gemini)+(pexel.com)",10,400,18,YELLOW);


                DrawText("A Journey to be remembered",230, 430, 18, GOLD); 
                DrawText("forever",360, 455, 18, GOLD); 
                Rectangle backButton = {300, 535, 200, 45};
                if (DrawMenuButton("BACK", backButton) ||
                    IsKeyPressed(KEY_ESCAPE))
                    menuScreen = MENU_MAIN;
            }
            // SETTINGS
            else if (menuScreen == MENU_SETTINGS)
            {
                DrawMenuHeader("SETTINGS", "Audio controls");
                Rectangle musicButton = {245, 190, 310, 58};
                Rectangle soundButton = {245, 265, 310, 58};
                Rectangle backButton = {300, 370, 200, 50};
                if (DrawMenuButton(musicEnabled ? "MUSIC  ON" : "MUSIC  OFF", musicButton))
                {
                    musicEnabled = !musicEnabled;
                    if (musicEnabled)
                        PlayMusicStream(theme);
                    else
                    {
                        StopMusicStream(theme);
                        StopMusicStream(bombSound);
                    }
                }
                if (DrawMenuButton(soundEnabled ? "SOUND  ON" : "SOUND  OFF", soundButton))
                {
                    soundEnabled = !soundEnabled;
                }
                // DrawText("Music controls the background theme and bomb warning.",
                // 190, 340, 15, LIGHTGRAY);
                // DrawText("Sound controls slicing, game-over and other SFX.",
                // 198, 360, 15, LIGHTGRAY);
                if (DrawMenuButton("BACK", backButton) || IsKeyPressed(KEY_ESCAPE))
                    menuScreen = MENU_MAIN;
            }
            EndDrawing();
            continue;
        }
        // FOR FRUIT RESPAWN
        timer += GetFrameTime();
        for (int i = 0; i < MAX_JUICE_PARTICLES; i++)
        {
            if (juiceParticles[i].active)
            {
                juiceParticles[i].position.x += juiceParticles[i].velocity.x;
                juiceParticles[i].position.y += juiceParticles[i].velocity.y;
                juiceParticles[i].velocity.y += 0.15f;
                juiceParticles[i].life -= GetFrameTime();
                if (juiceParticles[i].life <= 0)
                {
                    juiceParticles[i].active = false;
                }
            }
        }
        // NEW


        // Difficulty Control
        gameTime += GetFrameTime();
        if (gameTime > 10)
        {
            fruitAmount = 1;
        }
        if (gameTime > 30)
        {
            fruitAmount = GetRandomValue(1, 3);
            fireNextSpawn = GetRandomValue(2.0f, 5.5f);
        }
        if (gameTime > 60)
        {
            fruitAmount = GetRandomValue(1, 5);
            fireNextSpawn = GetRandomValue(1.50f, 5.0f);
            time = 0.8f;
        }
        // Nightmare
        if (gameTime > 60)
        {
            fruitAmount = GetRandomValue(2, 6);
            fireNextSpawn = GetRandomValue(0.5f, 3.0f);
            time = 0.5f;
        }
        if (life > 0)
        {
            // FIRE/BOMB random spawn
            fireTimer += GetFrameTime();
            specialTimer += GetFrameTime();
            if (showScorePopup)
            {
                scorePopupTimer -= GetFrameTime();
                if (scorePopupTimer <= 0)
                {
                    showScorePopup = false;
                }
            }



            // Combo timer
            if (comboTimer > 0.0f)
            {
                
                comboTimer -= GetFrameTime();
                if (comboTimer <= 0.0f)
                {
                    comboTimer = 0.0f;
                    int comboScore = comboCount * 10;
                    if (GetRandomValue(1, 100) <= CRITICAL_CHANCE)
                        {
                            comboScore += 20;
                            showCritical = true;
                            criticalTimer = CRITICAL_DISPLAY_TIME;
                        }
                    if (comboCount >= 2)
                    {
                        comboScore *= 2;
                        comboDisplayCount = comboCount;
                        showCombo = true;
                        comboDisplayTimer = COMBO_DISPLAY_TIME;
                        PlaySFX(comboSound, soundEnabled);


                    }
                    else
                    {
                        comboScore = 10;
                    }
                    score += comboScore;
                    if (score > maxScore)
                        maxScore = score;
                    comboCount = 0;
                }
            }
            if (showCombo)
            {
                comboDisplayTimer -= GetFrameTime();
                if (comboDisplayTimer <= 0.0f)
                    showCombo = false;
            }
            if (showCritical)
            {
                criticalTimer -= GetFrameTime();
                if (criticalTimer <= 0.0f)
                    showCritical = false;
            }
            if (!specialT && specialTimer >= specialNextSpawn)
            {
                specialTimer = 0;
                specialNextSpawn = GetRandomFloat(5.0f, 10.0f);
                specialT = 1;
                specialPosition.x = GetRandomValue(40, WIDTH - 40 - fire.width);
                specialPosition.y = HEIGHT;
                specialVelocity.x = GetRandomFloat(-2.5f, 2.5f);
                specialVelocity.y = GetRandomFloat(-8.f, -10.f);
            }
            if (specialT)
            {
                specialPosition.x += specialVelocity.x;
                specialPosition.y += specialVelocity.y * 2.5f;
                // This is the gravity
                specialVelocity.y += GetFrameTime() * 5.0f;
                if (specialPosition.y > HEIGHT)
                {
                    specialT = 0;
                }
                if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
                {
                    Vector2 mouse1 = GetMousePosition();
                    Rectangle specialRect = {specialPosition.x, specialPosition.y, (float)special.width, (float)special.height};
                    if (CheckCollisionPointRec(mouse1, specialRect))
                    {
                        PlaySFX(Special, soundEnabled);
                        score += 20;
                        showScorePopup = true;
                        scorePopupTimer = 1.0f;
                        scorePopupPosition = specialPosition;
                        specialT = 0;
                    }
                }
            }
            if (!fireActive && fireTimer >= fireNextSpawn)
            {
                fireTimer = 0;
                fireNextSpawn = GetRandomFloat(BOMB_RESPAWN_MIN, BOMB_RESPAWN_MAX);
                fireActive = true;
                if (musicEnabled)
                    PlayMusicStream(bombSound);
                firePosition.x = GetRandomValue(40, WIDTH - 40 - fire.width);
                firePosition.y = HEIGHT;
                fireVelocity.x = GetRandomFloat(-2.5f, 2.5f);
                fireVelocity.y = GetRandomFloat(-5.2f, -4.5f);
            }
            if (fireActive)
            {
                // Move bomb/fire using the same basic physics idea as fruits
                if (musicEnabled)
                    UpdateMusicStream(bombSound);
                firePosition.x += fireVelocity.x;
                firePosition.y += fireVelocity.y * 2.5f;
                fireVelocity.y += GetFrameTime() * 5.0f;
                // Swipe directly over fire/bomb = immediate game over
                if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
                {
                    Vector2 mouse = GetMousePosition();
                    Rectangle fireRect = {firePosition.x, firePosition.y, (float)fire.width, (float)fire.height};
                    if (CheckCollisionPointRec(mouse, fireRect))
                    {
                        StopMusicStream(bombSound);
                        PlaySFX(BombBlast, soundEnabled);
                        life = 0;
                        fireActive = false;
                    }
                }
                if (firePosition.y > HEIGHT || firePosition.x < -fire.width || firePosition.x > WIDTH)
                {
                    StopMusicStream(bombSound);
                    fireActive = false;
                }
            }
            // Fruits active and Initial value add
            if (timer >= time)
            {
                timer = 0;
                for (int f = 0; f < fruitAmount; f++)
                {
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
                            moving_Fruits[i].velocity.y = GetRandomFloat(-5.2f, -4.5f);
                            break;
                        }
                    }
                }
            }
            // Slice check
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
            {
                Vector2 mouse = GetMousePosition();
                for (int i = 0; i < MAX_FRUITS; i++)
                {
                    if (!moving_Fruits[i].active || moving_Fruits[i].sliced)
                        continue;
                    int type = moving_Fruits[i].type;
                    Texture2D texture = Fruits[type].Frame[0];
                    Rectangle fruitRect = {moving_Fruits[i].position.x, moving_Fruits[i].position.y, texture.width, texture.height};
                    if (CheckCollisionPointRec(mouse, fruitRect))
                    {
                        moving_Fruits[i].sliced = true;
                        Color juiceColor;

                        switch (type)
                        {
                        case 0:
                            juiceColor = (Color){247, 221, 203, 255};
                            break;

                        case 1:
                            juiceColor = YELLOW;
                            break;

                        case 2:
                            juiceColor = ORANGE;
                            break;

                        case 3:
                            juiceColor = RED;
                            break;

                        case 4:
                            juiceColor = (Color){131, 195, 70, 255};
                            break;

                        case 5:
                            juiceColor = WHITE;
                            break;

                        case 6:
                            juiceColor = (Color){222, 35, 57, 255};
                            break;

                        case 7:
                            juiceColor = (Color){255, 128, 0, 255};
                            break;

                        case 8:
                            juiceColor = WHITE;
                            break;

                        default:
                            juiceColor = (Color){247, 221, 203, 255};
                        }

                        SpawnJuiceParticles(moving_Fruits[i].position, juiceColor);
                        moving_Fruits[i].JuiceTimer = 0;
                        PlaySFX(sliceSound, soundEnabled);
                        // score += 10;

                        // if (score > maxScore)
                        //     maxScore = score;
                        moving_Fruits[i].slice1position = moving_Fruits[i].position;
                        moving_Fruits[i].slice1velocity = moving_Fruits[i].velocity;
                        moving_Fruits[i].slice2position = moving_Fruits[i].position;
                        moving_Fruits[i].slice2velocity = moving_Fruits[i].velocity;
                        if (comboTimer > 0.0f)
                            comboCount++;
                        else
                            comboCount = 1;
                        comboTimer = COMBO_TIME;
                        criticalPosition = moving_Fruits[i].position;
                    }
                }
            }
            // Moving value update
            for (int i = 0; i < MAX_FRUITS; i++)
            {
                if (moving_Fruits[i].active)
                {
                    moving_Fruits[i].rotation += moving_Fruits[i].rotationspeed;
                    if (!moving_Fruits[i].sliced)
                    {
                        moving_Fruits[i].position.x += moving_Fruits[i].velocity.x;
                        moving_Fruits[i].position.y += moving_Fruits[i].velocity.y * 2.5;
                        moving_Fruits[i].velocity.y += GetFrameTime() * 5.0;
                        // Reached bottom
                        if (moving_Fruits[i].position.y > HEIGHT)
                        {
                            moving_Fruits[i].active = false;
                            PlaySFX(losePointSound, soundEnabled);
                            life--;
                        }
                    }
                    else
                    {
                        // Sliced fruit pieces
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
        // DRAW GAME
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawTexture(background, 0, 0, WHITE);
        //mouse pointer follow

// Mouse pointer follow
static Vector2 Trail[TRAIL_POINTS];
static int TrailCount = 0;
static Vector2 LastMouse = {0, 0};
static bool TrailStarted = false;
static float smoothSpeed = 0.0f;

if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
{
    Vector2 mouse = GetMousePosition();

    if (!TrailStarted)
    {
        LastMouse = mouse;

        for (int i = 0; i < TRAIL_POINTS; i++)
            Trail[i] = mouse;

        TrailCount = 1;
        TrailStarted = true;
        smoothSpeed = 0.0f;
    }

    float dx = mouse.x - LastMouse.x;
    float dy = mouse.y - LastMouse.y;
    float mouseSpeed = sqrtf(dx * dx + dy * dy);

    smoothSpeed = smoothSpeed * 0.84f + mouseSpeed * 0.16f;

    float speedPower = smoothSpeed / 16.0f;

    if (speedPower > 1.0f)
        speedPower = 1.0f;

    int TrailLength = 5 + (int)(speedPower * 20.0f);

    if (TrailLength > TRAIL_POINTS)
        TrailLength = TRAIL_POINTS;

    for (int i = TRAIL_POINTS - 1; i > 0; i--)
        Trail[i] = Trail[i - 1];

    Trail[0] = mouse;

    if (TrailCount < TrailLength)
        TrailCount++;

    if (TrailCount > TrailLength)
        TrailCount--;

    if (TrailCount < 2)
        TrailCount = 2;

    DrawBlade(Trail, TrailCount, speedPower);

    LastMouse = mouse;
}
else
{
    TrailCount = 0;
    TrailStarted = false;
    smoothSpeed = 0.0f;
}
        //mouse pointer follow end
        // Draw fruits
        for (int i = 0; i < MAX_FRUITS; i++)
        {
            if (moving_Fruits[i].active)
            {
                /*if (!moving_Fruits[i].sliced)
                {
                    DrawTexture( Fruits[moving_Fruits[i].type].Frame[0], moving_Fruits[i].position.x, moving_Fruits[i].position.y, WHITE );
                }*/
                if (!moving_Fruits[i].sliced)
                {
                    Texture2D fruitTexture = Fruits[moving_Fruits[i].type].Frame[0];
                    Rectangle source = {0, 0, (float)fruitTexture.width, (float)fruitTexture.height};
                    Rectangle destination = {moving_Fruits[i].position.x, moving_Fruits[i].position.y, (float)fruitTexture.width, (float)fruitTexture.height};
                    Vector2 origin = {(float)fruitTexture.width / 2, (float)fruitTexture.height / 2};
                    DrawTexturePro(fruitTexture, source, destination, origin, moving_Fruits[i].rotation, WHITE);
                }
                else
                {
                    if (moving_Fruits[i].JuiceTimer < 1.0f)
                    {
                        DrawTexture(Fruits[moving_Fruits[i].type].Frame[3],
                                    moving_Fruits[i].position.x,
                                    moving_Fruits[i].position.y,
                                    WHITE);
                        moving_Fruits[i].JuiceTimer += GetFrameTime();
                    }
                    Texture2D slice1Texture = Fruits[moving_Fruits[i].type].Frame[1];
                    Rectangle slice1Source = {0, 0, (float)slice1Texture.width, (float)slice1Texture.height};
                    Rectangle slice1Destination = {moving_Fruits[i].slice1position.x, moving_Fruits[i].slice1position.y, (float)slice1Texture.width, (float)slice1Texture.height};
                    Vector2 slice1Origin = {(float)slice1Texture.width / 2, (float)slice1Texture.height / 2};
                    DrawTexturePro(slice1Texture, slice1Source, slice1Destination, slice1Origin, moving_Fruits[i].rotation, WHITE);
                    Texture2D slice2Texture = Fruits[moving_Fruits[i].type].Frame[2];
                    Rectangle slice2Source = {0, 0, (float)slice2Texture.width, (float)slice2Texture.height};
                    Rectangle slice2Destination = {moving_Fruits[i].slice2position.x, moving_Fruits[i].slice2position.y, (float)slice2Texture.width, (float)slice2Texture.height};
                    Vector2 slice2Origin = {(float)slice2Texture.width / 2, (float)slice2Texture.height / 2};
                    DrawTexturePro(slice2Texture, slice2Source, slice2Destination, slice2Origin, moving_Fruits[i].rotation, WHITE);
                    // DrawTexture( Fruits[moving_Fruits[i].type].Frame[1], moving_Fruits[i].slice1position.x, moving_Fruits[i].slice1position.y, WHITE );
                    // DrawTexture( Fruits[moving_Fruits[i].type].Frame[2], moving_Fruits[i].slice2position.x, moving_Fruits[i].slice2position.y, WHITE );
                }
            }
        }
        // Draw Special Fruit
        if (specialT)
        {
            DrawTexture(special, (int)specialPosition.x, (int)specialPosition.y, WHITE);
        }
        if (showScorePopup)
        {
            DrawText("+20", scorePopupPosition.x, scorePopupPosition.y, 35, YELLOW);
        }
        // Combo text
        if (showCombo && comboDisplayCount >= 2)
        {
            const char *comboText = TextFormat("%d COMBO!", comboDisplayCount);
            int comboFontSize = 60;
            int comboWidth = MeasureText(comboText, comboFontSize);
            int comboX = (WIDTH - comboWidth) / 2;
            DrawText(comboText, comboX, 92, comboFontSize, GOLD);
        }
        // Critical text
        if (showCritical)
        {
            const char *criticalText = "CRITICAL!";
            int criticalFontSize = 44;
            int criticalWidth = MeasureText(criticalText, criticalFontSize);
            int criticalX = (WIDTH - criticalWidth) / 2;
            DrawText(criticalText, criticalX, 158, criticalFontSize, WHITE);
            DrawText("x2", criticalPosition.x, criticalPosition.y, 38, RED);
        }
        // Draw fire / bomb
        if (fireActive)
        {
            DrawTexture(fire, (int)firePosition.x, (int)firePosition.y, WHITE);
        }
        // draw juice Bubble Effect
        for (int i = 0; i < MAX_JUICE_PARTICLES; i++)
        {
            if (juiceParticles[i].active)
            {
                DrawCircleV(juiceParticles[i].position, juiceParticles[i].radius, Fade(juiceParticles[i].color, juiceParticles[i].life));

                DrawCircle(
                    juiceParticles[i].position.x,
                    juiceParticles[i].position.y,
                    juiceParticles[i].radius * 0.3f,
                    WHITE);
            }
        }

        // Modern in-game HUD
        DrawRectangle(12, 12, 776, 72, Fade(BLACK, 0.48f));
        DrawRectangleLinesEx((Rectangle){12, 12, 776, 72}, 2, Fade(WHITE, 0.30f));
        // Player
        DrawText("PLAYER", 28, 22, 13, LIGHTGRAY);
        DrawText(playerName, 28, 42, 21, WHITE);
        // Score
        DrawText("SCORE", 245, 22, 13, LIGHTGRAY);
        DrawText(TextFormat("%d", score), 245, 40, 25, WHITE);
        // High score
        DrawText("HIGH SCORE", 410, 22, 13, LIGHTGRAY);
        DrawText(TextFormat("%d", maxScore), 410, 40, 25, WHITE);
        // Life
        DrawText("LIFE", 675, 22, 13, LIGHTGRAY);
        for (int heart = 0; heart < 3; heart++)
        {
            DrawText(heart < life ? "<3" : "x", 675 + heart * 28, 42, 20, heart < life ? WHITE : Fade(WHITE, 0.35f));
        }
        // GAME OVER
        if (life <= 0)
        {
            DrawTexture(aftergame, 0, 0, WHITE);
            // Dark overlay
            DrawRectangle(0, 0, WIDTH, HEIGHT, Fade(BLACK, 0.42f));
            // Play game over sound + save score once
            if (!gameover)
            {
                StopMusicStream(theme);
                PlaySFX(gameoverSound, soundEnabled);
                // Update player's personal high score
                if (currentPlayer != -1 && score > players[currentPlayer].highScore)
                {
                    players[currentPlayer].highScore = score;
                    maxScore = score;
                }
                // Save score
                SaveScores();
                gameover = true;
            }
            // Game over panel
            DrawRectangle(170, 105, 460, 390, Fade(BLACK, 0.72f));
            DrawRectangleLinesEx((Rectangle){170, 105, 460, 390}, 3, Fade(WHITE, 0.75f));
            DrawText("GAME OVER", 245, 145, 48, WHITE);
            DrawText(TextFormat("PLAYER  %s", playerName), 255, 215, 22, LIGHTGRAY);
            DrawText("FINAL SCORE", 275, 265, 18, LIGHTGRAY);
            DrawText(TextFormat("%d", score), 360, 290, 48, WHITE);
            DrawText(TextFormat("HIGH SCORE  %d", maxScore), 295, 355, 21, WHITE);
            // Restart button
            Rectangle restartButton = {205, 405, 180, 55};
            bool restartClicked = DrawMenuButton("RESTART", restartButton);
            // Menu button
            Rectangle menuButton = {415, 405, 180, 55};
            bool menuClicked = DrawMenuButton("MENU", menuButton);
            // Restart same player
            if (IsKeyPressed(KEY_R) || restartClicked)
            {
                score = 0;
                comboCount = 0;
                comboTimer = 0.0f;
                showCombo = false;
                comboDisplayTimer = 0.0f;
                showCritical = false;
                criticalTimer = 0.0f;
                life = 3;
                PlayMusicStream(theme);
                timer = 0;
                gameover = false;
                fireTimer = 0;
                fireActive = false;
                gameTime = 0;
                fruitAmount = 1;
                fireNextSpawn = GetRandomFloat(BOMB_RESPAWN_MIN, BOMB_RESPAWN_MAX);
                if (currentPlayer != -1)
                {
                    maxScore = players[currentPlayer].highScore;
                }
                for (int i = 0; i < MAX_FRUITS; i++)
                {
                    moving_Fruits[i].active = false;
                    moving_Fruits[i].sliced = false;
                }
            }
            // Return to menu
            if (IsKeyPressed(KEY_M) || menuClicked)
            {
                if (musicEnabled)
                    if (musicEnabled)
                        PlayMusicStream(theme);
                score = 0;
                comboCount = 0;
                comboTimer = 0.0f;
                showCombo = false;
                comboDisplayTimer = 0.0f;
                showCritical = false;
                criticalTimer = 0.0f;
                life = 3;
                timer = 0;
                fireTimer = 0;
                fireActive = false;
                fireNextSpawn = GetRandomFloat(BOMB_RESPAWN_MIN, BOMB_RESPAWN_MAX);
                startgame = false;
                gameover = false;
                gameTime = 0;
                fruitAmount = 1;
                playerName[0] = '\0';
                playerNameLength = 0;
                currentPlayer = -1;
                showPlayerScores = false;
                scoreScroll = 0;
                for (int i = 0; i < MAX_FRUITS; i++)
                {
                    moving_Fruits[i].active = false;
                    moving_Fruits[i].sliced = false;
                }
            }
        }
        EndDrawing();
    }
    // Cleanup
    UnloadTexture(background);
    UnloadTexture(Fornt);
    UnloadTexture(aftergame);
    UnloadTexture(aftergametxt);
    UnloadTexture(covertxt);
    UnloadTexture(menu);
    UnloadTexture(fire);
    UnloadTexture(special);
    for (int i = 0; i < NUM_FRUITS; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            UnloadTexture(Fruits[i].Frame[j]);
        }
    }
    UnloadSound(sliceSound);
    UnloadSound(gameoverSound);
    UnloadSound(respawnSound);
    UnloadSound(losePointSound);
    UnloadSound(BombBlast);
    UnloadSound(Special);
    UnloadSound(comboSound);
    StopMusicStream(theme);
    StopMusicStream(bombSound);
    UnloadMusicStream(theme);
    UnloadMusicStream(bombSound);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
