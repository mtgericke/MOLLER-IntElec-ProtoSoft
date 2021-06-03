#ifndef TRY1_H
#define TRY1_H

#include <stdio.h>
#include <math.h> 
#include <stdlib.h>
#include "nrutil.h"

#define PI 4.0*atan(1.0)
#define xa 1.0e4
#define tanxa tan(xa)
#define tan2xa tanxa*tanxa
#define xb 0.0
#define tanxb tan(xb)
#define tan2xb tanxb*tanxb
#define RAND_MAX 65536

#define NRANSI
#define ALPH 1.5
#define NDMX 50
#define MXDIM 10
#define TINY 1.0e-30

#define IM1 2147483563
#define IM2 2147483399
#define AM (1.0/IM1)
#define IMM1 (IM1-1)
#define IA1 40014
#define IA2 40692
#define IQ1 53668
#define IQ2 52774
#define IR1 12211
#define IR2 3791
#define NTAB 32
#define NDIV (1+IMM1/NTAB)
#define EPS 1.2e-7
#define RNMX (1.0-EPS)


extern long idum;

int DIM;
long idum;
//double EPS;
//double INV_EPS;
double c;

double integrand(double *y, double wgt);

void vegas(double regn[], int ndim, double (*fxn)(double [], double), int init,
	unsigned long ncall, int itmx, int nprn, double *tgral, double *sd,
	   double *chi2a);

void rebin(double rc, int nd, double r[], double xin[], double xi[]);

double ran2(long *idum);

#endif
