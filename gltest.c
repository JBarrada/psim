#include <GL/gl.h>
#include <GL/glut.h>
#include <stdio.h>

unsigned char screenbuf[400*400];


void main_loop_function() {
   // Clear color (screen) 
   // And depth (used internally to block obstructed objects)
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   memset(screenbuf, 1, 400*400);
   
   glDrawPixels(400, 400, GL_RGB, GL_UNSIGNED_BYTE_3_3_2, screenbuf);
   
   // Swap buffers (color buffers, makes previous render visible)
	glutSwapBuffers();
}

void main() {
	
	char fakeParam[] = "";
	char *fakeargv[] = { fakeParam, NULL };
	int fakeargc = 1;
  
	glutInit(&fakeargc, fakeargv);
		
	glutInitWindowSize(400, 400);

	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);

	glutCreateWindow("GLUT Example!!!");

	glutIdleFunc(main_loop_function);
	
	glutMainLoop();
}