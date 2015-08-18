#include <GL/gl.h>
#include <GL/freeglut.h>
#include <GL/glext.h>
#include <time.h>

#include <stdio.h>
#include "otypes.h"

#define MAXP 200

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

const double radius = .4; // max distance particles affect each other

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
	
	double neighbor_effect = .4;
	double pressure_effect = .0004;
	
	for (i=0; i<MAXP; i++) {
		for (n=0; n<MAXP; n++) {
			if (particles[i].neighbors[n]) {
				vector from_neighbor = vsub(&particles[i].pos, &particles[i].neighbors[n]->pos);
				from_neighbor.m = (radius - from_neighbor.m)/radius;
				
				//vector neighbor_vel = particles[i].neighbors[n]->vel_prev;
				
				//neighbor_vel.m *= (from_neighbor.m*pressure_effect);
				
				//vector final_vector = vadd(&from_neighbor, &neighbor_vel);
				
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
	resolve_collisions();
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
	//printf("x1:%d y1:%d x2:%d y2:%d\n", x1, y1, x2, y2);
	int dx, dy, dxabs, dyabs, sdx, sdy, x, y, px, py;
	dx = (x2-x1);
	dy = (y2-y1);
	dxabs=abs(dx);
	dyabs=abs(dy);
	
	if (dyabs < 1000 & dxabs < 1000) {
		sdx=(dx>=0)?1:-1;
		sdy=(dy>=0)?1:-1;
		x=dyabs>>1;
		y=dxabs>>1;
		px=x1;
		py=y1;
		
		int i;
		if (dxabs>=dyabs) {
			for(i=0;i<dxabs;i++) {
				y+=dyabs;
				if (y>=dxabs) {
					y-=dxabs;
					py+=sdy;
				}
				px+=sdx;
				set_pixel(px,py,c);
			}
		}
		else {
			for(i=0;i<dyabs;i++) {
				x+=dxabs;
				if (x>=dyabs) {
					x-=dyabs;
					px+=sdx;
				}
				py+=sdy;
				set_pixel(px,py,c);
			}
		}
	}
}

void draw_circle(int cx, int cy, double r, unsigned char c) {
	int i;
	double x, y;
	for (i=0; i<40; i++) {
		x = r * cos(((M_PI/2)/40)*i);
		y = r * sin(((M_PI/2)/40)*i);
		
		set_pixel(cx+x, cy+y, c);
		set_pixel(cx-x, cy+y, c);
		set_pixel(cx+x, cy-y, c);
		set_pixel(cx-x, cy-y, c);
	}
}

void solid() {
	double cx = 0, cy = 0;
	
	int i;
	point part_pos;
	for (i=0; i<MAXP; i++) {
		part_pos = cast_point(&particles[i].pos);
		cx += part_pos.x;
		cy += part_pos.y;
	}

	cx /= MAXP;
	cy /= MAXP;
	
	part_pos.x = cx;
	part_pos.y = cy;
	

	/*
	vector center = cast_vector(&part_pos);
	vector difference;
	double avg_r = 0;
	for (i=0; i<MAXP; i++) {
		difference = vsub(&particles[i].pos, &center);
		avg_r += abs(difference.m);
	}
	avg_r /= MAXP;
	
	draw_circle(cx*20, cy*20, avg_r*20, 0x1c);
	*/
	
	

	vector center = cast_vector(&part_pos);
	vector difference;
	int a;
	double current_angle;
	
	int x1=0, y1=0;
	
	int res = 40;
	point points[res];
	
	for (a=0; a<res; a++) {
		current_angle = a * ((2*M_PI)/res);

		
		double avg_r = 0;
		//int particles_sector = 0;
		for (i=0; i<MAXP; i++) {
			difference = vsub(&particles[i].pos, &center);
			if (difference.d <0 & difference.m>0)
				difference.d += 2*M_PI;
			//if (difference.m<0)
			if ((difference.d-current_angle) < 2*((2*M_PI)/(res)) & (difference.d-current_angle)>=0) {
				if (difference.m > avg_r) {
					avg_r = difference.m;
				}
				//avg_r += difference.m;
				//particles_sector++;
			}
		}
		//avg_r /= particles_sector;
		difference.m = avg_r;
		difference.d = current_angle;
		
		vector avg_vect = vadd(&center, &difference);
		points[a] = cast_point(&avg_vect);
		
		//draw_line(x1, y1, avg_point.x*20, avg_point.y*20, 0x1c);
		//set_pixel(avg_point.x*20, avg_point.y*20, 0x1c);
		//x1 = avg_point.x*20;
		//y1 = avg_point.y*20;
	}
	
	for (a=0; a<res; a++) {
		if (a==0) {
			draw_line(points[res-1].x*20, points[res-1].y*20, points[a].x*20, points[a].y*20, 0x1c);
		}
		else {
			draw_line(points[a-1].x*20, points[a-1].y*20, points[a].x*20, points[a].y*20, 0x1c);
		}
	}
}

int orientation(point p, point q, point r) {
    int val = (q.y - p.y) * (r.x - q.x) -
              (q.x - p.x) * (r.y - q.y);
 
    if (val == 0) return 0;  // colinear
    return (val > 0)? 1: 2; // clock or counterclock wise
}

void bubble_sort(vector list_actual[], long n, vector list[]) {
	long c, d;
	vector temp;
 
	for (c = 0 ; c < ( n - 1 ); c++) {
		for (d = 0 ; d < n - c - 1; d++) {
			if (list[d].d > list[d+1].d) {
				temp = list[d];
				list[d]   = list[d+1];
				list[d+1] = temp;
				
				temp = list_actual[d];
				list_actual[d]   = list_actual[d+1];
				list_actual[d+1] = temp;
			}
		}
	}
}

void convex_hull(point points[], int n) {
	// Initialize Result
	int i;
    int next[n];
    for (i = 0; i < n; i++)
        next[i] = -1;
 
    // Find the leftmost point
    int l = 0;
    for (i = 1; i < n; i++)
        if (points[i].x < points[l].x)
            l = i;
 
    // Start from leftmost point, keep moving counterclockwise
    // until reach the start point again
    int p = l, q;
    do
    {
        // Search for a point 'q' such that orientation(p, i, q) is
        // counterclockwise for all points 'i'
        q = (p+1)%n;
        for (i = 0; i < n; i++)
          if (orientation(points[p], points[i], points[q]) == 2)
             q = i;
 
        next[p] = q;  // Add q to result as a next point of p
        p = q; // Set p as q for next iteration
    } while (p != l);
 
    // Print Result
	//int a=0;
	
    for (i = 0; i < n; i++)
    {
        if (next[i] != -1) {
			//printf("p x:%f y:%f\n", points[i].x, points[i].y);
			//draw_circle(points[i].x, points[i].y, 5, 0x1c);
		}
    }
	
	
	int n_result = 0;
	for (i = 0; i < n; i++)
        if (next[i] != -1)
           n_result++;
	
	
	point results[n_result];
	int a=0;
    for (i = 0; i < n; i++)
    {
        if (next[i] != -1) {
           results[a] = points[i];
		   a++;
		}
    }
	
	vector vectors[n_result];
	for (i=0;i<n_result;i++){
		vectors[i] = cast_vector(&results[i]);
	}
	
	
	//find centeer
	double cx = 0, cy = 0;
	
	point part_pos;
	for (i=0; i<n_result; i++) {
		cx += results[i].x;
		cy += results[i].y;
	}

	cx /= n_result;
	cy /= n_result;
	
	part_pos.x = cx;
	part_pos.y = cy;
	
	vector compare = cast_vector(&part_pos);
	
	
	vector vect_comp[n_result];
	for (i=0;i<n_result;i++){
		vect_comp[i] = vsub(&compare, &vectors[i]);
	}
	
	bubble_sort(vectors, n_result, vect_comp);
	
	point last = {0,0};
	for (i = 0; i < n_result; i++)
    {
		if (i!=0) {
			point ppp = cast_point(&vectors[i]);
			point lll = cast_point(&vectors[i-1]);
			draw_line(lll.x, lll.y, ppp.x, ppp.y, 0x1c);
		}
		else {
			point ppp = cast_point(&vectors[i]);
			point lll = cast_point(&vectors[n_result-1]);
			draw_line(lll.x, lll.y, ppp.x, ppp.y, 0x1c);
		}
	}
}

void draw_gl() {
	memset(screenbuf, 0, 400*400);
	
	point gravpoint = cast_point(&gravity);
	draw_line(200,200, 200+gravpoint.x*1000, 200+gravpoint.y*1000, 0xE0);
	
	point points[MAXP];
	int i;
	for (i=0; i<MAXP; i++) {
		point particle_pos = cast_point(&particles[i].pos);
		
		
		int x, y;
		x = particle_pos.x * (400.0/WIDTH);
		y = particle_pos.y * (400.0/HEIGHT);
		
		
		points[i]= (point){x, y};
		
		draw_circle(x, y, radius*(400.0/WIDTH)/2, 0x01);
	}
	
	
	convex_hull(points, MAXP);

	//solid();
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDrawPixels(400, 400, GL_RGB, GL_UNSIGNED_BYTE_3_3_2, screenbuf);
	glutSwapBuffers();
}

void test_function() {
	update();
	
	draw_gl();
	//usleep(10000);
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
		for (x=0; x<20; x++) {
			particles[i].pos = cast_vector(&(point){(x/4.0)+5, (y/4.0)+10});
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
