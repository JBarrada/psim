#include <math.h>
#include <time.h>
#include <stdio.h>
#include "config.h"
#include "gfx.c"
#include "otypes.h"

#define NUMOBJS 4

typedef struct bb_rect {
	double width;
	double height;
} bb_rect;

typedef struct bb_circle {
	double radius;
} bb_circle;

typedef struct object {
	vector pos;
	vector vel;
	double mass;
	double inv_mass;
	double restitution;
	
	double sf;
	double df;
	
	double m_inertia;
	double orientation;
	double angular_vel;
	double torque;
	
	int skintype;
	bb_rect skin_rect;
	bb_circle skin_circle;
} object;

typedef struct manifold {
	object *a;
	object *b;
	double pen;
	vector normal;
	
	vector a_contact;
	vector b_contact;
} manifold;

int circle_v_circle(manifold *m) {
	vector n = vsub(&m->b->pos, &m->a->pos);
	double r = (m->a->skin_circle.radius + m->b->skin_circle.radius);
	
	if (n.m*n.m > r*r)
		return 0;

	if (n.m != 0) {
		m->a_contact = (vector){m->a->skin_circle.radius, n.d};
		m->b_contact = (vector){-m->b->skin_circle.radius, n.d};
		
		m->pen = r - n.m;
		m->normal = (vector) {1, n.d};
	}
	else {
		m->pen = m->a->skin_circle.radius;
		m->normal = (vector) {1, 0};
	}
	return 1;
}

int rect_v_rect(manifold *m) {
	vector n = vsub(&m->b->pos, &m->a->pos);
	point n_xy = cast_point(&n);
	double x_ovrlp = (m->a->skin_rect.width/2) + (m->b->skin_rect.width/2) - abs(n_xy.x);
	
	if (x_ovrlp > 0) {
		double y_ovrlp = (m->a->skin_rect.height/2) + (m->b->skin_rect.height/2) - abs(n_xy.y);
		
		if (y_ovrlp > 0) {
			
			if (x_ovrlp > y_ovrlp) {
				if (n_xy.x < 0)
					m->normal = (vector) {1, M_PI};
				else
					m->normal = (vector) {1, 0};
				
				m->pen = x_ovrlp;
				return 1;
			}
			else {
				if (n_xy.y < 0)
					m->normal = (vector) {1, -M_PI/2};
				else
					m->normal = (vector) {1, M_PI/2};
				
				m->pen = y_ovrlp;
				return 1;
			}
		}
	}
	return 0;
}

double clamp(double min, double max, double value) {
	if (value > max)
		value = max;
	
	if (value < min)
		value = min;
	
	return value;
}

int rect_v_circle(manifold *m) {
	vector n = vsub(&m->b->pos, &m->a->pos);
	point n_xy = cast_point(&n);
	point closest = (point) {n_xy.x, n_xy.y};
	closest.x = clamp(-m->a->skin_rect.width/2, m->a->skin_rect.width/2, closest.x);
	closest.y = clamp(-m->a->skin_rect.height/2, m->a->skin_rect.height/2, closest.y);
	
	int inside = 0;
	
	if (n_xy.x == closest.x & n_xy.y == closest.y) {
		inside = 1;
		
		if (abs(n_xy.x) > abs(n_xy.y)) {
			if(closest.x > 0)
				closest.x = m->a->skin_rect.width/2;
			else
				closest.x = -m->a->skin_rect.width/2;
		}
		else {
			if(closest.y > 0)
				closest.y = m->a->skin_rect.height/2;
			else
				closest.y = -m->a->skin_rect.height/2;
		}
	}
	
	vector closest_v = cast_vector(&closest);
	vector normal = vsub(&n, &closest_v);
	double d = normal.m;
	double r = m->b->skin_circle.radius;
	
	if ((d*d) > (r*r) & inside == 0)
		return 0;
	
	if (inside) {
		m->normal = (vector) {-1, normal.d};
		m->pen = r - d;
	}
	else {
		m->a_contact = (vector){n.m - m->b->skin_circle.radius, n.d};
		m->b_contact = (vector){-m->b->skin_circle.radius, n.d};
		
		m->normal = (vector) {1, normal.d};
		m->pen = r - d;
	}
	return 1;
}

void positional_correction(manifold *m) {
	const double percent = 0.2;
	const double slop = 0.01;
	double cs = ((m->pen - slop) < 0.0)? 0.0 : (m->pen - slop);
	vector correction = vsmult(&m->normal, (cs / (m->a->inv_mass + m->b->inv_mass)) * percent);
	
	m->a->pos = vsub(&m->a->pos, &(vector){correction.m*m->a->inv_mass, correction.d});
	m->b->pos = vadd(&m->b->pos, &(vector){correction.m*m->b->inv_mass, correction.d});
}

void resolve_friction(manifold *m) {
	vector rv = vsub(&m->b->vel, &m->a->vel);
	
	vector dotnormal = vsmult(&m->normal, vvmult(&rv, &m->normal));
	vector tangent = vsub(&rv, &dotnormal);
	tangent.m /= tangent.m;
	
	double jt = -vvmult(&rv, &tangent);
	
	double contact_a_t = vvmult(&m->a_contact, &tangent);
	contact_a_t *= contact_a_t;
	contact_a_t /= m->a->m_inertia;
	double contact_b_t = vvmult(&m->b_contact, &tangent);
	contact_b_t *= contact_b_t;
	contact_b_t /= m->b->m_inertia;
	
	if (m->a->m_inertia == 0) {
		contact_a_t = 0;
	}
	if (m->b->m_inertia == 0) {
		contact_b_t = 0;
	}
	
	//jt /= (m->a->inv_mass + m->b->inv_mass);
	jt /= (m->a->inv_mass + m->b->inv_mass + contact_a_t + contact_b_t);
	//printf("%f\n", contact_b_t);
	
	double mu = sqrt((m->a->sf*m->a->sf) + (m->b->sf*m->b->sf));
	
	vector friction_impulse;
	if (abs(jt) < (jt*mu)) {
		friction_impulse = vsmult(&tangent, jt);
	}
	else {
		double dynamic_friction = sqrt((m->a->df*m->a->df) + (m->b->df*m->b->df));
		friction_impulse = vsmult(&tangent, -jt*dynamic_friction);
	}
	
	vector a_accel = vsmult(&friction_impulse, m->a->inv_mass);
	vector b_accel = vsmult(&friction_impulse, m->b->inv_mass);

	m->a->vel = vadd(&m->a->vel, &a_accel);
	m->b->vel = vsub(&m->b->vel, &b_accel);
	
	m->a->angular_vel += (1 / m->a->m_inertia) * vvmult(&a_accel, &m->a_contact);
	m->b->angular_vel += (1 / m->b->m_inertia) * vvmult(&b_accel, &m->b_contact);
}

void resolve_collision(manifold *m) {
	positional_correction(m);
	
	vector rv = vsub(&m->b->vel, &m->a->vel);
	
	double normal_vel = vvmult(&rv, &m->normal);
	
	if (normal_vel <= 0) {
		double e = (m->a->restitution < m->b->restitution)? m->a->restitution : m->b->restitution;
		double j = -(1 + e) * normal_vel;
		j /= (m->a->inv_mass + m->b->inv_mass);
		
		vector impulse = vsmult(&m->normal, j);
		vector a_accel = vsmult(&impulse, m->a->inv_mass);
		vector b_accel = vsmult(&impulse, m->b->inv_mass);
		
		m->a->vel = vsub(&m->a->vel, &a_accel);
		m->b->vel = vadd(&m->b->vel, &b_accel);
		
		resolve_friction(m);
		
		//m->a->angular_vel += (1 / m->a->m_inertia) * vvmult(&a_accel, &m->a_contact);
		//m->b->angular_vel += (1 / m->b->m_inertia) * vvmult(&b_accel, &m->b_contact);
		
		//m->a->vel.m *= 0.99;
		//m->b->vel.m *= 0.99;
	}
}


object objects[NUMOBJS];

vector gravity = (vector) {0.01, -M_PI/2};

void external_forces() {
	int i;
	for (i=0; i<NUMOBJS; i++) {
		objects[i].vel = vadd(&objects[i].vel, &(vector){gravity.m*objects[i].mass, gravity.d});
		
		objects[i].orientation += objects[i].angular_vel;
	}
}

void generate_manifolds() {
	int i, j;
	for (i=0; i<NUMOBJS; i++) {
		for (j=i; j<NUMOBJS; j++) {
			if (i != j) {
				manifold m;
				int collision;
				switch (objects[i].skintype + objects[j].skintype) {
					case 0:
						m.a = &objects[i];
						m.b = &objects[j];
						collision = rect_v_rect(&m);
						break;
					case 1:
						if (objects[i].skintype == 0){
							m.a = &objects[i];
							m.b = &objects[j];
						}
						else {
							m.a = &objects[j];
							m.b = &objects[i];
						}
						collision = rect_v_circle(&m);
						break;
					case 2:
						m.a = &objects[i];
						m.b = &objects[j];
						collision = circle_v_circle(&m);
						break;
				}
				if (collision) {
					resolve_collision(&m);
				}
			}
		}
	}
}

void advance() {
	int i;
	for (i=0; i<NUMOBJS; i++) {
		objects[i].vel.m *= 0.999;
		objects[i].pos = vadd(&objects[i].pos, &objects[i].vel);
	}
}

void draw() {
	int i;
	for (i=0; i<NUMOBJS; i++) {
		point objxy = cast_point(&objects[i].pos);
		
		if (objects[i].skintype == 0) { // rect skin
			double left = objxy.x-(objects[i].skin_rect.width/2);
			double right = objxy.x+(objects[i].skin_rect.width/2);
			double up = objxy.y+(objects[i].skin_rect.height/2);
			double down = objxy.y-(objects[i].skin_rect.height/2);
			
			left *= W_G;
			right *= W_G;
			up *= W_G;
			down *= W_G;
			
			line(left, up, right, up, 0xff);
			line(left, down, right, down, 0xff);
			line(left, up, left, down, 0xff);
			line(right, up, right, down, 0xff);
		}
		
		if (objects[i].skintype == 1) {
			circle(objxy.x*W_G, objxy.y*W_G, objects[i].skin_circle.radius*W_G, 0xff);
			point orient = cast_point(&(vector){objects[i].skin_circle.radius*W_G, objects[i].orientation});
			line(objxy.x*W_G, objxy.y*W_G, objxy.x*W_G+orient.x, objxy.y*W_G+orient.y, 0xff);
		}
	}
}

void idle() {
	external_forces();
	generate_manifolds();
	advance();
	
	render();
	usleep(10000);
}

void main() {
	objects[0].pos = cast_vector(&(point){32, 2});
	objects[0].mass = 0;
	objects[0].inv_mass = 0;
	objects[0].restitution = 0.5;
	objects[0].skintype = 0;
	objects[0].skin_rect.width = 64;
	objects[0].skin_rect.height = 4;
	objects[0].sf = 0.01;
	objects[0].df = 0.01;
	
	objects[1].pos = cast_vector(&(point){2, 24+4.1});
	objects[1].mass = 0;
	objects[1].inv_mass = 0;
	objects[1].restitution = 0.5;
	objects[1].skintype = 0;
	objects[1].skin_rect.width = 4;
	objects[1].skin_rect.height = 48;
	objects[1].sf = 0.01;
	objects[1].df = 0.01;
	
	objects[2].pos = cast_vector(&(point){62, 24+4.1});
	objects[2].mass = 0;
	objects[2].inv_mass = 0;
	objects[2].restitution = 0.5;
	objects[2].skintype = 0;
	objects[2].skin_rect.width = 4;
	objects[2].skin_rect.height = 48;
	objects[2].sf = 0.01;
	objects[2].df = 0.01;
	
	
	int i;
	for (i=3; i<NUMOBJS; i++) {
		objects[i].pos = cast_vector(&(point){32, 24});
		objects[i].vel = (vector){.1, -3*M_PI/4};
		objects[i].mass = 1;
		objects[i].inv_mass = 1;
		objects[i].restitution = 0.5;
		objects[i].skintype = 1;
		objects[i].skin_circle.radius = 3;
		objects[i].m_inertia = objects[i].mass*(objects[i].skin_circle.radius*objects[i].skin_circle.radius);
		objects[i].sf = 0.01;
		objects[i].df = 0.01;
	}
	
	init(idle, draw);
}