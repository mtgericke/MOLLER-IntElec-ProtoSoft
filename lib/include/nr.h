#ifndef _NR_H_
#define _NR_H_


#include <stdio.h>


/*Changed float -> double && int -> long int*/
void odeint(double ystart[], long int nvar, double x1, double x2,
	double eps, double h1, double hmin, long int *nok, long int *nbad,
	void (*derivs)(double, double [], double []),
	void (*rkqs)(double [], double [], long int, double *, double, double,
	double [], double *, double *, void (*)(double, double [], double [])));
/*------------------------------------------*/


/*Changed float -> double && int -> long int*/
void rk4(double y[], double dydx[], long int n, double x, double h, double yout[],
	void (*derivs)(double, double [], double []));
void rkck(double y[], double dydx[], long int n, double x, double h,
	double yout[], double yerr[], void (*derivs)(double, double [], double []));
void rkdumb(double vstart[], long int nvar, double x1, double x2, long int nstep,
	void (*derivs)(double, double [], double []));
void rkqs(double y[], double dydx[], long int n, double *x,
	double htry, double eps, double yscal[], double *hdid, double *hnext,
	void (*derivs)(double, double [], double []));
/*------------------------------------------*/

#endif /* _NR_H_ */
