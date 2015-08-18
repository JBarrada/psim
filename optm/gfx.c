#include <GL/gl.h>
#include <GL/freeglut.h>
#include <GL/glext.h>

#define WIDTH_W 640
#define HEIGHT_W 480

#define WIDTH_G 64
#define HEIGHT_G 48

#define IRES 16.0

unsigned char screen_buffer[WIDTH_W*HEIGHT_W];

void set_pixel(int x, int y, unsigned char c) {
	if (x<WIDTH_W & y<HEIGHT_W & x>=0 & y>=0) {
		screen_buffer[y*WIDTH_W+x] = c;
	}	
}

void draw_line(int x1,int y1,int x2,int y2, unsigned char c) {
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

void draw_circle(int cx, int cy, double r, unsigned char c) {
	int i, res=8;
	double x, y;
	for (i=0; i<res; i++) {
		x = r * cos(((M_PI/2)/res)*i);
		y = r * sin(((M_PI/2)/res)*i);
		
		set_pixel(cx+x, cy+y, c);
		set_pixel(cx-x, cy+y, c);
		set_pixel(cx+x, cy-y, c);
		set_pixel(cx-x, cy-y, c);
	}
}

#include "convex_hull.c"

void render() {
	memset(screen_buffer, 0, WIDTH_W*HEIGHT_W);
	
	/*
	for (y=0; y<HEIGHT_G; y+=1) {
		draw_line(0, y*(HEIGHT_W/HEIGHT_G), WIDTH_W, y*(HEIGHT_W/HEIGHT_G), 0x24);
	}
	for (x=0; x<WIDTH_G; x+=1) {
		draw_line(x*(WIDTH_W/WIDTH_G), 0, x*(WIDTH_W/WIDTH_G), HEIGHT_W, 0x24);
	}
	*/
	
	point points[MAXP];
	int i, x, y;
	
	for (i=0; i<MAXP; i++) {
		
		x = particles[i].posxy.x * (WIDTH_W/WIDTH_G);
		y = particles[i].posxy.y * (HEIGHT_W/HEIGHT_G);
		
		points[i].x = x;
		points[i].y = y;
		
		if (i==0)
			draw_circle(x, y, radius*(WIDTH_W/WIDTH_G)/2, 0xe0);
		else
			draw_circle(x, y, radius*(WIDTH_W/WIDTH_G)/2, 0x03);
	}
	
	//convex_hull(points, MAXP);
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDrawPixels(WIDTH_W, HEIGHT_W, GL_RGB, GL_UNSIGNED_BYTE_3_3_2, screen_buffer);
	glutSwapBuffers();
}