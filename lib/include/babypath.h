#ifndef BABYPATH_H
#define BABYPATH_H

void vegas(double regn[], int ndim, double (*fxn)(double [], double), int init,
	unsigned long ncall, int itmx, int nprn, double *tgral, double *sd,
	   double *chi2a);


#endif