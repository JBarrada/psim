//#include "omath.h"
#include <math.h>

typedef struct vector {
	float m, d;
} vector;

typedef struct point {
	float x, y;
} point;

point cast_point(vector *a) {
	return (point){a->m*cos(a->d), a->m*sin(a->d)};
}

vector cast_vector(point *a) {
	return (vector){sqrt((a->x*a->x)+(a->y*a->y)), atan(a->y/a->x)};
}

vector vadd(vector *a, vector *b) {
	point p_a = cast_point(a);
	point p_b = cast_point(b);
	p_a.x += p_b.x;
	p_a.y += p_b.y;
	return cast_vector(&p_a);
}

vector vsub(vector *a, vector *b) {
	point p_a = cast_point(a);
	point p_b = cast_point(b);
	p_a.x -= p_b.x;
	p_a.y -= p_b.y;
	return cast_vector(&p_a);
}

float vvmult(vector *a, vector *b) { // might need to find greater direction
	return (a->m*b->m*cos(a->d-b->d));
}

vector vsmult(vector *v, float s) {
	return (vector){v->m*s, v->d};
}