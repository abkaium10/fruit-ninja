Fruit Ninja
A small Fruit Ninja-style slicing game made in C using raylib.
The game focuses on mouse-based fruit slicing, score tracking, combos, special fruits, bombs, visual effects, and a simple menu system. Player scores are saved locally so different players can keep their own high scores.
Features
- Fruit slicing with the mouse
- Blade / slash trail effect
- Multiple fruit types with animation frames
- Fruit rotation and movement
- Fruit slice animation
- Juice particle effect
- Combo system
- Critical hit system
- Special banana worth +20 points
- Bomb/fire hazard
- Three lives
- Increasing gameplay difficulty
- Player name system
- Local leaderboard
- Saved high scores using scores.txt
- Loading screen with progress bar
- Main menu with:
  - Play
  - How to Play
  - Leaderboard
  - Credits
  - Settings
- Music and sound effect controls
- Game-over screen
- Background music and gameplay sound effects
How to Play
1. Start the game.
2. Enter your player name.
3. Click START GAME.
4. Hold the left mouse button and move the mouse across the fruits.
5. Slice fruits to earn points.
6. Missing a fruit costs one life.
7. The special banana gives +20 points.
8. Do not slice the bomb/fire.
9. Build combos by slicing fruits within the combo time.
Controls
Control	Action
Left Mouse Button	Slice fruits
Mouse Movement	Move the blade
Enter	Start the game
Escape	Go back


Scoring
- Normal fruit: +10 points
- Special banana: +20 points
- Combos can increase the score
- Critical hits can add an extra bonus
- Missing a fruit removes one life
- Slicing the bomb ends the game
Difficulty
The game gradually increases the number of fruits and the frequency of hazards as the game continues.
Different gameplay stages use different fruit spawn amounts and bomb/fire spawn intervals.
Leaderboard
Scores are stored locally in:
scores.txt
The game supports up to 50 player records. The leaderboard displays the top local scores.
If scores.txt does not exist, the game creates it when player data needs to be saved.
Project Structure
A typical project layout is:
Fruit-Ninja/
│
├── assets/
│   ├── *.png
│   ├── *.mp3
│   └── *.wav
│
├── main.c
├── scores.txt
└── README.md
The exact asset filenames should match the paths used in the source code.
Requirements
- C compiler
- raylib
- raylib-supported operating system
- Required game assets inside the assets folder
The source uses standard C libraries together with raylib:
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
Running the Project
Make sure raylib is installed and that the assets folder is located beside the executable or working directory.
For a GCC/MinGW setup, a command similar to the following can be used after configuring raylib on your system:
gcc main.c -o FruitNinja -lraylib -lopengl32 -lgdi32 -lwinmm
The exact compile command may be different depending on how raylib was installed.
Then run:
FruitNinja.exe
Assets
The project uses image, music, and sound assets for the game UI and gameplay.
Some of the external sources listed in the game's Credits screen are:
- KHInsider
- Mixkit
- Fruit Ninja Wiki / Fandom
- Pexels
- Gemini
Credits
Developed by:

Abdul Kaium Mia
ID: 2505126

Tawsif Mollik Tushar
ID: 2505127

Built With
- C
- raylib
Notes
This is a local single-player game. The leaderboard uses a local text file rather than an online database, so scores are stored only on the machine where the game is running.