#include <math.h>

typedef struct vector {
	double m, d;
} vector;

typedef struct point {
	double x, y;
} point;


point cast_point(vector *a) {
	return (point){a->m*cos(a->d), a->m*sin(a->d)};
}

vector cast_vector(point *a) {
	double angle=0;
	if (a->x < 0)
		angle = atan(a->y/a->x) + M_PI;
	else if (a->y!= 0 & a->x != 0){
		angle = atan(a->y/a->x);
	}
	
	return (vector){sqrt((a->x*a->x)+(a->y*a->y)), angle};
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

double vvmult(vector *a, vector *b) { // might need to find greater direction
	return (a->m*b->m*cos(a->d-b->d));
}

vector vsmult(vector *v, double s) {
	return (vector){v->m*s, v->d};
}