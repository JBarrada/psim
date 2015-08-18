
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
