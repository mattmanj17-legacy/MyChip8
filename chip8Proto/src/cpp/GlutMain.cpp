#include <stdio.h>
#include <glut.h>
#include <thread>
#include <iostream>
#include "VCPU.h"

#define CYCLE_DELAY 3
#define TIMER_DELAY 16

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define PIXLE_SIZE 10

void runCycle(int value);
void updateTimers(int value);
void display();
void setupGLUT(int argc, char **argv);
void setGLUTCallbacks();
void setupTexture();
void updateTexture(VCPU* c8);
void reshape_window(GLsizei w, GLsizei h);
void keyboardUp(unsigned char key, int x, int y);
void keyboardDown(unsigned char key, int x, int y);

int display_width  = SCREEN_WIDTH * PIXLE_SIZE;
int display_height = SCREEN_HEIGHT * PIXLE_SIZE;

byte screenData[SCREEN_HEIGHT][SCREEN_WIDTH][3]; 
VCPU* myChip8;

// change this char* to load a new ROM
const char* romName= "BRIX.c8";

int main(int argc, char **argv)
{
	ROM myrom(romName);
	myChip8 = new VCPU(16,myrom);

	myChip8->setPlaySoundCallback([]()
	{
		PlaySound(TEXT("Blip_Select219.wav"), NULL, SND_FILENAME | SND_ASYNC);
	});

	setupGLUT(argc, argv);
	setGLUTCallbacks();
	setupTexture();

	glutTimerFunc(CYCLE_DELAY,runCycle,0);
	glutTimerFunc(TIMER_DELAY,updateTimers,0);

	glutMainLoop(); 
	return 0;
}

void runCycle(int value)
{
	myChip8->runCycle();
	glutTimerFunc(CYCLE_DELAY,runCycle,0);
}

void updateTimers(int value)
{
	myChip8->updateCounters();
	glutTimerFunc(TIMER_DELAY,updateTimers,0);
}

void display()
{
	if(myChip8->getDrawFlag())
	{
		updateTexture(myChip8);
		myChip8->setDrawFlag(false);
	}
}

void setupGLUT(int argc, char **argv)
{
	glutInit(&argc, argv);          
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(display_width, display_height);
    glutInitWindowPosition(320, 320);
	glutCreateWindow("CS278 Chip8 Emulator");
}

void setGLUTCallbacks()
{
	glutDisplayFunc(display);
	glutIdleFunc(display);
    glutReshapeFunc(reshape_window);        
	glutKeyboardFunc(keyboardDown);
	glutKeyboardUpFunc(keyboardUp);
}

void setupTexture()
{
	for(int y = 0; y < SCREEN_HEIGHT; ++y)		
		for(int x = 0; x < SCREEN_WIDTH; ++x)
			screenData[y][x][0] = screenData[y][x][1] = screenData[y][x][2] = 0;

	glTexImage2D(GL_TEXTURE_2D, 0, 3, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)screenData);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP); 
	
	glEnable(GL_TEXTURE_2D);
}

void updateTexture(VCPU* c8)
{	
	glClear(GL_COLOR_BUFFER_BIT);
	
	for(int y = 0; y < 32; ++y)		
		for(int x = 0; x < 64; ++x)
			if(c8->getScreen(x,y) == 0)
				screenData[y][x][0] = screenData[y][x][1] = screenData[y][x][2] = 0;	// Disabled
			else 
				screenData[y][x][0] = screenData[y][x][1] = screenData[y][x][2] = 255;  // Enabled
	
	glTexSubImage2D(GL_TEXTURE_2D, 0 ,0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)screenData);
	glBegin( GL_QUADS );
		glTexCoord2d(0.0, 0.0);		glVertex2d(0.0,			  0.0);
		glTexCoord2d(1.0, 0.0); 	glVertex2d(display_width, 0.0);
		glTexCoord2d(1.0, 1.0); 	glVertex2d(display_width, display_height);
		glTexCoord2d(0.0, 1.0); 	glVertex2d(0.0,			  display_height);
	glEnd();

	glutSwapBuffers();
}

void reshape_window(GLsizei w, GLsizei h)
{
	glClearColor(0.0f, 0.0f, 0.5f, 0.0f);
	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, w, h, 0);        
    glMatrixMode(GL_MODELVIEW);
    glViewport(0, 0, w, h);
	
	display_width = w;
	display_height = h;
}

void keyboardDown(unsigned char key, int x, int y)
{
	if(key == 27)    // esc
		exit(0);

	if(key == '1')		myChip8->keyStates[0x1] = true;
	else if(key == '2')	myChip8->keyStates[0x2] = true;
	else if(key == '3')	myChip8->keyStates[0x3] = true;
	else if(key == '4')	myChip8->keyStates[0xC] = true;

	else if(key == 'q')	myChip8->keyStates[0x4] = true;
	else if(key == 'w')	myChip8->keyStates[0x5] = true;
	else if(key == 'e')	myChip8->keyStates[0x6] = true;
	else if(key == 'r')	myChip8->keyStates[0xD] = true;

	else if(key == 'a')	myChip8->keyStates[0x7] = true;
	else if(key == 's')	myChip8->keyStates[0x8] = true;
	else if(key == 'd')	myChip8->keyStates[0x9] = true;
	else if(key == 'f')	myChip8->keyStates[0xE] = true;

	else if(key == 'z')	myChip8->keyStates[0xA] = true;
	else if(key == 'x')	myChip8->keyStates[0x0] = true;
	else if(key == 'c')	myChip8->keyStates[0xB] = true;
	else if(key == 'v')	myChip8->keyStates[0xF] = true;
}

void keyboardUp(unsigned char key, int x, int y)
{
	if(key == '1')		myChip8->keyStates[0x1] = false;
	else if(key == '2')	myChip8->keyStates[0x2] = false;
	else if(key == '3')	myChip8->keyStates[0x3] = false;
	else if(key == '4')	myChip8->keyStates[0xC] = false;

	else if(key == 'q')	myChip8->keyStates[0x4] = false;
	else if(key == 'w')	myChip8->keyStates[0x5] = false;
	else if(key == 'e')	myChip8->keyStates[0x6] = false;
	else if(key == 'r')	myChip8->keyStates[0xD] = false;

	else if(key == 'a')	myChip8->keyStates[0x7] = false;
	else if(key == 's')	myChip8->keyStates[0x8] = false;
	else if(key == 'd')	myChip8->keyStates[0x9] = false;
	else if(key == 'f')	myChip8->keyStates[0xE] = false;

	else if(key == 'z')	myChip8->keyStates[0xA] = false;
	else if(key == 'x')	myChip8->keyStates[0x0] = false;
	else if(key == 'c')	myChip8->keyStates[0xB] = false;
	else if(key == 'v')	myChip8->keyStates[0xF] = false;
}
