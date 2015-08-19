#include <GL/gl.h>
#include <GL/freeglut.h>
#include <GL/glext.h>

unsigned char screen_buffer[WIDTH_W*HEIGHT_W];

void (*draw_function)();

void set_pixel(int x, int y, unsigned char c) {
	if (x<WIDTH_W & y<HEIGHT_W & x>=0 & y>=0) {
		screen_buffer[y*WIDTH_W+x] = c;
	}	
}

void line(int x1,int y1,int x2,int y2, unsigned char c) {
	int dx, dy, dxabs, dyabs, x, y, px, py;
	dx = (x2-x1);
	dy = (y2-y1);
	dxabs=abs(dx);
	dyabs=abs(dy);
	
	if (dyabs < 1000 & dxabs < 1000) {
		x=dyabs>>1;
		y=dxabs>>1;
		px=x1;
		py=y1;
		
		int i;
		if (dxabs>=dyabs) {
			for(i=0;i<dxabs;i++) {
				y+=dyabs;
				if (y>=dxabs) {
					y-=dxabs;
					py+=(dy>=0)?1:-1;
				}
				px+=(dx>=0)?1:-1;
				set_pixel(px,py,c);
			}
		}
		else {
			for(i=0;i<dyabs;i++) {
				x+=dxabs;
				if (x>=dyabs) {
					x-=dyabs;
					px+=(dx>=0)?1:-1;
				}
				py+=(dy>=0)?1:-1;
				set_pixel(px,py,c);
			}
		}
	}
}

void circle(int cx, int cy, double r, unsigned char c) {
	int i, res=5;
	double x, y, x_, y_;
	for (i=0; i<=res; i++) {
		x = r * cos(((M_PI/2)/res)*i);
		y = r * sin(((M_PI/2)/res)*i);
		
		if (i != 0) {
			line(cx+x_, cy+y_, cx+x, cy+y, c);
			line(cx-x_, cy+y_, cx-x, cy+y, c);
			line(cx+x_, cy-y_, cx+x, cy-y, c);
			line(cx-x_, cy-y_, cx-x, cy-y, c);
		}
		
		x_ = x;
		y_ = y;
	}
}

void render() {
	memset(screen_buffer, 0, WIDTH_W*HEIGHT_W);
	
	draw_function();
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDrawPixels(WIDTH_W, HEIGHT_W, GL_RGB, GL_UNSIGNED_BYTE_3_3_2, screen_buffer);
	glutSwapBuffers();
}

void init(void (*idle)(), void (*draw)()) {
	draw_function = draw;
	
	char fakeParam[] = "";
	char *fakeargv[] = { fakeParam, NULL };
	int fakeargc = 1;
	
	glutInit(&fakeargc, fakeargv);
	glutInitWindowSize(WIDTH_W, HEIGHT_W);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutCreateWindow("!");
	glutIdleFunc(idle);
	glutDisplayFunc(render);

	glutMainLoop();
}

