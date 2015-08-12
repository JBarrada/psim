#include <GL/gl.h>
#include <GL/glut.h>
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
	//vector pos_prev;
	vector vel;
	particle* neighbors[MAXP];
};

const double radius = 2; // max distance particles affect each other

const double scale = 1;
/*
const double c_radius = 1.0; // distance from wall that counts as collision
const double r_density = 0.5; // rest density
const double l_d_velocity = 0.1; // viscosity's linear dependence on velocity
const double q_d_velocity = 0.1; // viscosity's quadratic dependence on velocity
const double k = 0.1; // stiffness used in double_density_relax
const double k_near = 0.1; // near-stiffness used in double_density_relax
*/
vector gravity = {.5, -M_PI/2};

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
		particles[i].vel = vadd(&particles[i].vel, &gravity);
	}
}

void do_math() {
	int i, n;
	
	double neighbor_effect = 0.1;
	
	for (i=0; i<MAXP; i++) {
		for (n=0; n<MAXP; n++) {
			if (particles[i].neighbors[n]) {
				vector from_neighbor = vsub(&particles[i].pos, &particles[i].neighbors[n]->pos);
				from_neighbor.m *= neighbor_effect;
				particles[i].vel = vadd(&particles[i].vel, &from_neighbor);
			}
			else {
				break;
			}
		}
	}
}

void advance_particles() {
	int i;
	for (i=0; i<MAXP; i++) {
		//particles[i].vel.m *= scale;
		particles[i].pos = vadd(&particles[i].pos, &particles[i].vel);
	}
}

void resolve_collisions() {
	double friction = 0.2;
	
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
	//do_math();
	resolve_collisions();
	advance_particles();
	update_neighbors();
}

int is_particle_here(double x, double y) {
	point current = {x, y};
	vector current_v = cast_vector(&current);
	int i;
	for (i=0; i<MAXP; i++) {
		vector sub = vsub(&particles[i].pos, &current_v);
		if (sub.m<=0.25){
			return 1;
		}
	}
	return 0;
}
void draw() {
	double y, x;
	
	printf("--------------------|\n");
	for (y=HEIGHT-1; y>=0; y-=.5) {
		for (x=0; x<WIDTH; x+=.5) {
			if (is_particle_here(x, y)) {
				printf("0");
			}
			else {
				printf(" ");
			}
		}
		printf("|\n");
	}
}

void debug(char * msg) {
	point pos_xy = cast_point(&particles[0].pos);
	point vel_xy = cast_point(&particles[0].vel);
	
	printf("%s ------------------------------\n", msg); 
	printf("POS: x=%f  y=%f\tm=%f  d=%f\n", pos_xy.x, pos_xy.y, particles[0].pos.m, particles[0].pos.d);
	printf("VEL: x=%f  y=%f\tm=%f  d=%f\n", vel_xy.x, vel_xy.y, particles[0].vel.m, particles[0].vel.d);
	printf("\n");
}

void draw_gl() {
	memset(screenbuf, 0, 400*400);
	
	int i;
	for (i=0; i<MAXP; i++) {
		point particle_pos = cast_point(&particles[i].pos);
		
		int x, y;
		x = particle_pos.x * (400.0/WIDTH);
		y = particle_pos.y * (400.0/HEIGHT);
		
		if (x<400 & y<400 & x>=0 & y>=0) {
			screenbuf[y*400+x] = 0xff;
		}	
	}
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDrawPixels(400, 400, GL_RGB, GL_UNSIGNED_BYTE_3_3_2, screenbuf);
	glutSwapBuffers();
}

void test_function() {
	update();
	
	draw_gl();
	usleep(80000);
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
	
	//particles[0].pos = cast_vector(&(point){10, 10});
	//particles[1].pos = cast_vector(&(point){11, 9.8});
	
	/*
	draw();
	getchar();
	
	for (i=0; i<30; i++) {
		update();
	
		draw();
		debug("");
		getchar();
	}*/
	
	
	
	char fakeParam[] = "";
	char *fakeargv[] = { fakeParam, NULL };
	int fakeargc = 1;
  
	glutInit(&fakeargc, fakeargv);
	glutInitWindowSize(400, 400);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutCreateWindow("!");
	glutIdleFunc(test_function);

	glutMainLoop();
}
