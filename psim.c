#include "otypes.h"

typedef struct particle {
	vector pos;
	vector pos_prev;
	vector vel;
	int index;
}

const float radius = 1.0; // max distance particles affect each other
const float c_radius = 2.0; // distance from wall that counts as collision
const float r_density = 0.5; // rest density
const float l_d_velocity = 0.1; // viscosity's linear dependence on velocity
const float q_d_velocity = 0.1; // viscosity's quadratic dependence on velocity
const float k = 0.1; // stiffness used in double_density_relax
const float k_near = 0.1; // near-stiffness used in double_density_relax
const vector gravity = {1, (3*M_PI)/2}

void main() {
	
}