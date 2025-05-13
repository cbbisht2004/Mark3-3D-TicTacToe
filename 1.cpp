#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string>
#include <iostream>
#include <string>
#include <algorithm>
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#include <SOIL/SOIL.h>       
//added this later
#include <chrono>
using namespace std::chrono;
void resetGame(int value);

//added this later
int vibrateX = -1, vibrateY = -1, vibrateZ = -1;
int vibrationFrames = 0;

//add variables to store hovered cube
int hoveredX = -1, hoveredY = -1, hoveredZ = -1;


auto getTimeInMillis() {
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

GLuint grassTexture;
//
//i added this later.
bool vibrating[3][3][3];
float vibrationStartTime[3][3][3];

//display transitions
#include <chrono>
using Clock = std::chrono::steady_clock;

bool transitioning = false;
float rotateX = 0.0f, rotateY = 0.0f;
float startRotateX = 0.0f, startRotateY = 0.0f;
float targetRotateX = 0.0f, targetRotateY = 0.0f;

const float transitionDuration = 1.0f; // seconds
Clock::time_point transitionStart;


float rotateSpeed = 0.1f;  // Smaller = smoother

int lastX = 0, lastY = 0;
bool isDragging = false;

//players turns in display
std::string playerName[3] = { "", "", "" };  // index 1 and 2 used
int nameEntryPhase = 1; // 1 = Player 1 typing, 2 = Player 2 typing, 0 = done
std::string nameBuffer = "";
bool nameEntryDone = false;

// Add new state variable for opening screen
bool showOpeningScreen = true;
bool playButtonHovered = false;

#define SIZE 3
#define size 3
int board[SIZE][SIZE][SIZE] = { 0 }; // 0 = empty, 1 = X, 2 = O
int currentPlayer = 1;
int playerSymbol[3]; // 1: X, 2: O
int winner = 0;
float spacing = 2.5f;

bool dragging = false;

int winLine[3][3] = { {-1, -1, -1}, {-1, -1, -1}, {-1, -1, -1} };

// Add new texture for opening screen
GLuint openingTexture;

// Add new variable for win message
bool showWinMessage = false;
std::string winMessage = "";

// Add new variable for button glow effect
float buttonGlowIntensity = 0.0f;
bool buttonGlowIncreasing = true;

// Add new variables for sound and text effects
#include <windows.h>  // For PlaySound
#pragma comment(lib, "winmm.lib")  // For sound library

bool titleGlowing = false;
float titleGlowIntensity = 0.0f;
bool titleGlowIncreasing = true;

// Sound variables
bool isBackgroundMusicPlaying = false;
bool isGameMusicPlaying = false;

// Add these near the top with other global variables
float gameTimer = 60.0f; // 60 seconds initial time
bool timerActive = false;
float timerColor[3] = { 0.0f, 1.0f, 0.0f }; // Start with green
bool showGameOver = false;
GLuint jumpscareTexture;
float vibrationIntensity = 0.0f;
bool isVibrating = false;

// Add this near the top with other global variables
float originalTimer = 60.0f; // Store the original timer value

// Add new variable for black screen state
bool showBlackScreen = false;
float blackScreenStartTime = 0.0f;

void playMoveSound(int player) {
    if (player == 1) { // X player
        PlaySound(TEXT("click2.wav"), NULL, SND_ASYNC | SND_FILENAME);
    }
    else { // O player
        PlaySound(TEXT("click2.wav"), NULL, SND_ASYNC | SND_FILENAME);
    }
}

void playWinSound() {
    PlaySound(TEXT("winsound.wav"), NULL, SND_ASYNC | SND_FILENAME);
}

// ---------------------------- SYMBOLS ----------------------------


void drawX() {
    glLineWidth(4);
    glColor3f(0.0f, 1.0f, 0.0f);  // Lime Green (#00FF00)
    glBegin(GL_LINES);
    glVertex3f(-0.3f, -0.3f, 0.51f);
    glVertex3f(0.3f, 0.3f, 0.51f);
    glVertex3f(0.3f, -0.3f, 0.51f);
    glVertex3f(-0.3f, 0.3f, 0.51f);
    glEnd();
}

void drawO() {
    glLineWidth(4);
    glColor3f(0.0f, 0.6f, 1.0f);  // Light blue
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 100; i++) {
        float theta = 2.0f * 3.1415926f * float(i) / float(100);
        float dx = 0.35f * cosf(theta);
        float dy = 0.35f * sinf(theta);
        glVertex3f(dx, dy, 0.51f);
    }
    glEnd();
}
void drawCubeOutline() {
    glColor3f(0.0f, 0.0f, 0.0f); // Black
    glLineWidth(2.0f);

    glBegin(GL_LINES);
    float s = 0.5f;

    float v[8][3] = {
        {-s, -s, -s}, {s, -s, -s}, {s, s, -s}, {-s, s, -s},
        {-s, -s,  s}, {s, -s,  s}, {s, s,  s}, {-s, s,  s}
    };

    int edges[12][2] = {
        {0,1}, {1,2}, {2,3}, {3,0}, // back
        {4,5}, {5,6}, {6,7}, {7,4}, // front
        {0,4}, {1,5}, {2,6}, {3,7}  // sides
    };

    for (int i = 0; i < 12; i++) {
        glVertex3fv(v[edges[i][0]]);
        glVertex3fv(v[edges[i][1]]);
    }

    glEnd();
}


// ---------------------------- RENDER ----------------------------
void drawSymbolOnFrontFace(int state) {
    if (state == 1 || state == 2) {
        glColor3f(1.0f, 1.0f, 1.0f);
        void (*drawSymbol)() = (state == 1) ? drawX : drawO;

        glPushMatrix();
        drawSymbol(); // Draw on the front face
        glPopMatrix();
    }
}
void drawTexturedCube() {
    glBindTexture(GL_TEXTURE_2D, grassTexture);

    glBegin(GL_QUADS);

    // Front Face
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f, 0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5f, -0.5f, 0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5f, 0.5f, 0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, 0.5f, 0.5f);

    // Back Face
    glTexCoord2f(0.0f, 0.0f); glVertex3f(0.5f, -0.5f, -0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f, 0.5f, -0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(0.5f, 0.5f, -0.5f);

    // Top Face
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, 0.5f, 0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5f, 0.5f, 0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5f, 0.5f, -0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, 0.5f, -0.5f);

    // Bottom Face
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5f, -0.5f, -0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5f, -0.5f, 0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, -0.5f, 0.5f);

    // Left Face
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, -0.5f, 0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f, 0.5f, 0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, 0.5f, -0.5f);

    // Right Face
    glTexCoord2f(0.0f, 0.0f); glVertex3f(0.5f, -0.5f, 0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5f, -0.5f, -0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5f, 0.5f, -0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(0.5f, 0.5f, 0.5f);

    glEnd();
}


void drawCube(int state, int highlight, int x, int y, int z) {
    bool isHovered = (x == hoveredX && y == hoveredY && z == hoveredZ);

    if (highlight) {
        glColor3f(1.0f, 0.0f, 0.0f); // Bright red for winning condition
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glColor4f(1.0f, 0.0f, 0.0f, 0.8f); // Additive blending for extra glow
    }
    else if (isHovered) {
        if (state == 1) {
            glColor3f(0.0f, 1.0f, 0.0f); // Lime green glow for X
        }
        else if (state == 2) {
            glColor3f(0.0f, 0.6f, 1.0f); // Blue glow for O
        }
        else {
            glColor3f(0.0f, 1.0f, 0.0f); // Lime green glow for empty (matching X)
        }
    }
    else {
        if (state == 1) {
            glColor3f(0.0f, 1.0f, 0.0f); // Lime green tint for X
        }
        else if (state == 2) {
            glColor3f(0.0f, 0.6f, 1.0f); // Blue tint for O
        }
        else {
            glColor3f(1.0f, 1.0f, 1.0f); // Default white tint for empty
        }
    }

    if (vibrating[x][y][z]) {
        float elapsed = (getTimeInMillis() - vibrationStartTime[x][y][z]) / 1000.0f;

        if (elapsed < 0.5f) {
            // Horror-like erratic trembling
            float jitterX = sin(elapsed * 60.0f + rand() % 100) * 0.05f;
            float jitterY = cos(elapsed * 70.0f + rand() % 100) * 0.05f;
            float jitterZ = sin(elapsed * 90.0f + rand() % 100) * 0.03f;
            glTranslatef(jitterX, jitterY, jitterZ);

            // Leave a faint ghostly after-image (trace)
            glPushMatrix();
            glDisable(GL_DEPTH_TEST);  // So it draws on top
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glColor4f(1.0f, 0.0f, 0.0f, 0.15f); // Red ghost trace

            glTranslatef(-jitterX * 2, -jitterY * 2, -jitterZ * 2); // offset back from current
            glutSolidCube(1.0);  // Draw a faded ghost cube
            glDisable(GL_BLEND);
            glEnable(GL_DEPTH_TEST);
            glPopMatrix();
        }
        else {
            vibrating[x][y][z] = false; // Stop vibration
        }
    }

    glEnable(GL_TEXTURE_2D);
    drawTexturedCube();
    glDisable(GL_TEXTURE_2D);

    // Add translucent overlay for used boxes
    if (state != 0) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        if (state == 1) { // X player
            glColor4f(0.0f, 0.1f, 0.6f, 0.2f); // Dark red with low opacity
        }
        else { // O player
            glColor4f(0.0f, 0.0f, 0.6f, 0.2f); // Dark blue with low opacity
        }

        // Draw overlay cube covering all faces
        glBegin(GL_QUADS);
        // Front face
        glVertex3f(-0.5f, -0.5f, 0.51f);
        glVertex3f(0.5f, -0.5f, 0.51f);
        glVertex3f(0.5f, 0.5f, 0.51f);
        glVertex3f(-0.5f, 0.5f, 0.51f);

        // Back face
        glVertex3f(-0.5f, -0.5f, -0.51f);
        glVertex3f(-0.5f, 0.5f, -0.51f);
        glVertex3f(0.5f, 0.5f, -0.51f);
        glVertex3f(0.5f, -0.5f, -0.51f);

        // Top face
        glVertex3f(-0.5f, 0.51f, -0.5f);
        glVertex3f(0.5f, 0.51f, -0.5f);
        glVertex3f(0.5f, 0.51f, 0.5f);
        glVertex3f(-0.5f, 0.51f, 0.5f);

        // Bottom face
        glVertex3f(-0.5f, -0.51f, -0.5f);
        glVertex3f(-0.5f, -0.51f, 0.5f);
        glVertex3f(0.5f, -0.51f, 0.5f);
        glVertex3f(0.5f, -0.51f, -0.5f);

        // Right face
        glVertex3f(0.51f, -0.5f, -0.5f);
        glVertex3f(0.51f, 0.5f, -0.5f);
        glVertex3f(0.51f, 0.5f, 0.5f);
        glVertex3f(0.51f, -0.5f, 0.5f);

        // Left face
        glVertex3f(-0.51f, -0.5f, -0.5f);
        glVertex3f(-0.51f, -0.5f, 0.5f);
        glVertex3f(-0.51f, 0.5f, 0.5f);
        glVertex3f(-0.51f, 0.5f, -0.5f);
        glEnd();

        glDisable(GL_BLEND);
    }

    drawSymbolOnFrontFace(state);

    // If hovered, draw a glowing outline
    if (isHovered) {
        glLineWidth(3.0f); // Make outline thicker for hover
        glColor3f(1.0f, 1.0f, 0.0f); // Bright yellow outline
        drawCubeOutline();
        glLineWidth(1.0f); // Reset line width
    }
    else {
        drawCubeOutline(); // Normal outline
    }
}


void keyboard(int key, int x, int y) {
    const float step = 5.0f;
    switch (key) {
    case GLUT_KEY_LEFT:  rotateY -= step; break;
    case GLUT_KEY_RIGHT: rotateY += step; break;
    case GLUT_KEY_UP:    rotateX -= step; break;
    case GLUT_KEY_DOWN:  rotateX += step; break;
    }
    glutPostRedisplay();
}

void renderBitmapString(float x, float y, void* font, const std::string& str) {
    glRasterPos2f(x, y);
    for (char c : str) {
        glutBitmapCharacter(font, c);
    }
}

// Add new function to draw scary text
void drawScaryText(float x, float y, const char* text, void* font) {
    // Draw main text in red
    glColor3f(0.8f, 0.0f, 0.0f);
    renderBitmapString(x, y, font, text);

    // Draw shadow effect
    glColor3f(0.2f, 0.0f, 0.0f);
    renderBitmapString(x + 2, y - 2, font, text);
}

//void drawPlayButton(int x, int y, int width, int height, bool isHovered) {
//    // Update glow intensity
//    if (buttonGlowIncreasing) {
//        buttonGlowIntensity += 0.02f;
//        if (buttonGlowIntensity >= 1.0f) buttonGlowIncreasing = false;
//    }
//    else {
//        buttonGlowIntensity -= 0.02f;
//        if (buttonGlowIntensity <= 0.3f) buttonGlowIncreasing = true;
//    }
//
//    // Draw button background
//    if (isHovered) {
//        glColor3f(0.7f + buttonGlowIntensity * 0.3f, 0.1f, 0.1f);
//    }
//    else {
//        glColor3f(0.4f + buttonGlowIntensity * 0.2f, 0.0f, 0.0f);
//    }
//
//    // Draw rectangular button (no rounded corners)
//    glBegin(GL_QUADS);
//    glVertex2f(x, y);
//    glVertex2f(x + width, y);
//    glVertex2f(x + width, y + height);
//    glVertex2f(x, y + height);
//    glEnd();
//
//    // Draw button border with glow effect
//    glColor3f(0.9f + buttonGlowIntensity * 0.1f, 0.2f, 0.2f);
//    glLineWidth(3.0f);
//    glBegin(GL_LINE_LOOP);
//    glVertex2f(x, y);
//    glVertex2f(x + width, y);
//    glVertex2f(x + width, y + height);
//    glVertex2f(x, y + height);
//    glEnd();
//
//    // Draw button text with scary effect
//    int textWidth = glutBitmapLength(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)"PLAY");
//    drawScaryText(x + (width - textWidth) / 2, y + height / 2 + 8, "PLAY", GLUT_BITMAP_TIMES_ROMAN_24);
//}

void drawGlowingText(float x, float y, const char* text, void* font) {
    // Update glow intensity
    if (titleGlowing) {
        if (titleGlowIncreasing) {
            titleGlowIntensity += 0.05f;
            if (titleGlowIntensity >= 1.0f) titleGlowIncreasing = false;
        }
        else {
            titleGlowIntensity -= 0.05f;
            if (titleGlowIntensity <= 0.3f) titleGlowIncreasing = true;
        }
    }

    // Draw main text with glow effect
    glColor3f(0.8f + titleGlowIntensity * 0.2f, 0.0f, 0.0f);
    renderBitmapString(x, y, font, text);

    // Draw shadow effect
    glColor3f(0.2f + titleGlowIntensity * 0.1f, 0.0f, 0.0f);
    renderBitmapString(x + 2, y - 2, font, text);
}

// Add new variables for halo effect
float haloIntensity = 0.0f;
bool haloIncreasing = true;
float haloPulseSpeed = 0.05f;
float haloMaxIntensity = 0.8f;
float haloMinIntensity = 0.2f;

void drawGlowingTextWithHalo(float x, float y, const char* text, void* font) {
    // Update halo intensity
    if (haloIncreasing) {
        haloIntensity += haloPulseSpeed;
        if (haloIntensity >= haloMaxIntensity) haloIncreasing = false;
    }
    else {
        haloIntensity -= haloPulseSpeed;
        if (haloIntensity <= haloMinIntensity) haloIncreasing = true;
    }

    // Draw halo effect (multiple layers for soft glow)
    for (int i = 0; i < 3; i++) {
        float alpha = (haloIntensity * 0.3f) / (i + 1);
        glColor4f(0.8f, 0.0f, 0.0f, alpha);
        renderBitmapString(x - i, y - i, font, text);
        renderBitmapString(x + i, y - i, font, text);
        renderBitmapString(x - i, y + i, font, text);
        renderBitmapString(x + i, y + i, font, text);
    }

    // Draw main text
    glColor3f(0.8f + haloIntensity * 0.2f, 0.0f, 0.0f);
    renderBitmapString(x, y, font, text);
}

// Add this function to handle timer updates
void updateTimer(int value) {
    if (timerActive && !showGameOver) {
        gameTimer -= 0.1f; // Decrease by 0.1 seconds

        // Update color based on remaining time
        if (gameTimer > 30.0f) {
            // Green to Yellow transition
            float t = (60.0f - gameTimer) / 30.0f;
            timerColor[0] = t;
            timerColor[1] = 1.0f;
            timerColor[2] = 0.0f;
        }
        else if (gameTimer > 10.0f) {
            // Yellow to Red transition
            float t = (30.0f - gameTimer) / 20.0f;
            timerColor[0] = 1.0f;
            timerColor[1] = 1.0f - t;
            timerColor[2] = 0.0f;
        }
        else {
            // Red and vibrating
            timerColor[0] = 1.0f;
            timerColor[1] = 0.0f;
            timerColor[2] = 0.0f;
            isVibrating = true;
            vibrationIntensity = 0.1f * (10.0f - gameTimer) / 10.0f;
        }

        if (gameTimer <= 0.0f) {
            showGameOver = true;
            // Play jumpscare sound
            PlaySound(TEXT("outro.wav"), NULL, SND_ASYNC | SND_FILENAME);
            // Schedule exit after 5 seconds
            glutTimerFunc(7000, [](int) { exit(0); }, 0);
            glutPostRedisplay();
        }
        else {
            glutPostRedisplay();
            glutTimerFunc(100, updateTimer, 0);
        }
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    if (showBlackScreen) {
        // Set up 2D orthographic projection for black screen
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, 800, 0, 800, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        // Disable depth test for 2D rendering
        glDisable(GL_DEPTH_TEST);

        // Draw black screen
        glColor3f(0.0f, 0.0f, 0.0f);
        glBegin(GL_QUADS);
        glVertex2f(0.0f, 0.0f);
        glVertex2f(800.0f, 0.0f);
        glVertex2f(800.0f, 800.0f);
        glVertex2f(0.0f, 800.0f);
        glEnd();

        // Check if 2 seconds have passed
        float currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
        if (currentTime - blackScreenStartTime >= 2.0f) {
            showBlackScreen = false;
            showGameOver = true;
            // Play jumpscare sound with simple filename
            PlaySound(TEXT("Insidious Drawing Jumpscare Sound Effect.wav"), NULL, SND_ASYNC | SND_FILENAME);
            // Schedule exit after 5 seconds
            glutTimerFunc(4000, [](int) { exit(0); }, 0);
        }

        // Restore 3D projection
        glEnable(GL_DEPTH_TEST);
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();

        glutPostRedisplay();
    }
    else if (showGameOver) {
        // Set up 2D orthographic projection for game over screen
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, 800, 0, 800, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        // Disable depth test for 2D rendering
        glDisable(GL_DEPTH_TEST);

        // Draw jumpscare texture
        if (jumpscareTexture) {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, jumpscareTexture);
            glColor3f(1.0f, 1.0f, 1.0f);

            glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
            glTexCoord2f(1.0f, 0.0f); glVertex2f(800.0f, 0.0f);
            glTexCoord2f(1.0f, 1.0f); glVertex2f(800.0f, 800.0f);
            glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 800.0f);
            glEnd();

            glDisable(GL_TEXTURE_2D);
        }
        else {
            // If texture failed to load, draw a red screen
            glColor3f(1.0f, 0.0f, 0.0f);
            glBegin(GL_QUADS);
            glVertex2f(0.0f, 0.0f);
            glVertex2f(800.0f, 0.0f);
            glVertex2f(800.0f, 800.0f);
            glVertex2f(0.0f, 800.0f);
            glEnd();
        }

        // Draw GAME OVER text
        glColor3f(1.0f, 0.0f, 0.0f);
        int textWidth = glutBitmapLength(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)"GAME OVER");
        renderBitmapString((800 - textWidth) / 2, 400, GLUT_BITMAP_TIMES_ROMAN_24, "GAME OVER");

        // Restore 3D projection
        glEnable(GL_DEPTH_TEST);
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }
    else if (showOpeningScreen) {
        // Set up 2D orthographic projection for the opening screen
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, 800, 0, 800, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        // Disable depth test for 2D rendering
        glDisable(GL_DEPTH_TEST);

        // Set background color to darker
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Even darker for opening screen
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw the opening screen texture
        if (openingTexture) {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, openingTexture);
            glColor3f(1.0f, 1.0f, 1.0f);  // Reset color to white for texture

            glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
            glTexCoord2f(1.0f, 0.0f); glVertex2f(800.0f, 0.0f);
            glTexCoord2f(1.0f, 1.0f); glVertex2f(800.0f, 800.0f);
            glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 800.0f);
            glEnd();

            glDisable(GL_TEXTURE_2D);
        }

        // Draw game title with halo effect (centered) using largest font
        int titleWidth = glutBitmapLength(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)"MARK³");
        int titleX = (800 - titleWidth) / 2;

        //Draw "MARK" with larger font
        drawGlowingTextWithHalo(titleX, 600, "MARK³", GLUT_BITMAP_TIMES_ROMAN_24);


        // Draw subtitle (centered) with larger font
        // Draw plain subtitle: "A Game of Horror"
        int subtitleWidth = glutBitmapLength(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)"A Game of Horror");
        glColor3f(0.6f, 0.0f, 0.0f);  // Plain red color
        renderBitmapString((800 - subtitleWidth) / 2, 550, GLUT_BITMAP_TIMES_ROMAN_24, "A Game of Horror");

        // Draw flickering "Press Enter to Continue" with scary glow
        int enterWidth = glutBitmapLength(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)"Press Enter to Continue");
        drawScaryText((800 - enterWidth) / 2, 250, "Press Enter to Continue", GLUT_BITMAP_TIMES_ROMAN_24);


        //additional credits
        int credits = glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)"an OpenGl experiment by Devanjali and Chetan");
        drawScaryText((800 - credits) / 2, 40, "an OpenGl experiment by Devanjali and Chetan", GLUT_BITMAP_HELVETICA_18);

        int credits2 = glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)"Submitted to: Armaan Sir.");
        drawScaryText((800 - credits2) / 2, 20, "submitted to: Armaan Sir.", GLUT_BITMAP_HELVETICA_18);


        // Restore 3D projection
        glEnable(GL_DEPTH_TEST);
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }
    else {
        // Original game display code with darker background
        glClearColor(0.02f, 0.05f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Set up 3D perspective
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(45.0, 1.0, 1.0, 100.0);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        // Camera setup with slight horror effect (subtle movement)
        float cameraZ = -15.0f + sin(glutGet(GLUT_ELAPSED_TIME) * 0.001f) * 0.2f;
        glTranslatef(0.0f, 0.0f, cameraZ);

        if (!isDragging && transitioning) {
            auto now = Clock::now();
            float elapsed = std::chrono::duration<float>(now - transitionStart).count();
            float t = elapsed / transitionDuration;

            if (t >= 1.0f) {
                t = 1.0f;
                transitioning = false;
                glutIdleFunc(nullptr);
            }

            float smoothT = t * t * (3 - 2 * t);
            rotateX = startRotateX + (targetRotateX - startRotateX) * smoothT;
            rotateY = startRotateY + (targetRotateY - startRotateY) * smoothT;
            glutPostRedisplay();
        }

        glRotatef(rotateX, 1, 0, 0);
        glRotatef(rotateY, 0, 1, 0);

        // Draw the 3D game board
        bool anyVibrating = false;
        for (int x = 0; x < SIZE; x++) {
            for (int y = 0; y < SIZE; y++) {
                for (int z = 0; z < SIZE; z++) {
                    if (vibrating[x][y][z]) {
                        anyVibrating = true;
                    }

                    glPushMatrix();
                    glTranslatef((x - 1) * spacing, (y - 1) * spacing, (z - 1) * spacing);

                    int isWin = 0;
                    for (int i = 0; i < 3; i++) {
                        if (winLine[i][0] == x && winLine[i][1] == y && winLine[i][2] == z) {
                            isWin = 1;
                            break;
                        }
                    }

                    drawCube(board[x][y][z], isWin, x, y, z);
                    glPopMatrix();
                }
            }
        }

        // Draw player turn or name entry text
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, 800, 0, 800, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glDisable(GL_DEPTH_TEST);

        if (showWinMessage) {
            // Draw win message
            glColor3f(1.0f, 0.0f, 0.0f);
            int winTextWidth = glutBitmapLength(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)winMessage.c_str());
            renderBitmapString((800 - winTextWidth) / 2, 400, GLUT_BITMAP_TIMES_ROMAN_24, winMessage.c_str());
        }
        else {
            // Draw normal game text
            glColor3f(1.0f, 0.0f, 0.0f);
            if (!nameEntryDone) {
                std::string prompt = (nameEntryPhase == 1 ? "Player 1 Name: " : "Player 2 Name: ") + nameBuffer + "_";
                renderBitmapString(30, 760, GLUT_BITMAP_HELVETICA_18, prompt);
            }
            else {
                std::string symbol = playerSymbol[currentPlayer] == 1 ? "X" : "O";
                std::string name = playerName[currentPlayer];
                std::string text = symbol + "'s Turn: " + name;
                renderBitmapString(30, 760, GLUT_BITMAP_HELVETICA_18, text);
            }
        }

        glEnable(GL_DEPTH_TEST);
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();

        if (anyVibrating) {
            glutPostRedisplay();
        }

        // Draw timer in top right corner
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, 800, 0, 800, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glDisable(GL_DEPTH_TEST);

        // Draw timer background
        glColor3f(0.1f, 0.1f, 0.1f);
        glEnd();

        // Draw timer text
        glColor3f(timerColor[0], timerColor[1], timerColor[2]);
        char timerStr[32];
        sprintf_s(timerStr, "%d", int(gameTimer));

        renderBitmapString(610, 760, GLUT_BITMAP_HELVETICA_18, timerStr);

        glEnable(GL_DEPTH_TEST);
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }

    glutSwapBuffers();
}


// ---------------------------- WIN CHECK ----------------------------

void checkwin() {
    int lines[100][3][3] = { 0 };  // increased size to fit all lines
    int idx = 0;

    // rows, columns, pillars
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            // varying k in z (depth)
            for (int k = 0; k < size; k++) lines[idx][k][0] = i, lines[idx][k][1] = j, lines[idx][k][2] = k; idx++;
            // varying j in y
            for (int k = 0; k < size; k++) lines[idx][k][0] = i, lines[idx][k][1] = k, lines[idx][k][2] = j; idx++;
            // varying i in x
            for (int k = 0; k < size; k++) lines[idx][k][0] = k, lines[idx][k][1] = i, lines[idx][k][2] = j; idx++;
        }
    }

    // cube space diagonals (4 total)
    for (int i = 0; i < size; i++) lines[idx][i][0] = i, lines[idx][i][1] = i, lines[idx][i][2] = i; idx++;
    for (int i = 0; i < size; i++) lines[idx][i][0] = i, lines[idx][i][1] = i, lines[idx][i][2] = size - 1 - i; idx++;
    for (int i = 0; i < size; i++) lines[idx][i][0] = i, lines[idx][i][1] = size - 1 - i, lines[idx][i][2] = i; idx++;
    for (int i = 0; i < size; i++) lines[idx][i][0] = size - 1 - i, lines[idx][i][1] = i, lines[idx][i][2] = i; idx++;

    // face diagonals
    // xy planes (z fixed)
    for (int z = 0; z < size; z++) {
        for (int i = 0; i < size; i++) lines[idx][i][0] = i, lines[idx][i][1] = i, lines[idx][i][2] = z; idx++;
        for (int i = 0; i < size; i++) lines[idx][i][0] = i, lines[idx][i][1] = size - 1 - i, lines[idx][i][2] = z; idx++;
    }

    // yz planes (x fixed)
    for (int x = 0; x < size; x++) {
        for (int i = 0; i < size; i++) lines[idx][i][0] = x, lines[idx][i][1] = i, lines[idx][i][2] = i; idx++;
        for (int i = 0; i < size; i++) lines[idx][i][0] = x, lines[idx][i][1] = size - 1 - i, lines[idx][i][2] = i; idx++;
    }

    // xz planes (y fixed)
    for (int y = 0; y < size; y++) {
        for (int i = 0; i < size; i++) lines[idx][i][0] = i, lines[idx][i][1] = y, lines[idx][i][2] = i; idx++;
        for (int i = 0; i < size; i++) lines[idx][i][0] = size - 1 - i, lines[idx][i][1] = y, lines[idx][i][2] = i; idx++;
    }

    // check all lines
    for (int i = 0; i < idx; i++) {
        int a[3];
        for (int j = 0; j < 3; j++)
            a[j] = board[lines[i][j][0]][lines[i][j][1]][lines[i][j][2]];

        if (a[0] != 0 && a[0] == a[1] && a[1] == a[2]) {
            winner = a[0];
            for (int j = 0; j < 3; j++)
                for (int d = 0; d < 3; d++)
                    winLine[j][d] = lines[i][j][d];

            // Reset timer to original - 5 seconds
            originalTimer = max(10, originalTimer - 10);
            gameTimer = originalTimer;

            // Set win message
            showWinMessage = true;
            winMessage = playerName[winner] + " WINS!";

            // Play win sound
            playWinSound();

            glutPostRedisplay();
            glutTimerFunc(3500, resetGame, 0);
            break;
        }
    }
}


// ---------------------------- INPUT ----------------------------

void motion(int x, int y) {
    rotateY += (x - lastX) * 0.5f;
    rotateX += (y - lastY) * 0.5f;
    lastX = x;
    lastY = y;
    glutPostRedisplay();
}

// mousemove with detection of the hovered cube:
void mouseMove(int x, int y) {
    if (showOpeningScreen) {
        lastX = x;
        lastY = y;
        glutPostRedisplay();
        return;
    }

    lastX = x;
    lastY = y;

    GLint viewport[4];
    GLdouble modelview[16], projection[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    glGetIntegerv(GL_VIEWPORT, viewport);

    y = viewport[3] - y; // Flip y

    hoveredX = hoveredY = hoveredZ = -1; // Reset hover

    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            for (int k = 0; k < SIZE; k++) {
                float cx = (i - 1) * spacing;
                float cy = (j - 1) * spacing;
                float cz = (k - 1) * spacing;

                GLdouble sx, sy, sz;
                gluProject(cx, cy, cz, modelview, projection, viewport, &sx, &sy, &sz);

                float dx = fabs(sx - x);
                float dy = fabs(sy - y);

                if (dx < 40 && dy < 40) {
                    hoveredX = i;
                    hoveredY = j;
                    hoveredZ = k;
                    glutPostRedisplay();
                    return;
                }
            }
        }
    }

    glutPostRedisplay();
}
//rest game after win
void resetGame(int value) {
    for (int x = 0; x < SIZE; x++)
        for (int y = 0; y < SIZE; y++)
            for (int z = 0; z < SIZE; z++)
                board[x][y][z] = 0;

    winner = 0;
    showWinMessage = false;
    winMessage = "";
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            winLine[i][j] = -1;

    playerSymbol[1] = (rand() % 2) + 1;
    playerSymbol[2] = 3 - playerSymbol[1];
    currentPlayer = 1;

    // Reset timer to current original value
    gameTimer = originalTimer;

    printf("\nNew Game!\n");
    printf("Player 1: %s\n", playerSymbol[1] == 1 ? "X" : "O");
    printf("Player 2: %s\n", playerSymbol[2] == 1 ? "X" : "O");

    glutPostRedisplay();
}
void idle() {
    if (transitioning) {
        glutPostRedisplay();
    }
    else if (showOpeningScreen) {
        // Force redraw of opening screen for continuous flickering
        glutPostRedisplay();
    }
}


void normalKey(unsigned char key, int x, int y) {
    if (showOpeningScreen) {
        if (key == 13 || key == '\r') { // Enter key
            titleGlowing = true;

            // Stop opening screen music
            if (isBackgroundMusicPlaying) {
                PlaySound(NULL, NULL, 0); // Stop current sound
                isBackgroundMusicPlaying = false;
            }

            // Start game background music
            mciSendString(TEXT("open \"Insidious3.wav\" type mpegvideo alias bgm"), NULL, 0, NULL);
            mciSendString(TEXT("play bgm repeat"), NULL, 0, NULL);
            isGameMusicPlaying = true;

            // Play transition sound
            PlaySound(TEXT("Outlast ｜ Stinger #13 ♪ [Sound Effect].wav"), NULL, SND_ASYNC | SND_NOSTOP | SND_FILENAME);

            // Wait for glow effect to complete
            glutTimerFunc(1000, [](int) {
                showOpeningScreen = false;
                glutPostRedisplay();
                }, 0);
            return;
        }
    }

    if (!nameEntryDone) {
        if (key == 13 || key == '\r') { // Enter
            playerName[nameEntryPhase] = nameBuffer;
            nameBuffer.clear();

            if (nameEntryPhase == 1) {
                nameEntryPhase = 2;
            }
            else {
                nameEntryPhase = 0;
                nameEntryDone = true;

                // Assign random symbols
                playerSymbol[1] = (rand() % 2) + 1;
                playerSymbol[2] = 3 - playerSymbol[1];
                currentPlayer = 1;

                // Reset timer to original value
                gameTimer = originalTimer;
                timerActive = true;
                glutTimerFunc(100, updateTimer, 0);

                printf("\nPlayer 1 (%s): %s", playerName[1].c_str(), playerSymbol[1] == 1 ? "X" : "O");
                printf("\nPlayer 2 (%s): %s\n", playerName[2].c_str(), playerSymbol[2] == 1 ? "X" : "O");
            }
        }
        else if (key == 8 || key == 127) { // Backspace
            if (!nameBuffer.empty())
                nameBuffer.pop_back();
        }
        else if (isprint(key)) {
            nameBuffer += key;
        }

        glutPostRedisplay();
        return;
    }

    // Camera view keys (1–6) if not typing name
    startRotateX = rotateX;
    startRotateY = rotateY;

    switch (key) {
    case '1': targetRotateX = 0.0f; targetRotateY = 0.0f; break;
    case '2': targetRotateX = 0.0f; targetRotateY = 180.0f; break;
    case '3': targetRotateX = 0.0f; targetRotateY = -90.0f; break;
    case '4': targetRotateX = 0.0f; targetRotateY = 90.0f; break;
    case '5': targetRotateX = -90.0f; targetRotateY = 0.0f; break;
    case '6': targetRotateX = 90.0f; targetRotateY = 0.0f; break;
    }

    transitioning = true;
    transitionStart = Clock::now();
    glutIdleFunc(idle);
}



void mouse(int button, int state, int x, int y) {
    if (showOpeningScreen) {
        // Remove mouse click handling for play button
        return;
    }

    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && !winner) {
        GLint viewport[4];
        GLdouble modelview[16], projection[16];
        glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
        glGetDoublev(GL_PROJECTION_MATRIX, projection);
        glGetIntegerv(GL_VIEWPORT, viewport);

        // Flip y for OpenGL
        y = viewport[3] - y;

        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                for (int k = 0; k < SIZE; k++) {
                    float cx = (i - 1) * spacing;
                    float cy = (j - 1) * spacing;
                    float cz = (k - 1) * spacing;

                    GLdouble sx, sy, sz;
                    gluProject(cx, cy, cz, modelview, projection, viewport, &sx, &sy, &sz);

                    float dx = fabs(sx - x);
                    float dy = fabs(sy - y);

                    if (dx < 40 && dy < 40) { // 40 pixel radius
                        if (board[i][j][k] == 0) {
                            board[i][j][k] = playerSymbol[currentPlayer];
                            // Play move sound for current player
                            playMoveSound(playerSymbol[currentPlayer]);
                            currentPlayer = 3 - currentPlayer;
                            checkwin();
                            vibrating[i][j][k] = true;
                            vibrationStartTime[i][j][k] = getTimeInMillis();
                            glutPostRedisplay();
                            return;
                        }
                    }
                }
            }
        }
    }
}


// ---------------------------- INIT & MAIN ----------------------------

void init() {
    // Initialize all game state variables
    showOpeningScreen = true;
    playButtonHovered = false;
    nameEntryDone = false;
    transitioning = false;
    isDragging = false;
    dragging = false;

    currentPlayer = 1;
    winner = 0;
    nameEntryPhase = 1;
    nameBuffer.clear();
    playerName[1].clear();
    playerName[2].clear();

    // Initialize player symbols (will be set properly after name entry)
    playerSymbol[1] = 0;
    playerSymbol[2] = 0;

    // Initialize camera
    rotateX = 0.0f;
    rotateY = 0.0f;
    startRotateX = 0.0f;
    startRotateY = 0.0f;
    targetRotateX = 0.0f;
    targetRotateY = 0.0f;

    // Initialize mouse tracking
    lastX = 0;
    lastY = 0;
    hoveredX = -1;
    hoveredY = -1;
    hoveredZ = -1;

    // Initialize vibration effects
    for (int x = 0; x < SIZE; x++) {
        for (int y = 0; y < SIZE; y++) {
            for (int z = 0; z < SIZE; z++) {
                vibrating[x][y][z] = false;
                vibrationStartTime[x][y][z] = 0.0f;
            }
        }
    }

    // Initialize game board
    for (int x = 0; x < SIZE; x++) {
        for (int y = 0; y < SIZE; y++) {
            for (int z = 0; z < SIZE; z++) {
                board[x][y][z] = 0;
            }
        }
    }

    // Initialize win line
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            winLine[i][j] = -1;
        }
    }

    // Initialize OpenGL settings
    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE, GL_LINEAR);

    GLfloat fogColor[] = { 0.02f, 0.05f, 0.12f, 1.0f }; // Cooler, darker blue
    glFogfv(GL_FOG_COLOR, fogColor);

	glFogf(GL_FOG_START, 8.0f);// Set fog end to a larger value for a more subtle effect
    glFogf(GL_FOG_END, 32.0f);

    glHint(GL_FOG_HINT, GL_NICEST);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);

    glClearColor(0.02f, 0.05f, 0.12f, 1.0f); // Matching cooler, darker blue background
    srand(time(NULL));

    // Load texture
    grassTexture = SOIL_load_OGL_texture(
        "red-paint-wall-background-texture.jpg",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_TEXTURE_REPEATS
    );

    if (!grassTexture) {
        printf("Failed to load texture\n");
    }
    else {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, grassTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    // Load opening screen texture
    openingTexture = SOIL_load_OGL_texture(
        "12.jpg", // Using the same texture as grass
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_TEXTURE_REPEATS
    );

    if (!openingTexture) {
        printf("Failed to load opening screen texture: %s\n", SOIL_last_result());
    }

    // Start background music for opening screen
    PlaySound(TEXT("insidiousshine.wav"), NULL, SND_ASYNC | SND_LOOP | SND_FILENAME);
    isBackgroundMusicPlaying = true;

    // Load jumpscare texture
    jumpscareTexture = SOIL_load_OGL_texture(
        "123.jpg",  // Changed to simple filename
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_TEXTURE_REPEATS
    );

    if (!jumpscareTexture) {
        printf("Failed to load jumpscare texture: %s\n", SOIL_last_result());
        // Try loading a different texture as fallback
        jumpscareTexture = SOIL_load_OGL_texture(
            "red-paint-wall-background-texture.jpg",
            SOIL_LOAD_AUTO,
            SOIL_CREATE_NEW_ID,
            SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_TEXTURE_REPEATS
        );
        if (!jumpscareTexture) {
            printf("Failed to load fallback jumpscare texture: %s\n", SOIL_last_result());
        }
    }
}

void reshape(int w, int h) {
    if (h == 0) h = 1;
    float aspect = (float)w / h;

    // Set viewport to maintain aspect ratio
    int viewportWidth = w;
    int viewportHeight = h;
    int viewportX = 0;
    int viewportY = 0;

    // Calculate viewport to maintain 1:1 aspect ratio
    if (aspect > 1.0f) {
        // Window is wider than tall
        viewportWidth = h;
        viewportX = (w - viewportWidth) / 2;
    }
    else {
        // Window is taller than wide
        viewportHeight = w;
        viewportY = (h - viewportHeight) / 2;
    }

    glViewport(viewportX, viewportY, viewportWidth, viewportHeight);

    if (showOpeningScreen) {
        // Set up 2D orthographic projection for opening screen
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        // Use the smaller dimension to maintain square aspect ratio
        float orthoSize = (w < h) ? w : h;
        glOrtho(0, orthoSize, 0, orthoSize, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }
    else {
        // Set up 3D perspective projection for game
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(45.0, 1.0, 1.0, 100.0);  // Use 1.0 for aspect ratio to maintain square view
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 800);
    glutCreateWindow("3D Tic Tac Toe");

    // Initialize game state
    showOpeningScreen = true;
    nameEntryDone = false;
    nameEntryPhase = 1;
    nameBuffer.clear();
    playerName[1].clear();
    playerName[2].clear();

    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutPassiveMotionFunc(mouseMove);
    glutKeyboardFunc(normalKey);
    glutSpecialFunc(keyboard);
    glutIdleFunc(idle);  // Add idle function for continuous updates
    glutMainLoop();

    return 0;
}