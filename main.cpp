#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <string>
#include <cmath>
#include <windows.h>
#include <mmsystem.h>

#pragma comment(lib, "winmm.lib")

#define PI 3.14159265358979323846


const int winW = 900, winH = 650;


enum GameState { MENU, HELP, HISCORE, PLAYING, PAUSED, GAMEOVER } gameState = MENU;


struct Basket {
    int x, y, w, h;
    bool enlarged = false;
    int enlargeTimer = 0;
} basket = {winW/2, 50, 110, 35};

int airflow = 0;


const int NUM_CHICKENS = 3;
int chickenXs[NUM_CHICKENS] = {200, 450, 700};
int stickY = winH - 90;


enum EggType { GOLDEN, BLUE, NORMAL, POOP, PERK_LARGE, PERK_SLOW, PERK_TIME, BOMB, LIFE };
struct Egg {
    int x, y, r;
    EggType type;
    bool active;
    int vx, vy;
};
std::vector<Egg> eggs;


int score = 0, highScore = 0;
float gameTime = 40.0f;
float elapsedTime = 0.0f;
int slowFallTimer = 0;


int lives = 3;
const int MAX_LIVES = 3;

// 100. sound functions 
void playSound(const char* soundFile) {
    std::string fullPath = "sounds/" + std::string(soundFile);
    if (!PlaySoundA(fullPath.c_str(), NULL, SND_FILENAME | SND_ASYNC)) {
        
        PlaySoundA(soundFile, NULL, SND_FILENAME | SND_ASYNC);
    }
}

void playScoreSound() { playSound("score.wav"); }
void playBombSound() { playSound("bomb.wav"); }
void playLifeSound() { playSound("life.wav"); }
void playPerkSound() { playSound("perk.wav"); }
void playPoopSound() { playSound("poop.wav"); }


// 57. Drawing Utilities
void drawCircle(int x, int y, int r, float cr, float cg, float cb) {
    glColor3f(cr, cg, cb);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2i(x, y);
    for (int i = 0; i <= 360; i += 10) {
        float rad = i * 3.14159 / 180;
        glVertex2i(x + (int)(cos(rad) * r), y + (int)(sin(rad) * r));
    }
    glEnd();
}

void drawEllipse(int x, int y, int rx, int ry, float cr, float cg, float cb) {
    glColor3f(cr, cg, cb);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2i(x, y);
    for (int i = 0; i < 360; i += 10) {
        float rad = i * 3.14159 / 180;
        glVertex2i(x + (int)(rx*cos(rad)), y + (int)(ry*sin(rad)));
    }
    glEnd();
}

void drawPolygon(const int* px, const int* py, int n, float r, float g, float b) {
    glColor3f(r, g, b);
    glBegin(GL_POLYGON);
    for(int i=0; i<n; i++) glVertex2i(px[i], py[i]);
    glEnd();
}

void drawStick(int x, int y, int w, int h) {
    glColor3f(0.7, 0.45, 0.18);
    glBegin(GL_QUADS);
    glVertex2i(x-w/2, y-h/2);
    glVertex2i(x+w/2, y-h/2);
    glVertex2i(x+w/2, y+h/2);
    glVertex2i(x-w/2, y+h/2);
    glEnd();
}

// 100. Chicken Drawing 
void drawChicken(int x, int y) {
    // Body
    drawEllipse(x, y-5, 25, 35, 1, 0.95, 0.8);

    // Head
    drawCircle(x, y+25, 15, 1, 0.95, 0.8);

    // Beak
    glColor3f(1, 0.7, 0.2);
    glBegin(GL_TRIANGLES);
    glVertex2i(x, y+25);
    glVertex2i(x+8, y+20);
    glVertex2i(x+3, y+15);
    glEnd();

    // Comb
    glColor3f(0.9, 0.2, 0.2);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2i(x, y+40);
    glVertex2i(x-4, y+38);
    glVertex2i(x-2, y+36);
    glVertex2i(x, y+35);
    glVertex2i(x+2, y+36);
    glVertex2i(x+4, y+38);
    glEnd();

    // Jhuti
    drawEllipse(x-2, y+18, 3, 2, 0.9, 0.2, 0.2);

    // Eyes
    drawCircle(x-4, y+28, 2, 0, 0, 0);
    drawCircle(x+4, y+28, 2, 0, 0, 0);

    // Wings
    drawEllipse(x-18, y-8, 12, 8, 0.98, 0.97, 0.75);
    drawEllipse(x+18, y-8, 12, 8, 0.98, 0.97, 0.75);

    // Tail feathers
    glColor3f(0.8, 0.8, 0.9);
    glBegin(GL_TRIANGLES);
    glVertex2i(x-25, y-15);
    glVertex2i(x-15, y-10);
    glVertex2i(x-20, y-5);
    glEnd();

    glColor3f(0.7, 0.7, 0.8);
    glBegin(GL_TRIANGLES);
    glVertex2i(x+25, y-15);
    glVertex2i(x+15, y-10);
    glVertex2i(x+20, y-5);
    glEnd();

    // Legs
    glColor3f(1, 0.8, 0.2);
    glLineWidth(3);
    glBegin(GL_LINES);
    // Left leg
    //glVertex2i(x-8, y-35);
    //glVertex2i(x-8, y-50);
    //glVertex2i(x-8, y-50);
    //glVertex2i(x-12, y-55);
    // Right leg
    //glVertex2i(x+8, y-35);
    //glVertex2i(x+8, y-50);
    //glVertex2i(x+8, y-50);
    //glVertex2i(x+12, y-55);
    //glEnd();
    glLineWidth(1);
}


// 57. Basket
void drawBasket(float Bx, float By, float Bradius, bool enlarged) {
    int Amount = 100;
    float r = enlarged ? Bradius * 1.3f : Bradius;

    // basket body
    glColor3f(0.64, 0.48, 0.20);
    glBegin(GL_TRIANGLE_FAN);
        glVertex2f(Bx, By);
        for(int i = 0; i <= Amount;i++) {
            glVertex2f( Bx + (r * cos(-i * PI / Amount)),
                       By + (r * sin(-i * PI / Amount)) );
        }
    glEnd();

    // Basket er uporer border
    glColor3f(0.5, 0.18, 0.04);
    glLineWidth(5.0);
    glBegin(GL_LINE_LOOP);
        for(int i = 0; i <= Amount;i++) {
            glVertex2f(
                Bx + (r * cos(-i *  PI / Amount)),
                By + (r * sin(-i * PI /Amount)) );
        }
    glEnd();

    // Basket band
    glColor3f(0.5, 0.18, 0.04);
    glBegin(GL_QUADS);
    glVertex2f(Bx + (r+3), By );
    glVertex2f(Bx + (r+3), By + 40 );
    glVertex2f(Bx - (r+3), By + 40 );
    glVertex2f(Bx - (r+3), By );
    glEnd();

    // Basket vertical lines
    glColor3f(0.65, 0.4, 0.0);
    glBegin(GL_LINES);
    for(float i = 0; i <= r; i=i+15)
    {
        glVertex2f(Bx + i, By );
        glVertex2f(Bx + i, By + 40 );
        glVertex2f(Bx - i, By + 40 );
        glVertex2f(Bx - i, By );
    }
    glEnd();
}

// 100. Bomb 
void drawBomb(int x, int y, int r) {
    drawCircle(x, y, r, 0.05,0.05,0.05);
    glColor3f(0.8,0.8,0.8);
    glBegin(GL_LINES);
    glVertex2i(x, y+r); glVertex2i(x, y+r+13);
    glEnd();
    drawCircle(x, y+r+16, 4, 1,0.3,0.1);
}


// 57. Environment
void drawTree(int x, int y) {
    glColor3f(0.5, 0.32, 0.18);
    glBegin(GL_QUADS);
    glVertex2i(x-12, y);
    glVertex2i(x+12, y);
    glVertex2i(x+18, y+120);
    glVertex2i(x-18, y+120);
    glEnd();
    glColor3f(0.55,0.35,0.2);
    glBegin(GL_LINES);
    glVertex2i(x, y+60); glVertex2i(x-40, y+110);
    glVertex2i(x, y+80); glVertex2i(x+60, y+150);
    glEnd();
    drawEllipse(x, y+120, 46, 40, 0.13,0.7,0.23);
    drawEllipse(x+25, y+150, 32, 24, 0.2,0.8,0.27);
    drawEllipse(x-30, y+146, 27, 20, 0.2,0.7,0.3);
    drawEllipse(x+5, y+175, 22, 13, 0.1,0.7,0.25);
}

void drawGrass() {
    glColor3f(0.2,0.8,0.22);
    glBegin(GL_POLYGON);
    glVertex2i(0,0);
    glVertex2i(winW,0);
    glVertex2i(winW,70);
    glVertex2i(0,70);
    glEnd();
    for(int x=15; x<winW; x+=25) {
        glColor3f(0.1+0.1*(x%2),0.7+0.1*(x%3),0.1+0.1*(x%4));
        glBegin(GL_TRIANGLES);
        glVertex2i(x,68);
        glVertex2i(x+3,80+rand()%12);
        glVertex2i(x+6,68);
        glEnd();
    }
}

void drawCloud(int x, int y, int size) {
    drawCircle(x, y, size, 1,1,1);
    drawCircle(x+size/2, y+size/3, size*2/3, 1,1,1);
    drawCircle(x-size/2, y+size/4, size*2/3, 1,1,1);
    drawCircle(x, y-size/3, size/2, 1,1,1);
}

void drawClouds() {
    drawCloud(170, winH-50, 35);
    drawCloud(410, winH-35, 43);
    drawCloud(720, winH-60, 28);
    drawCloud(600, winH-80, 20);
}

void drawSky() {
    glBegin(GL_QUADS);
    glColor3f(0.7,0.95,0.99);
    glVertex2i(0,winH); glVertex2i(winW,winH);
    glColor3f(0.95,0.95,1.0);
    glVertex2i(winW,0); glVertex2i(0,0);
    glEnd();
}

// 57. Game Logic

void drawEgg(Egg& e) {
    if (!e.active) return;
    if (e.type == GOLDEN)
        drawEllipse(e.x, e.y, e.r, e.r+4, 1, 0.8, 0.1);
    else if (e.type == BLUE)
        drawEllipse(e.x, e.y, e.r, e.r+4, 0.2,0.5,1);
    else if (e.type == NORMAL)
        drawEllipse(e.x, e.y, e.r, e.r+4, 1,1,0.95);
    else if (e.type == POOP)
        drawEllipse(e.x, e.y, e.r, e.r, 0.4,0.2,0.1);
    else if (e.type == PERK_LARGE)
        drawEllipse(e.x, e.y, e.r, e.r+2, 0.7,1,0.3);
    else if (e.type == PERK_SLOW)
        drawEllipse(e.x, e.y, e.r, e.r+2, 0.7,0.7,1);
    else if (e.type == PERK_TIME)
        drawEllipse(e.x, e.y, e.r, e.r+2, 1,0.5,0.8);
    else if (e.type == BOMB)
        drawBomb(e.x, e.y, e.r+2);
    else if (e.type == LIFE)
        drawHeart(e.x, e.y, e.r+2);
}

void drawText(float x, float y, const char* s, float r=0, float g=0, float b=0) {
    glColor3f(r,g,b);
    glRasterPos2f(x, y);
    for (int i = 0; s[i]; i++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, s[i]);
}

void spawnEgg() {
    int idx = rand() % NUM_CHICKENS;
    Egg e;
    e.x = chickenXs[idx] + rand()%26 - 13;
    e.y = stickY-10;
    e.r = 15 + rand()%3;
    int n = rand()%25;
    if(n<=2) e.type = GOLDEN;
    else if(n<=6) e.type = BLUE;
    else if(n<=15) e.type = NORMAL;
    else if(n==16) e.type = POOP;
    else if(n==17) e.type = PERK_LARGE;
    else if(n==18) e.type = PERK_SLOW;
    else if(n==19) e.type = PERK_TIME;
    else if(n==20) e.type = BOMB;
    else if(n==21) e.type = LIFE;
    else e.type = NORMAL;
    e.active = true;
    e.vx = airflow + (rand()%3-1);
    e.vy = (slowFallTimer>0)?2:6;
    eggs.push_back(e);
}

void update(int value) {
    if (gameState != PLAYING) return;
    if (rand()%18==0) spawnEgg();
    if (rand()%120==0) airflow = (rand()%7)-3;
    for (Egg& e : eggs) {
        if (!e.active) continue;
        e.x += e.vx;
        e.y -= e.vy;
        int basketWcur = basket.enlarged ? basket.w+40 : basket.w;
        if (e.y-e.r < basket.y+basket.h/2+10 &&
            e.x > basket.x-basketWcur/2 && e.x < basket.x+basketWcur/2) {
            e.active = false;


            if (e.type == GOLDEN || e.type == BLUE || e.type == NORMAL) {
                playScoreSound();
                if (e.type == GOLDEN) score += 10;
                else if (e.type == BLUE) score += 5;
                else if (e.type == NORMAL) score += 1;
            }
            else if (e.type == POOP) {
                playPoopSound();
                if (score > 0) score -= 10;
                if (score < 0) score = 0;
            }
            else if (e.type == PERK_LARGE || e.type == PERK_SLOW || e.type == PERK_TIME) {
                playPerkSound();
                if (e.type == PERK_LARGE) { basket.enlarged=true; basket.enlargeTimer=300;}
                else if (e.type == PERK_SLOW) { slowFallTimer=300;}
                else if (e.type == PERK_TIME) { gameTime+=8;}
            }
            else if (e.type == BOMB) {
                playBombSound();
                lives--;
                if (lives <= 0) {
                    lives = 0;
                    gameState = GAMEOVER;
                }
            }
            else if (e.type == LIFE) {
                playLifeSound();
                if (lives < MAX_LIVES) lives++;
            }
        }
        if (e.y+e.r<0 || e.x<0 || e.x>winW) e.active=false;
    }
    if(basket.enlarged && --basket.enlargeTimer<=0) basket.enlarged=false;
    if(slowFallTimer>0) slowFallTimer--;
    elapsedTime += 0.016f;
    if (elapsedTime >= 1.0f) {
        gameTime -= 1.0f;
        elapsedTime = 0.0f;
    }
    if (gameTime <= 0) {
        if(score>highScore) highScore=score;
        gameState = GAMEOVER;
    }
    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}





void myInit() {
    glClearColor(0.95f, 0.95f, 1.0f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, winW, 0, winH);
}

int main(int argc, char** argv) {
    srand((unsigned int)time(0));
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(winW, winH);
    glutInitWindowPosition(200, 60);
    glutCreateWindow("Catch The Egg Game - Life & Bombs with Sound");
    myInit();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMainLoop();
    return 0;
}
