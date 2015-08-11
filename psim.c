#include "otypes.h"
#include <stdio.h>

#define MAXP 100

typedef struct particle particle;

struct particle {
	vector pos;
	vector pos_prev;
	vector vel;
	int index;
	particle* neighbors[MAXP];
};

const float radius = 2.0; // max distance particles affect each other
const float c_radius = 2.0; // distance from wall that counts as collision
const float r_density = 0.5; // rest density
const float l_d_velocity = 0.1; // viscosity's linear dependence on velocity
const float q_d_velocity = 0.1; // viscosity's quadratic dependence on velocity
const float k = 0.1; // stiffness used in double_density_relax
const float k_near = 0.1; // near-stiffness used in double_density_relax
vector gravity = {1, (3*M_PI)/2};

particle particles[MAXP];


void update_neighbors() {
	int i, j, n=0;
	for (i=0; i<MAXP; i++) {
		for (j=0; j<MAXP; j++) {
			if ((&particles[i] != &particles[j]) & (vsub(&particles[j].pos, &particles[i].pos).m < radius)) { 
				particles[i].neighbors[n] = &particles[j];
				n++;
			}
		}
	}
}

void apply_external_forces() {
	int i;
	for (i=0; i<MAXP; i++) {
		particles[i].vel = vadd(&particles[i].vel, &gravity);
	}
}

void update() {
	apply_external_forces();
	// apply_viscosity();
	// advance_particles();
	update_neighbors();
}

void main() {
	int y, x, i=0;
	for (y=0; y<10; y++) {
		for (x=0; x<10; x++) {
			particles[i].pos = cast_vector(&(point){x, y});
			i++;
		}
	}
	
	update_neighbors();
	for (i=0; i<MAXP; i++)
		if (particles[1].neighbors[i])
			printf("neighbor from particle %d m:%f d:%f\n", i, particles[1].neighbors[i]->pos.m, particles[1].neighbors[i]->pos.d);
}