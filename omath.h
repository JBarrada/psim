#define T0(x) ( 1 )
#define T1(x) ( x )
#define T2(x) ( 2 * x*x - 1 )
#define T3(x) ( 4 * x*x*x - 3 * x )
#define T4(x) ( 8 * x*x*x*x - 8 * x*x + 1 )
#define T5(x) ( 16 * x*x*x*x*x - 20 * x*x*x + 5 * x )
 
#define C0 1.276278962f
#define C1 -.285261569f
#define C2 0.009118016f
#define C3 -.000136587f
#define C4 0.000001185f
#define C5 -.000000007f
 
#define P(z) ( C0 * T0(z) + C1 * T1(z) + C2 * T2(z) + C3 * T3(z) + C4 * T4(z) + C5 * T5(z) )

#define M_PI 	3.14159265358979323846
#define M_PId2 	1.57079632679489661923
#define M_PIt2	6.28318530717958647692

float sin(float x) {
	x /= M_PIt2;
	double w = 4 * x;
	double z = 2 * w * w - 1;

	return P(z) * w;
}

float cos(float x) {
	return sin(M_PId2-x);
}

float atan(float x) {
	return (sin(M_PId2-x) / sin(x));
}

float sqrt(float n) {
  float x = n;
  float y = 1;
  float e = 0.000001;
  while(x - y > e) {
    x = (x + y)/2;
    y = n/x;
  }
  return x;
}
