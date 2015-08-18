#include <time.h>
#include <stdio.h>

#include "otypes.h"

#define MAXP 2

typedef struct particle particle;
struct particle {
	vector pos;
	vector vel;
	vector vel_prev;
	point posxy;
	unsigned int index;
	particle* neighbors[MAXP];
};

typedef struct collision {
	double distance;
	vector normal;
} collision;

particle particles[MAXP];

const double radius = 1;

#include "gfx.c"

point focalpoint = {WIDTH_G/2,HEIGHT_G/2};
vector focalvector = {0, 0};

vector gravity = {0.01, 3*M_PI/2};

void update_neighbors() {
	int i, j, n;
	for (i=0; i<MAXP; i++) {
		n=0;
		
		int tl, tm, tr, ml, mm, mr, bl, bm, br, stride, pj;
		
		stride = WIDTH_G;
		
		tl = particles[i].index - stride - 1;
		tm = particles[i].index - stride;
		tr = particles[i].index - stride + 1;
		ml = particles[i].index - 1;
		mm = particles[i].index;
		mr = particles[i].index + 1;
		bl = particles[i].index + stride - 1;
		bm = particles[i].index + stride;
		br = particles[i].index + stride + 1;
		
		for (j=0; j<MAXP; j++) {
			pj = particles[j].index;
			
			if (pj==tl | pj==tm | pj==tr | pj==ml | pj==mm | pj==mr | pj==bl | pj==bm | pj==br) {
				double distance = vsub(&particles[i].pos, &particles[j].pos).m;
				if ((&particles[i] != &particles[j]) & (distance < (radius))) { 
					particles[i].neighbors[n] = &particles[j];
					n++;
				}
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
		//vector direction = vsub(&focalvector, &particles[i].pos);
		//direction.m = 0.01;
		//particles[i].vel = vadd(&particles[i].vel, &direction);
		
		particles[i].vel = vadd(&particles[i].vel, &gravity);
	}
}

void do_math() {
	int i, n;
	
	double neighbor_effect = .5;
	
	for (i=0; i<MAXP; i++) {
		for (n=0; n<MAXP; n++) {
			if (particles[i].neighbors[n]) {
				vector from_neighbor = vsub(&particles[i].pos, &particles[i].neighbors[n]->pos);
				from_neighbor.m = (radius - from_neighbor.m)/radius;
				
				vector final_vector = from_neighbor;
				
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

void advance_particles() {
	int i;
	for (i=0; i<MAXP; i++) {
		particles[i].vel.m *= 0.98;
		particles[i].pos = vadd(&particles[i].pos, &particles[i].vel);
		
		particles[i].posxy = cast_point(&particles[i].pos);
		particles[i].index = (int)(particles[i].posxy.y)*(WIDTH_G) + (int)(particles[i].posxy.x);
	}
}

collision get_nearest(point p) {
	double distance;
	vector normal;
	distance = p.x;
	normal = (vector) {1, 0};
	
	//right
	if ((WIDTH_G/1)-p.x < distance) {
		distance = (WIDTH_G/1)-p.x;
		normal = (vector) {1, M_PI};
	}
	
	//bottom
	if (p.y < distance) {
		distance = p.y;
		normal = (vector) {1, M_PI/2};
	}
	
	//top
	if (HEIGHT_G-p.y < distance) {
		distance = HEIGHT_G-p.y;
		normal = (vector) {1, -M_PI/2};
	}
	
	return (collision){distance, normal};
}

void resolve_collisions() {
	double friction = 0.9;
	
	int i;

	for (i=0; i<MAXP; i++) {
		point p_pos = cast_point(&particles[i].pos);
		
		collision c = get_nearest(p_pos);
		
		if (c.distance<radius & c.distance>-radius) {
			//printf("%f\n", c.distance);
			particles[i].vel.m -= pow(particles[i].vel.m*(radius-c.distance), 4);
			
			//printf("IN M: %f ---", particles[i].vel);
			
			double projection = vvmult(&particles[i].vel, &c.normal);
			vector scaled = vsmult(&c.normal, 2*projection);
			vector reflection = vsub(&particles[i].vel, &scaled);
			reflection.m *= friction;
			
			particles[i].vel = reflection;
			
			//printf("OUT M: %f\n", particles[i].vel);
			vector newpos = vadd(&particles[i].vel, &particles[i].pos);
			c = get_nearest(cast_point(&newpos));
			c.normal.m = radius-c.distance;
			if (c.distance < radius) {
				printf("SUPPLEMENT #%d dist:%f\n", i, c.distance);
				particles[i].pos = vadd(&particles[i].pos, &c.normal);
			}
			
			if (particles[i].vel.m < .06) {
				particles[i].vel.m = 0;
			}
		}
	}
}

void update() {
	apply_external_forces();
	do_math();
	resolve_collisions();
	advance_particles();
	//printf("m: %f       y: %f\n\n",particles[0].vel, particles[0].posxy.y);
	update_neighbors();
}

void test_function() {
	clock_t tic = clock();

    update();

    clock_t toc = clock();

    //printf("%f mS\n", ((double)(toc - tic) / CLOCKS_PER_SEC)*1000);
	
	render();
	//usleep(10000);
}

void main() {
	focalvector = cast_vector(&focalpoint);
	
	int y, x, i=0;
	/*
	for (y=0; y<10; y++) {
		for (x=0; x<50; x++) {
			particles[i].pos = cast_vector(&(point){(x/4.0)+5, (y/4.0)+10});
			i++;
		}
	}
	*/
	particles[0].pos = cast_vector(&(point){32, 24});
	//particles[0].vel = (vector){2, -M_PI/2};
	
	particles[1].pos = cast_vector(&(point){64-8, 8});
	////particles[1].vel = (vector){0.6, M_PI};
	particles[1].vel = vsub(&focalvector, &particles[1].pos);
	particles[1].vel.m = 0.8;
	

	char fakeParam[] = "";
	char *fakeargv[] = { fakeParam, NULL };
	int fakeargc = 1;
  
	glutInit(&fakeargc, fakeargv);
	glutInitWindowSize(WIDTH_W, HEIGHT_W);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutCreateWindow("!");
	glutIdleFunc(test_function);
	glutDisplayFunc(render);

	glutMainLoop();
}
