/*********
   CTIS164 - Template Source Program
----------
STUDENT : Aizaz Hassan
SECTION :3
HOMEWORK:4
----------
PROBLEMS:
----------
ADDITIONAL FEATURES: movement of moon
					 loading screen
					 speed of arrow alterable
*********/

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>
#include "vec.h"

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

#define TIMER_PERIOD  20 // Period for the timer.
#define TIMER_ON         1 // 0:disable timer, 1:enable timer

#define D2R 0.0174532

#define LOAD 0 //game states
#define RUN 1

/* Global Variables for Template File */
bool up = false, down = false, right = false, left = false;
int  winWidth, winHeight; // current Window width and height

int gameState = LOAD;
float angle;
int vel = 3;
//
// to draw circle, center at (x,y)
// radius r
//
void circle(int x, int y, int r)
{
#define PI 3.1415
	float angle;
	glBegin(GL_POLYGON);
	for (int i = 0; i < 100; i++)
	{
		angle = 2 * PI*i / 100;
		glVertex2f(x + r * cos(angle), y + r * sin(angle));
	}
	glEnd();
}

void circle_wire(int x, int y, int r)
{
#define PI 3.1415
	float angle;

	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < 100; i++)
	{
		angle = 2 * PI*i / 100;
		glVertex2f(x + r * cos(angle), y + r * sin(angle));
	}
	glEnd();
}

void print(int x, int y, const char *string, void *font)
{
	int len, i;

	glRasterPos2f(x, y);
	len = (int)strlen(string);
	for (i = 0; i < len; i++)
	{
		glutBitmapCharacter(font, string[i]);
	}
}
// display text with variables.
// vprint(-winWidth / 2 + 10, winHeight / 2 - 20, GLUT_BITMAP_8_BY_13, "ERROR: %d", numClicks);
void vprint(int x, int y, void *font, const char *string, ...)
{
	va_list ap;
	va_start(ap, string);
	char str[1024];
	vsprintf_s(str, string, ap);
	va_end(ap);

	int len, i;
	glRasterPos2f(x, y);
	len = (int)strlen(str);
	for (i = 0; i < len; i++)
	{
		glutBitmapCharacter(font, str[i]);
	}
}

void vprint2(int x, int y, float size, const char *string, ...) {
	va_list ap;
	va_start(ap, string);
	char str[1024];
	vsprintf_s(str, string, ap);
	va_end(ap);
	glPushMatrix();
	glTranslatef(x, y, 0);
	glScalef(size, size, 1);

	int len, i;
	len = (int)strlen(str);
	for (i = 0; i < len; i++)
	{
		glutStrokeCharacter(GLUT_STROKE_ROMAN, str[i]);
	}
	glPopMatrix();
}

typedef struct {
	int r, g, b;
}color_t; //color structure

typedef struct {
	vec_t coordinate; //position
	vec_t vel; //velocity vector
	color_t color; //color of moon
}object_t; //structure for moon

typedef struct {
	vec_t coordinate; //position
	vec_t velocity; //velocity vector
	bool visible; //to check whether fire is visible
	float angle; //angle of arrow
}fire_t;

typedef struct {
	vec_t pos;
	vec_t N;
} vertex_t;

color_t mulColor(float k, color_t c) {
	color_t tmp = { k * c.r, k * c.g, k * c.b };
	return tmp;
}

double distanceImpact(double d) {
	return (-1.0 / 350.0) * d + 1.0;
} //to change impact of light with distance

color_t calculateColor(object_t source, vertex_t v) {
	vec_t L = subV(source.coordinate, v.pos);
	vec_t uL = unitV(L);
	float factor = dotP(uL, v.N) * distanceImpact(magV(L));
	return mulColor(factor, source.color);
} //calculates color

vec_t mouse; //position of mouse
color_t colBox = { 0, 0.18 * 256, 0.39 * 256 }; //color of box with "START written"

object_t obj = { {-360,200}, {2,0}, {250, 235, 215} }; //initial values of moon
fire_t fire = { {0,0},{0,0}, false, 0 }; //initial values of arrow
void clouds()
{
	glColor3ub(8, 56, 246);
	int x = 0;
	for (int k = 0; k < 5; k++) {
		circle(-300 + x, 200, 30);
		circle(-275 + x, 200, 30);
		circle(-240 + x, 200, 30);
		x += 150;
	}
} //draws clouds
void load()
{
	vprint2(-150, 150, 0.35, "GLUT HW4");
	vprint2(-200, 100, 0.3, "BY AIZAZ HASSAN");
	glColor3ub(colBox.r, colBox.g, colBox.b);
	glRectf(-110, 25, 100, -10);
	glColor3f(0, 0, 0);
	vprint(-100, 0, GLUT_BITMAP_TIMES_ROMAN_24, "START");
	vprint(-100, -200, GLUT_BITMAP_HELVETICA_12, "Click START to begin game");
} //display loading screen
void drawObject(object_t obj)
{
	glColor3ub(obj.color.r, obj.color.g, obj.color.b);
	circle(obj.coordinate.x, obj.coordinate.y, 40);
} //draws moon
void drawFire(fire_t fire)
{
	if (!fire.visible)
		return;
	glColor3f(1, 1, 0);
	vec_t v1 = subV(fire.coordinate, pol2rec({ 50,fire.angle }));
	vec_t v2 = subV(v1, pol2rec({ -5,fire.angle + 90 }));
	vec_t v3 = subV(v2, pol2rec({ -50,fire.angle }));
	vec_t v4 = addV(fire.coordinate, pol2rec({ 10,fire.angle + 15 }));
	/*vec_t v4 = addV(fire.coordinate, pol2rec({ 50,fire.angle+10 }));
	vec_t v5 = addV(v1, pol2rec({ -10,fire.angle + 90 }));*/
	glColor3f(1, 0, 0);
	glBegin(GL_POLYGON);
	glVertex2f(fire.coordinate.x, fire.coordinate.y);
	glVertex2f(v1.x, v1.y);
	glVertex2f(v2.x, v2.y);
	glVertex2f(v3.x, v3.y);
	glVertex2f(fire.coordinate.x, fire.coordinate.y);
	glEnd();
	glColor3f(1, 1, 0);
	glBegin(GL_TRIANGLES);
	glVertex2f(fire.coordinate.x, fire.coordinate.y);
	glVertex2f(v3.x, v3.y);
	glVertex2f(v4.x, v4.y);
	glEnd();
}//draws arrow

//
// To display onto window using OpenGL commands
//
void display() {
	//
	// clear window to black
	//
	glClearColor(0, 0.18, 0.39, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	switch (gameState)
	{
	case LOAD:load();
		break;
	case RUN:clouds();
		drawObject(obj);
		drawFire(fire);
		glColor3f(1, 1, 1);

		double y = -250;
		color_t res = { 0, 0, 0 };
		for (int x = -400; x <= 400; x++) {
			vertex_t P = { { x, -100 }, { 0, 1 } };

			color_t res = { 0, 0, 0 };
			res = calculateColor(obj, P);

			glBegin(GL_LINES);
			glColor3f(res.r, res.g, res.b);
			glVertex2f(x, y);

			glColor3f(0, 0, 0);
			glVertex2f(x, -300);
			glEnd();
			if (y < -100)
				y += 0.18;
		}
		glColor3f(1, 1, 0);
		vprint(-380, -100, GLUT_BITMAP_8_BY_13, "up arrow = move moon up");
		vprint(-380, -120, GLUT_BITMAP_8_BY_13, "down arrow = move moon down");
		vprint(-380, -140, GLUT_BITMAP_8_BY_13, "left mouse click = display arrow");
		vprint(-380, -160, GLUT_BITMAP_8_BY_13, "a = slow down fire");
		vprint(-380, -180, GLUT_BITMAP_8_BY_13, "d = speed up fire");
		vprint(-380, -200, GLUT_BITMAP_8_BY_13, "speed of arrow: %d", vel);
	}

	glutSwapBuffers();
}

//
// key function for ASCII charachters like ESC, a,b,c..,A,B,..Z
//
void onKeyDown(unsigned char key, int x, int y)
{
	// exit when ESC is pressed.
	if (key == 27)
		exit(0);
	if (gameState == RUN) {
		if (key == 'd'&&vel <= 5) //increases velocity of arrow
			vel++;
		if (key == 'a'&&vel > 1) //decreases velocity of arrow
			vel--;
	}

	// to refresh the window it calls display() function
	glutPostRedisplay();
}

void onKeyUp(unsigned char key, int x, int y)
{
	// exit when ESC is pressed.
	if (key == 27)
		exit(0);

	// to refresh the window it calls display() function
	glutPostRedisplay();
}

//
// Special Key like GLUT_KEY_F1, F2, F3,...
// Arrow Keys, GLUT_KEY_UP, GLUT_KEY_DOWN, GLUT_KEY_RIGHT, GLUT_KEY_RIGHT
//
void onSpecialKeyDown(int key, int x, int y)
{
	// Write your codes here.
	switch (key) {
	case GLUT_KEY_UP: up = true; break;
	case GLUT_KEY_DOWN: down = true; break;
	case GLUT_KEY_LEFT: left = true; break;
	case GLUT_KEY_RIGHT: right = true; break;
	}

	// to refresh the window it calls display() function
	glutPostRedisplay();
}

//
// Special Key like GLUT_KEY_F1, F2, F3,...
// Arrow Keys, GLUT_KEY_UP, GLUT_KEY_DOWN, GLUT_KEY_RIGHT, GLUT_KEY_RIGHT
//
void onSpecialKeyUp(int key, int x, int y)
{
	// Write your codes here.
	switch (key) {
	case GLUT_KEY_UP: up = false; break;
	case GLUT_KEY_DOWN: down = false; break;
	case GLUT_KEY_LEFT: left = false; break;
	case GLUT_KEY_RIGHT: right = false; break;
	}

	// to refresh the window it calls display() function
	glutPostRedisplay();
}

//
// When a click occurs in the window,
// It provides which button
// buttons : GLUT_LEFT_BUTTON , GLUT_RIGHT_BUTTON
// states  : GLUT_UP , GLUT_DOWN
// x, y is the coordinate of the point that mouse clicked.
//
void onClick(int button, int stat, int x, int y)
{
	// Write your codes here.

	mouse.x = x - winWidth / 2;
	mouse.y = winHeight / 2 - y;

	vec_t temp;

	if (gameState == RUN)
		if (button == GLUT_LEFT_BUTTON && stat == GLUT_DOWN && !fire.visible) //to display arrow pointing towards moon
		{
			fire.visible = true;
			fire.coordinate = mouse;
			temp.x = obj.coordinate.x - fire.coordinate.x;
			temp.y = obj.coordinate.y - fire.coordinate.y;
			angle = atan2(temp.y, temp.x) / D2R;
			fire.angle = angle;
		}

	if (gameState == LOAD)
		if (button == GLUT_LEFT_BUTTON && stat == GLUT_DOWN && mouse.x >= -110 && mouse.x <= 100 && mouse.y<25 && mouse.y>-10)
			gameState = RUN; //to move from loading screen to running game
	// to refresh the window it calls display() function
	glutPostRedisplay();
}

//
// This function is called when the window size changes.
// w : is the new width of the window in pixels.
// h : is the new height of the window in pixels.
//
void onResize(int w, int h)
{
	winWidth = w;
	winHeight = h;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-w / 2, w / 2, -h / 2, h / 2, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	display(); // refresh window.
}

void onMoveDown(int x, int y) {
	// Write your codes here.



	// to refresh the window it calls display() function   
	glutPostRedisplay();
}

// GLUT to OpenGL coordinate conversion:
//   x2 = x1 - winWidth / 2
//   y2 = winHeight / 2 - y1
void onMove(int x, int y) {
	// Write your codes here.
	mouse.x = x - winWidth / 2;
	mouse.y = winHeight / 2 - y;
	if (gameState == LOAD)
		if (mouse.x >= -110 && mouse.x <= 100 && mouse.y<25 && mouse.y>-10) //to change color of rectangle
		{
			colBox.r = 255;
			colBox.g = 0;
			colBox.b = 0;
		}
		else {
			colBox.r = 0;
			colBox.g = 0.18 * 256;
			colBox.b = 0.39 * 256;
		}

	// to refresh the window it calls display() function
	glutPostRedisplay();
}

#if TIMER_ON == 1
void onTimer(int v) {

	glutTimerFunc(TIMER_PERIOD, onTimer, 0);
	// Write your codes here.
	if (gameState == RUN) {
		obj.coordinate = addV(obj.coordinate, obj.vel); //moves moon
		if (obj.coordinate.x > 360 || obj.coordinate.x < -360) //moon hits wall and moves in opposite direction
			obj.vel.x *= -1;

		vec_t temp;
		temp.x = obj.coordinate.x - fire.coordinate.x;
		temp.y = obj.coordinate.y - fire.coordinate.y;
		angle = atan2(temp.y, temp.x) / D2R;
		fire.angle = angle; //to align arrow towards moon

		fire.velocity = mulV(vel, unitV(subV(obj.coordinate, fire.coordinate))); //velocity of arrow
		if (fire.visible)  //to move arrow toward moon
			fire.coordinate = addV(fire.coordinate, fire.velocity);

		vec_t check; //to check if arrow hits moon
		check.x = fire.coordinate.x - obj.coordinate.x;
		check.y = fire.coordinate.y - obj.coordinate.y;
		float d = sqrtf(check.x*check.x + check.y*check.y);
		if (d <= 40)
			fire.visible = false;

		if (down&&obj.coordinate.y > -50) //to move moon down
			obj.coordinate.y--;
		if (up&&obj.coordinate.y <= 260) //to move moon up
			obj.coordinate.y++;
	}

	// to refresh the window it calls display() function
	glutPostRedisplay(); // display()
}
#endif

void Init() {

	// Smoothing shapes
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}

void main(int argc, char *argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	//glutInitWindowPosition(100, 100);
	glutCreateWindow("GLUT HW4 by Aizaz Hassan");

	glutDisplayFunc(display);
	glutReshapeFunc(onResize);

	//
	// keyboard registration
	//
	glutKeyboardFunc(onKeyDown);
	glutSpecialFunc(onSpecialKeyDown);

	glutKeyboardUpFunc(onKeyUp);
	glutSpecialUpFunc(onSpecialKeyUp);

	//
	// mouse registration
	//
	glutMouseFunc(onClick);
	glutMotionFunc(onMoveDown);
	glutPassiveMotionFunc(onMove);

#if  TIMER_ON == 1
	// timer event
	glutTimerFunc(TIMER_PERIOD, onTimer, 0);
#endif

	Init();

	glutMainLoop();
}