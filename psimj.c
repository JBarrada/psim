#include <GL/gl.h>
#include <GL/freeglut.h>
#include <GL/glext.h>
#include <time.h>

#include <stdio.h>
#include "otypes.h"

#define MAXP 100

#define HEIGHT 20
#define WIDTH 20

unsigned char screenbuf[400*400];

typedef struct particle particle;

struct particle {
	vector pos;
	vector vel;
	vector vel_prev;
	particle* neighbors[MAXP];
};

const double radius = .5; // max distance particles affect each other

const double scale = 1;
vector gravity = {.01, -M_PI/2};

vector mousepoint = {0,0};

particle particles[MAXP];

void update_neighbors() {
	int i, j, n;
	for (i=0; i<MAXP; i++) {
		n=0;
		
		for (j=0; j<MAXP; j++) {
			if ((&particles[i] != &particles[j]) & (vsub(&particles[j].pos, &particles[i].pos).m < radius)) { 
				particles[i].neighbors[n] = &particles[j];
				n++;
			}
		}
		
		for (n=n; n<MAXP; n++) {
			particles[i].neighbors[n] = NULL; // reset remaining neighbor pointers
		}
	}
}

void apply_external_forces() {
	int i;
	for (i=0; i<MAXP; i++) {
		//particles[i].vel = vadd(&particles[i].vel, &gravity);
		
		vector direction = vsub(&mousepoint, &particles[i].pos);
		direction.m = gravity.m;
		
		particles[i].vel = vadd(&particles[i].vel, &direction);
	}
}

void do_math() {
	int i, n;
	
	double neighbor_effect = .5;
	double pressure_effect = .0004;
	
	for (i=0; i<MAXP; i++) {
		for (n=0; n<MAXP; n++) {
			if (particles[i].neighbors[n]) {
				vector from_neighbor = vsub(&particles[i].pos, &particles[i].neighbors[n]->pos);
				from_neighbor.m = (radius - from_neighbor.m)/radius;
				
				vector neighbor_vel = particles[i].neighbors[n]->vel_prev;
				
				neighbor_vel.m *= (from_neighbor.m*pressure_effect);
				
				vector final_vector = vadd(&from_neighbor, &neighbor_vel);
				
				final_vector.m *= neighbor_effect;
				particles[i].vel = vadd(&particles[i].vel, &final_vector);
				particles[i].vel.m *= 0.99;
			}
			else {
				break;
			}
		}
	}
}

void universe() {
	int i, n;
	
	double neighbor_effect = 0.000001;
	
	for (i=0; i<MAXP; i++) {
		for (n=0; n<MAXP; n++) {
			vector from_neighbor = vsub(&particles[i].pos, &particles[n].pos);
			//from_neighbor.m = 1/from_neighbor.m;
			from_neighbor.m *= neighbor_effect;
			particles[i].vel = vadd(&particles[i].vel, &from_neighbor);
		}
	}
}

void advance_particles() {
	int i;
	for (i=0; i<MAXP; i++) {
		particles[i].vel.m *= 0.9;
		particles[i].pos = vadd(&particles[i].pos, &particles[i].vel);
		particles[i].vel_prev = particles[i].vel;
	}
}

void resolve_collisions() {
	double friction = 0.9;
	
	int i;
	double distance;
	vector normal;
	for (i=0; i<MAXP; i++) {
		point p_pos = cast_point(&particles[i].pos);
		
		//left 
		distance = p_pos.x;
		normal = (vector) {1, 0};
		
		//right
		if (WIDTH-p_pos.x < distance) {
			distance = WIDTH-p_pos.x;
			normal = (vector) {1, M_PI};
		}
		
		//bottom
		if (p_pos.y < distance) {
			distance = p_pos.y;
			normal = (vector) {1, M_PI/2};
		}
		
		//bottom
		if (HEIGHT-p_pos.y < distance) {
			distance = HEIGHT-p_pos.y;
			normal = (vector) {1, -M_PI/2};
		}
		
		if (distance < radius) {
			double projection = vvmult(&particles[i].vel, &normal);
			vector scaled = vsmult(&normal, 2*projection);
			vector reflection = vsub(&particles[i].vel, &scaled);
			reflection.m *= friction;
			
			particles[i].vel = reflection;
		}
	}
}

void update() {
	apply_external_forces();
	do_math();
	//universe();
	//resolve_collisions();
	advance_particles();
	update_neighbors();
}

void debug(char * msg) {
	point pos_xy = cast_point(&particles[0].pos);
	point vel_xy = cast_point(&particles[0].vel);
	
	printf("%s ------------------------------\n", msg); 
	printf("POS: x=%f  y=%f\tm=%f  d=%f\n", pos_xy.x, pos_xy.y, particles[0].pos.m, particles[0].pos.d);
	printf("VEL: x=%f  y=%f\tm=%f  d=%f\n", vel_xy.x, vel_xy.y, particles[0].vel.m, particles[0].vel.d);
	printf("\n");
}

void set_pixel(int x, int y, unsigned char c) {
	if (x<400 & y<400 & x>=0 & y>=0) {
		screenbuf[y*400+x] = c;
	}	
}

void draw_line(int x1,int y1,int x2,int y2, unsigned char c) {
	double dx, dy;
	dx = (x2-x1);
	dy = (y2-y1);
	
	int i;
	if (abs(dx) > abs(dy)) {
		double slope = dy/dx;
		for (i=0; i!=dx; i+=(dx>0)?1:-1) {
			set_pixel(i+x1, slope*i+y1, c);
		}
	}
	else {
		double slope = dx/dy;
		for (i=0; i!=dy; i+=(dy>0)?1:-1) {
			set_pixel(slope*i+x1, i+y1, c);
		}
	}
}

void draw_circle(int cx, int cy, double r) {
	int i;
	double x, y;
	for (i=0; i<40; i++) {
		x = r * cos(((M_PI/2)/40)*i);
		y = r * sin(((M_PI/2)/40)*i);
		
		set_pixel(cx+x, cy+y, 0xff);
		set_pixel(cx-x, cy+y, 0xff);
		set_pixel(cx+x, cy-y, 0xff);
		set_pixel(cx-x, cy-y, 0xff);
	}
}

void draw_gl() {
	memset(screenbuf, 0, 400*400);
	
	point gravpoint = cast_point(&gravity);
	draw_line(200,200, 200+gravpoint.x*1000, 200+gravpoint.y*1000, 0xE0);
	
	int i;
	for (i=0; i<MAXP; i++) {
		point particle_pos = cast_point(&particles[i].pos);
		
		int x, y;
		x = particle_pos.x * (400.0/WIDTH);
		y = particle_pos.y * (400.0/HEIGHT);
		
		draw_circle(x, y, radius*(400.0/WIDTH)/2);
		//set_pixel(x, y, 0xff);
	}
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDrawPixels(400, 400, GL_RGB, GL_UNSIGNED_BYTE_3_3_2, screenbuf);
	glutSwapBuffers();
}

void test_function() {
	update();
	
	draw_gl();
	usleep(10000);
}

void keybd(unsigned char key, int x, int y) {
	point mouse_now = {(x/400.0)*WIDTH, HEIGHT-((y/400.0)*HEIGHT)};
	mousepoint = cast_vector(&mouse_now);
	
	if (key == 97) {
		gravity.d -= M_PI/20; 
	}
	else if (key == 100) {
		gravity.d += M_PI/20; 
	}
}

void main() {
	int y, x, i=0;
	
	for (y=0; y<10; y++) {
		for (x=0; x<10; x++) {
			particles[i].pos = cast_vector(&(point){x+5, y+10});
			i++;
		}
	}
	
	memset(screenbuf, 0, 400*400);
	
	
	char fakeParam[] = "";
	char *fakeargv[] = { fakeParam, NULL };
	int fakeargc = 1;
  
	glutInit(&fakeargc, fakeargv);
	glutInitWindowSize(400, 400);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutCreateWindow("!");
	glutIdleFunc(test_function);
	glutDisplayFunc(draw_gl);
	glutKeyboardFunc(keybd);

	glutMainLoop();
}
