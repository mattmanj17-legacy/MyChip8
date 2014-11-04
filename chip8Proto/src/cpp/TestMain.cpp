#include <stdio.h>
#include <glut.h>
#include "VCPU.h"
#include <thread>

const char* romName= "BRIX.c8";

// Display size
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32

VCPU* myChip8;
int modifier = 10;

// Window size
int display_width = SCREEN_WIDTH * modifier;
int display_height = SCREEN_HEIGHT * modifier;
void display();
void reshape_window(GLsizei w, GLsizei h);
void keyboardUp(unsigned char key, int x, int y);
void keyboardDown(unsigned char key, int x, int y);

// Use new drawing method
#define DRAWWITHTEXTURE
typedef unsigned __int8 u8;
u8 screenData[SCREEN_HEIGHT][SCREEN_WIDTH][3]; 
void setupTexture();
const int delayMil = 3;
const int timerDelay = 16;
bool draw=false;
bool cycleCap = true;

void delay(int value){
	draw=true;
	glutTimerFunc(delayMil,delay,0);
}

void pingTimers(int value){
	bool sound = myChip8->updateCounters();
	if(sound){
		PlaySound(TEXT("Blip_Select219.wav"), NULL, SND_FILENAME | SND_ASYNC);
		std::cout<<"beep"<<std::endl;
	}
	glutTimerFunc(timerDelay,pingTimers,0);
}

int main(int argc, char **argv){
	ROM myrom(romName);
	myChip8 = new VCPU(16,myrom);	
	// Setup OpenGL
	glutInit(&argc, argv);          
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(display_width, display_height);
    glutInitWindowPosition(320, 320);
	glutCreateWindow("CS278 Chip8 Emulator");
	glutDisplayFunc(display);
	glutIdleFunc(display);
    glutReshapeFunc(reshape_window);        
	glutKeyboardFunc(keyboardDown);
	glutKeyboardUpFunc(keyboardUp);
	glutTimerFunc(delayMil,delay,0);
	glutTimerFunc(timerDelay,pingTimers,0);
#ifdef DRAWWITHTEXTURE
	setupTexture();			
#endif	
	glutMainLoop(); 
	return 0;
}

// Setup Texture
void setupTexture(){
	// Clear screen
	for(int y = 0; y < SCREEN_HEIGHT; ++y)		
		for(int x = 0; x < SCREEN_WIDTH; ++x)
			screenData[y][x][0] = screenData[y][x][1] = screenData[y][x][2] = 0;
	// Create a texture 
	glTexImage2D(GL_TEXTURE_2D, 0, 3, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)screenData);
	// Set up the texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP); 
	// Enable textures
	glEnable(GL_TEXTURE_2D);
}
void updateTexture(VCPU* c8){	
	// Update pixels
	for(int y = 0; y < 32; ++y)		
		for(int x = 0; x < 64; ++x)
			if(c8->getScreen(x,y) == 0)
				screenData[y][x][0] = screenData[y][x][1] = screenData[y][x][2] = 0;	// Disabled
			else 
				screenData[y][x][0] = screenData[y][x][1] = screenData[y][x][2] = 255;  // Enabled
	// Update Texture
	glTexSubImage2D(GL_TEXTURE_2D, 0 ,0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)screenData);
	glBegin( GL_QUADS );
		glTexCoord2d(0.0, 0.0);		glVertex2d(0.0,			  0.0);
		glTexCoord2d(1.0, 0.0); 	glVertex2d(display_width, 0.0);
		glTexCoord2d(1.0, 1.0); 	glVertex2d(display_width, display_height);
		glTexCoord2d(0.0, 1.0); 	glVertex2d(0.0,			  display_height);
	glEnd();
}

// Old gfx code
void drawPixel(int x, int y)
{
	glBegin(GL_QUADS);
		glVertex3f((x * modifier) + 0.0f,     (y * modifier) + 0.0f,	 0.0f);
		glVertex3f((x * modifier) + 0.0f,     (y * modifier) + modifier, 0.0f);
		glVertex3f((x * modifier) + modifier, (y * modifier) + modifier, 0.0f);
		glVertex3f((x * modifier) + modifier, (y * modifier) + 0.0f,	 0.0f);
	glEnd();
}

void updateQuads(VCPU* c8)
{
// Draw
	for(int y = 0; y < 32; ++y)		
		for(int x = 0; x < 64; ++x){
			if(c8->getScreen(x,y) == 0) 
				glColor3f(0.0f,0.0f,0.0f);			
			else 
				glColor3f(1.0f,1.0f,1.0f);
			drawPixel(x, y);
		}
}
void display(){
	if(draw || !cycleCap){
		myChip8->runCycle();
		draw = false;
	}
	if(myChip8->getDrawFlag())
	{
		// Clear framebuffer
		glClear(GL_COLOR_BUFFER_BIT);
#ifdef DRAWWITHTEXTURE
		updateTexture(myChip8);
#else
		updateQuads(myChip8);		
#endif			
		// Swap buffers!
		glutSwapBuffers();    
		// Processed frame
		myChip8->setDrawFlag(false);
	}
}
void reshape_window(GLsizei w, GLsizei h){
	glClearColor(0.0f, 0.0f, 0.5f, 0.0f);
	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, w, h, 0);        
    glMatrixMode(GL_MODELVIEW);
    glViewport(0, 0, w, h);
	// Resize quad
	display_width = w;
	display_height = h;
}
void keyboardDown(unsigned char key, int x, int y){
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
void keyboardUp(unsigned char key, int x, int y){
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