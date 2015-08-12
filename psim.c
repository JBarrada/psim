#include <stdio.h>
#include "otypes.h"

#define MAXP 100

#define HEIGHT 20
#define WIDTH 20

typedef struct particle particle;

struct particle {
	vector pos;
	vector pos_prev;
	vector vel;
	int index;
	particle* neighbors[MAXP];
};

const double radius = 5.0; // max distance particles affect each other
const double c_radius = 1.0; // distance from wall that counts as collision
const double r_density = 0.5; // rest density
const double l_d_velocity = 0.1; // viscosity's linear dependence on velocity
const double q_d_velocity = 0.1; // viscosity's quadratic dependence on velocity
const double k = 0.1; // stiffness used in double_density_relax
const double k_near = 0.1; // near-stiffness used in double_density_relax
vector gravity = {1, -M_PI/2};

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

void apply_viscosity(double time_step) {
	int i, n;
	for (i=0; i<MAXP; i++) {
		printf("-------\n");
		for (n=0; n<MAXP; n++) {
			if (particles[i].neighbors[n]) {
				printf("neigh\n");
				vector v_ni = vsub(&particles[i].pos, &particles[i].neighbors[n]->pos);
				
				vector vel_in = vsub(&particles[i].neighbors[n]->vel, &	particles[i].vel);
				double vel_inward = vvmult(&vel_in, &v_ni);
				
				if (vel_inward > 0.0) {
					double length = v_ni.m;
					vel_inward /= length;
					v_ni.m /= length; // normalize vector
					double q = length/radius;
					
					double i_scal = 0.5*time_step*(1-q)*(l_d_velocity*vel_inward + q_d_velocity*(vel_inward*vel_inward));
					vector i_vect = vsmult(&v_ni, i_scal);
					
					particles[i].vel = vsub(&particles[i].vel, &i_vect);
				}
			}
			else {
				break;
			}
		}
	}
}

void advance_particles(double time_step) {
	int i;
	for (i=0; i<MAXP; i++) {
		particles[i].pos_prev = particles[i].pos; // might have to make a new copy
		particles[i].pos = vsmult(&particles[i].vel, time_step);
		particles[i].pos = vadd(&particles[i].pos_prev, &particles[i].pos);
		// grid move particle
	}
}

void double_density_relax(double time_step) {
	int i, n;
	for (i=0; i<MAXP; i++) {
		double dens = 0;
		double dens_near = 0;
		
		for (n=0; n<MAXP; n++) {
			if (particles[i].neighbors[n]) {
				double temp_n = vsub(&particles[i].pos, &particles[i].neighbors[n]->pos).m;
				double q = 1.0 - (temp_n/radius);
				dens = dens + (q*q);
				dens_near = dens_near + (q*q*q);
			}
			else {
				break;
			}
		}
		
		dens = k * (dens - r_density);
		dens_near = k_near * dens_near;
		double delta = 0;
		vector d;
		for (n=0; n<MAXP; n++) {
			if (particles[i].neighbors[n]) {
				double temp_n = vsub(&particles[i].pos, &particles[i].neighbors[n]->pos).m;
				double q = 1.0 - (temp_n/radius);
				
				vector ni_unit = vsub(&particles[i].pos, &particles[i].neighbors[n]->pos);
				ni_unit.m = ni_unit.m / temp_n;
				
				double d_mult = 0.5*(time_step*time_step)*(dens*q + dens_near*(q*q));
				d = vsmult(&ni_unit, d_mult);
				particles[i].neighbors[n]->pos = vadd(&particles[i].neighbors[n]->pos, &d);
				// delta?
			}
			else {
				break;
			}
		}
		
		particles[i].pos = vadd(&particles[i].pos, &d); // i dont know what is happening
	}
}

void resolve_collisions(double time_step) {
	int i;
	double distance;
	vector tangent, normal;
	for (i=0; i<MAXP; i++) {
		point p_pos = cast_point(&particles[i].pos);
		
		//left 
		distance = p_pos.x;
		tangent = (vector) {1, M_PI/2};
		normal = (vector) {1, 0};
		
		//right
		if (WIDTH-p_pos.x < distance) {
			distance = WIDTH-p_pos.x;
			tangent = (vector) {1, -M_PI/2};
			normal = (vector) {1, M_PI};
		}
		
		//bottom
		if (p_pos.y < distance) {
			distance = p_pos.y;
			tangent = (vector) {1, M_PI};
			normal = (vector) {1, M_PI/2};
		}
		
		if (distance < c_radius) {
			double friction = 0.1;
			tangent = vsmult(&tangent, time_step*0.1); // friction
			
			particles[i].pos = vsub(&particles[i].pos, &tangent);
			
			double collision_softness = 0.1;
			vector soft = vsmult(&normal, (radius+distance)*collision_softness);
			particles[i].pos = vsub(&particles[i].pos, &soft);
		}
	}
}


void debug(char * msg) {
	point pos_xy = cast_point(&particles[1].pos);
	point vel_xy = cast_point(&particles[1].vel);
	
	printf("%s ------------------------------\n", msg); 
	printf("POS: x=%f  y=%f\tm=%f  d=%f\n", pos_xy.x, pos_xy.y, particles[1].pos.m, particles[1].pos.d);
	printf("VEL: x=%f  y=%f\tm=%f  d=%f\n", vel_xy.x, vel_xy.y, particles[1].vel.m, particles[1].vel.d);
	printf("\n");
}

void update() {
	apply_external_forces();
	apply_viscosity(0.05);
	advance_particles(0.05);
	update_neighbors();
	double_density_relax(0.05);
	// resolve_collisions(0.05);
	// update_velocity();
}

int is_particle_here(int x, int y) {
	point pos_xy;
	int i;
	for (i=0; i<MAXP; i++) {
		pos_xy = cast_point(&particles[i].pos);
		if ((floor(pos_xy.x)) == x & (floor(pos_xy.y) == y)){
			return 1;
		}
	}
	return 0;
}
void draw() {
	int y, x;
	
	printf("--------------------|\n");
	for (y=HEIGHT-1; y>=0; y--) {
		for (x=0; x<WIDTH; x++) {
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


void main() {
	int y, x, i=0;
	for (y=0; y<10; y++) {
		for (x=0; x<10; x++) {
			particles[i].pos = cast_vector(&(point){x, y+10});
			i++;
		}
	}
	
	
	
	draw();
	getchar();
	
	for (i=0; i<30; i++) {
		update();
	
		draw();
		debug("");
		getchar();
	}
	
	
	/*
	for (i=0; i<MAXP; i++)
		if (particles[1].neighbors[i])
			printf("neighbor from particle %d m:%f d:%f\n", i, particles[1].neighbors[i]->pos.m, particles[1].neighbors[i]->pos.d);
		*/
}
