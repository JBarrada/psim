#include <math.h>
#include <time.h>
#include <stdio.h>
#include "config.h"
#include "gfx.c"
#include "otypes.h"

typedef struct node {
	double radius;
	double mass;
	vector pos;
	vector vel;
	vector vel_;
} node;

typedef struct spring {
	double d; // resting distance
	double k; // stiffness
	double b; // damping
	
	node* node_a;
	node* node_b;
} spring;

node nodes[NODES];
spring springs[SPRINGS];

vector gravity = (vector) {0.1, -M_PI/2};

void draw() {
	int i;
	for (i=0; i<NODES; i++) {
		point node_xy = cast_point(&nodes[i].pos);
		circle(node_xy.x*W_G, node_xy.y*W_G, nodes[i].radius*W_G, 0x0f);
	}
	
	for (i=0; i<SPRINGS; i++) {
		point node_a_xy = cast_point(&springs[i].node_a->pos);
		point node_b_xy = cast_point(&springs[i].node_b->pos);
		line(node_a_xy.x*W_G, node_a_xy.y*W_G, node_b_xy.x*W_G, node_b_xy.y*W_G, 0x0f);
	}
}

void external_forces() {
	int i;
	for (i=0; i<NODES; i++) {
		vector node_g_force = vsmult(&gravity, nodes[i].mass);
		nodes[i].vel = vadd(&nodes[i].vel, &node_g_force);
	}
}

void spring_forces() {
	int i; 
	for (i=0; i<SPRINGS; i++) {
		vector displacement = vsub(&springs[i].node_b->pos, &springs[i].node_a->pos);
		
		vector relative_vel = vsub(&springs[i].node_b->vel_, &springs[i].node_a->vel_);
		vector damping = vsmult(&relative_vel, springs[i].b);
		
		vector force = vsmult(&(vector){1, displacement.d}, -springs[i].k*(displacement.m-springs[i].d));
		force = vsub(&force, &damping);
		
		// b node acceleration
		vector b_force = (vector){force.m/springs[i].node_b->mass, force.d};
		springs[i].node_b->vel = vadd(&springs[i].node_b->vel, &b_force);
		
		// a node acceleration
		vector a_force = (vector){force.m/springs[i].node_a->mass, force.d+M_PI};
		springs[i].node_a->vel = vadd(&springs[i].node_a->vel, &a_force);
	}
}

void resolve_collisions() {
	int i;
	
	//double friction = 0.9;
	
	vector normal = (vector) {1, M_PI/2};
	point node_xy;
	for (i=0; i<NODES; i++) {
		node_xy = cast_point(&nodes[i].pos);
		double distance = node_xy.y;
		
		if (distance < nodes[i].radius) {
			double projection = vvmult(&nodes[i].vel, &normal);
			vector scaled = vsmult(&normal, 2*projection);
			vector reflection = vsub(&nodes[i].vel, &scaled);
			
			//reflection.m *= friction;
			
			nodes[i].vel = reflection;
		}
	}
}

void update_positioning() {
	int i;
	for (i=0; i<NODES; i++) {
		nodes[i].vel.m *= 0.9; // friction 
		nodes[i].pos = vadd(&nodes[i].pos, &nodes[i].vel);
		nodes[i].vel_ = nodes[i].vel;
	}
}


void idle() {
	//external_forces();
	spring_forces();
	//resolve_collisions();
	update_positioning();
	
	render();
	usleep(40000);
}

void soft_body_circle(double cx, double cy, double r, int n) {
	nodes[0] = (node){1, 1, cast_vector(&(point){cx,cy}), (vector){0,0}, (vector){0,0}};

	int i, s=0;
	double step = (2*M_PI)/(n-1);

	for (i=0; i<(n-1); i++) {
		vector nodepos = cast_vector(&(point){cx+r*cos(step*i), cy+r*sin(step*i)});
		
		nodes[i+1] = (node){1, 1, nodepos, (vector){0,0}, (vector){0,0}};
	}
	
	for (i=1; i<n; i++) {
		springs[s].node_a = &nodes[0];
		springs[s].node_b = &nodes[i];
		springs[s].d = r;
		s++;
		
		springs[s].node_a = &nodes[i];
		springs[s].node_b = &nodes[1+(i%(n-1))];
		springs[s].d = sqrt(pow((r*cos(step))-r,2) + pow((r*sin(step)),2));
		s++;
		
		springs[s].k = 0.5;
		springs[s].b = 0.1;
		springs[s-1].k = 0.5;
		springs[s-1].b = 0.1;
	}
}

void main() {
	soft_body_circle(32, 24, 6, NODES);
	
	init(idle, draw);
}