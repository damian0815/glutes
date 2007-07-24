/* 
 * GLUT|ES Sample program.
 *
 * Copyright (c) Joachim Pouderoux, 2005.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

// Warning : this sample must be compiled with the UNICODE char set
//           because of wsprintf used to format a string for glutTrueTypeString

#include <windows.h>
#include <stdio.h>

#pragma warning(disable : 4305)

#define GLUTES_STATIC
//#define GLUTES_USE_TEXTURED_FONTS	// if not defined, fonts are rendered as points cloud.
//#define GLES_CL					// uncomment for the common light (CL) mode

#include <GLES/glutes.h>

// Set these 2 macros to the wanted/possible resolution
//#define SCREEN_WIDTH	480 // To enable VGA mode on PocketPC, see readme.txt
//#define SCREEN_HEIGHT	640
#define SCREEN_WIDTH	240
#define SCREEN_HEIGHT	320

// Macros for float/int from/to fixed conversions
#define _INT2FIXED(_x_)		(GLfixed)((_x_) << 16)
#define _FIXED2INT(_x_)		(int)((_x_) >> 16)
#define _FLOAT2FIXED(_x_)	(GLfixed)((_x_) *  (float)(1 << 16) + 0.5f)
#define _FIXED2FLOAT(_x_)	((float)(_x_) / (float)(1 << 16))


static GLfloat gAngle = 0.f;
static int gAnimate = 1; 
static int gPrimitive = 20;
static int gRandomPrimitive = 1;

static int gShowFps = 1; 
DWORD gTime = 0;  


void init(void)
{
#ifndef GLES_CL
	GLfloat position[] = { 0.5, 0.5, 3.0, 0.0 };
	
	glEnable(GL_DEPTH_TEST);
		
	glLightfv(GL_LIGHT0, GL_POSITION, position);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	{
		GLfloat mat[3] = {0.1745, 0.01175, 0.01175};	
		glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT, mat);
		mat[0] = 0.04136; mat[1] = 0.04136; mat[2] = 0.61424;	
		glMaterialfv (GL_FRONT_AND_BACK, GL_DIFFUSE, mat);
		mat[0] = 0.727811; mat[1] = 0.626959; mat[2] = 0.626959;
		glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, mat);
		glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, 0.6*128.0);
	}
	
	glEnable(GL_FOG);
	{
		GLfloat fogColor[4] = {0.5, 0.5, 0.5, 1.0};
		
		glFogx (GL_FOG_MODE, GL_EXP);
		glFogfv (GL_FOG_COLOR, fogColor);
		glFogf (GL_FOG_DENSITY, 0.35);
		glHint (GL_FOG_HINT, GL_DONT_CARE);
		glFogf (GL_FOG_START, -20.0);
		glFogf (GL_FOG_END, -13.0);
	}
	glClearColor(0.5, 0.5, 0.5, 1.0);

#else // CL profile

	GLfixed position[] = { _FLOAT2FIXED(0.5), _FLOAT2FIXED(0.5), _FLOAT2FIXED(3.0), _FLOAT2FIXED(0.0) };
	
	glEnable(GL_DEPTH_TEST);
	
	glLightxv(GL_LIGHT0, GL_POSITION, position);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	{
		GLfixed mat[3] = {_FLOAT2FIXED(0.1745), _FLOAT2FIXED(0.01175), _FLOAT2FIXED(0.01175)};	
		glMaterialxv (GL_FRONT_AND_BACK, GL_AMBIENT, mat);
		mat[0] = _FLOAT2FIXED(0.04136); mat[1] = _FLOAT2FIXED(0.04136); mat[2] = _FLOAT2FIXED(0.61424);	
		glMaterialxv (GL_FRONT_AND_BACK, GL_DIFFUSE, mat);
		mat[0] = _FLOAT2FIXED(0.727811); mat[1] = _FLOAT2FIXED(0.626959); mat[2] = _FLOAT2FIXED(0.626959);
		glMaterialxv (GL_FRONT_AND_BACK, GL_SPECULAR, mat);
		glMaterialx (GL_FRONT_AND_BACK, GL_SHININESS, _FLOAT2FIXED(0.6*128.0)); 
	}
	 
	glEnable(GL_FOG); 
	{
		GLfixed fogColor[4] = {_FLOAT2FIXED(0.5), _FLOAT2FIXED(0.5), _FLOAT2FIXED(0.5), _FLOAT2FIXED(1.0)};
		
		glFogx (GL_FOG_MODE, GL_EXP);
		glFogxv (GL_FOG_COLOR, fogColor);
		glFogx (GL_FOG_DENSITY, _FLOAT2FIXED(0.35));
		glHint (GL_FOG_HINT, GL_DONT_CARE);
		glFogx (GL_FOG_START, _FLOAT2FIXED(-20.0));
		glFogx (GL_FOG_END, _FLOAT2FIXED(-13.0));
	}
	glClearColorx(_FLOAT2FIXED(0.5), _FLOAT2FIXED(0.5), _FLOAT2FIXED(0.5), _FLOAT2FIXED(1.0));
#endif
}

#ifndef GLES_CL
void renderPrimitive(GLfloat x, GLfloat y, GLfloat z)
{
	glPushMatrix();
	glTranslatef(x, y, z);
	switch(gPrimitive)
	{
	case 20:
		glutSolidSphere(0.4, 10, 10);
		break;
	case 21:
		glutSolidCone(0.4, 1.0, 10, 10);
		break;
	case 22:
		glutSolidCube(0.8); 
		break;
	case 23: 
		glutSolidTorus(0.2, 0.3, 10, 10);
		break;
	case 24:
		glutWireSphere(0.4, 10, 10);
		break;  
	case 25:  
	default: 
		glutWireCube(0.8);   
		break;
	} 
	glPopMatrix();
} 

#else

void renderPrimitive(GLfixed x, GLfixed y, GLfixed z)
{
	glPushMatrix();
	glTranslatex(x, y, z);
	switch(gPrimitive)
	{
	case 20:
		glutSolidSpherex(_FLOAT2FIXED(0.4), 10, 10);
		break;
	case 21:
		glutSolidConex(_FLOAT2FIXED(0.4), _FLOAT2FIXED(1.0), 10, 10);
		break;
	case 22:
		glutSolidCubex(_FLOAT2FIXED(0.8)); 
		break;
	case 23: 
		glutSolidTorusx(_FLOAT2FIXED(0.2), _FLOAT2FIXED(0.3), 10, 10);
		break;
	case 24:
		glutWireSpherex(_FLOAT2FIXED(0.4), 10, 10);
		break;  
	case 25:
	default: 
		glutWireCubex(_FLOAT2FIXED(0.8));   
		break;
	} 
	glPopMatrix();
} 
#endif

void display()
{
	//char txt[150]; 
	WCHAR wtxt[150];
	DWORD tmp = GetTickCount();
	DWORD dt = tmp - gTime;
	gTime = tmp; 

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glPushMatrix();

	glDisable(GL_CULL_FACE);

#ifndef GLES_CL

	glOrthof(-2.5, 2.5, -2.5,  2.5, -10.0, 10.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity ();
	glTranslatef(0, 0, -2.f);

	glEnable(GL_LIGHTING);
	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);

	glPushMatrix();
	glRotatef(gAngle, 1., 1., 1.);
	{
		//glutSolidTeapot(1);
		renderPrimitive(-2., 0., 0.);
		renderPrimitive(-1., 0., 0.);
		renderPrimitive(0., 0., 0.);
		renderPrimitive(1., 0., 0.);
		renderPrimitive(2., 0., 0.);
	}
	glPopMatrix();
	
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glColor4f(1., 0, 0, 1);
	glRotatef(-gAngle, 1., 1., 1.);
	glTranslatef(-2., 0, 0);
	glScalef (0.003,0.003, 0.1);
	glLineWidth(2);
	glutStrokeString(GLUT_STROKE_ROMAN, "Welcome on GLUT|ES!");
	glLineWidth(1);

	if(gShowFps)
	{
		glDisable(GL_LIGHTING);
		glColor4f(1.f, 1.f, 1.f, 1);
		wsprintf(wtxt, L"GLUT|\x42DS @ %d fps", (int)((float)1000.f/(float)dt));
		//glutBitmapString(GLUT_BITMAP_8_BY_13, 5, 5, txt);  
		glutTrueTypeString(L"Arial", 12, 1|2, 5, 5, wtxt);  
	}
	glPopMatrix();
	glutSwapBuffers();

#else
	
	glOrthox(_FLOAT2FIXED(-2.5), _FLOAT2FIXED(2.5), _FLOAT2FIXED(-2.5),  _FLOAT2FIXED(2.5), _FLOAT2FIXED(-10.0), _FLOAT2FIXED(10.0));
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity ();
	glTranslatex(0, 0, _FLOAT2FIXED(-2.0));

	glEnable(GL_LIGHTING);
	glClearColorx(_FLOAT2FIXED(0.5), _FLOAT2FIXED(0.5), _FLOAT2FIXED(0.5), _FLOAT2FIXED(1.0));
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glPushMatrix();
	glRotatex(_FLOAT2FIXED(gAngle), _FLOAT2FIXED(1.0), _FLOAT2FIXED(1.0), _FLOAT2FIXED(1.0));
	{
		renderPrimitiveCL(_FLOAT2FIXED(-2.0), 0, 0);
		renderPrimitiveCL(_FLOAT2FIXED(-1.0), 0, 0);
		renderPrimitiveCL(_FLOAT2FIXED(0.0), 0, 0);
		renderPrimitiveCL(_FLOAT2FIXED(1.0), 0, 0);
		renderPrimitiveCL(_FLOAT2FIXED(2.0), 0, 0);
	}
	glPopMatrix();
	
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glColor4x(_FLOAT2FIXED(1.0), _FLOAT2FIXED(0.0), _FLOAT2FIXED(0.0), _FLOAT2FIXED(1.0));
	glRotatex(_FLOAT2FIXED(-gAngle), _FLOAT2FIXED(1.0), _FLOAT2FIXED(1.0), _FLOAT2FIXED(1.0));
	glTranslatex(_FLOAT2FIXED(-2.0), 0, 0);
	glScalex(_FLOAT2FIXED(0.003), _FLOAT2FIXED(0.003), _FLOAT2FIXED(0.1));
	glLineWidthx(_FLOAT2FIXED(2.0));
	glutStrokeString(GLUT_STROKE_ROMAN, "Welcome on GLUT|ES!");
	glLineWidthx(_FLOAT2FIXED(1.0));

	if(gShowFps)
	{
		glDisable(GL_LIGHTING);
		glColor4x(_FLOAT2FIXED(1.0), _FLOAT2FIXED(1.0), _FLOAT2FIXED(1.0), _FLOAT2FIXED(1));
		wsprintf(wtxt, L"GLUT|\x42DS @ %d fps", (int)((float)1000.f/(float)dt));		
		//glutBitmapString(GLUT_BITMAP_8_BY_13, 5, 5, txt); 
		glutTrueTypeString(L"Arial", 12, 1|2, 5, 5, wtxt);
	}
#endif

	glPopMatrix();
	glutSwapBuffers();
}


void reshape(int w, int h)
{ 
	glViewport(0, 0, (GLsizei) w, (GLsizei) h);
}

void idle(void)
{
	if(gAnimate)
		gAngle += 10.f;

	if(gAngle >= 360.f)
	{
		gAngle -= 360.f;
		if(gRandomPrimitive)
			gPrimitive = (gPrimitive - 20 + 1) % 6  + 20;
	}
    glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key) 
	{
	case '\n':
		// Simulate a RIGHT click to open the menu
		glutSimulateButton(GLUT_RIGHT_BUTTON, 10, 20);
		break;
	default:
		break;
    }
}

void special(int key, int x, int y)
{
    switch (key)
	{
	case GLUT_KEY_UP:
	case GLUT_KEY_DOWN:
	case GLUT_KEY_LEFT:
	case GLUT_KEY_RIGHT:
	case GLUT_KEY_F4: 
		gAnimate = !gAnimate;
		break;
	default:
		break;
    }
}

void specialup(int key, int x, int y)
{
    switch (key)
	{
	
	case GLUT_KEY_F2: 
		gAnimate = !gAnimate;
		break;

	case GLUT_KEY_F3:
		glutSimulateButton(GLUT_RIGHT_BUTTON, 10, 20);
		break;
	default:
		break;
    }
}

void mouse(int button, int state, int x, int y)
{
	glutSimulateButton(GLUT_RIGHT_BUTTON, x, y);
}

void menu(int entry)
{
	switch(entry)
	{
	case 1:
		exit(0);
		break;
	case 10: 
		gShowFps = 1; 
		break; 
	case 11:
		gShowFps = 0;
		break;
	case 20: case 21: case 22: case 23: case 24: case 25:
		gRandomPrimitive = 0;
		gPrimitive = entry;
		break;
	case 29:
		gRandomPrimitive = 1;
		break;
	default:
		break;
	}
}

int main(int argc, char **argv)
{
	int submenu1, submenu2;

    glutInit(&argc, argv); 

	//glutInitDisplayString("depth=8 stencil=8");
	
	glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT); 
    glutCreateWindow("Hello GLUT|ES"); 
	    
    glutDisplayFunc(display); 
    glutIdleFunc(idle); 
    glutKeyboardFunc(keyboard); 
	glutSpecialFunc(special); 
	//glutSpecialUpFunc(specialup);   
	glutMouseFunc(mouse);
	   
	glutReshapeFunc(reshape); 
	
	// Create menu
	submenu1 = glutCreateMenu(menu); 
	glutAddMenuEntry("Show",		10);
	glutAddMenuEntry("Hide",		11);
 
	submenu2 = glutCreateMenu(menu); 
	glutAddMenuEntry("Sphere",		20);
	glutAddMenuEntry("Cone",		21);
	glutAddMenuEntry("Cube",		22);  
	glutAddMenuEntry("Torus",		23);
	glutAddMenuEntry("Wire sphere",	24);
	glutAddMenuEntry("Wire cube",	25); 
	glutAddMenuEntry("Random",		29); 
	 
	glutCreateMenu(menu); 
	glutAddSubMenu("Model",		 	submenu2);
	glutAddSubMenu("FPS",			submenu1);
	glutAddMenuEntry("Quit",		1);
	
	glutAttachMenu(GLUT_RIGHT_BUTTON); 

	init();
	 
	gTime = GetTickCount();
	 
    glutMainLoop();	
	
    return 0; 
}
