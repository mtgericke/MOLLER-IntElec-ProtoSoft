/////////////////////////////////////////////////////////////////////////////////////
//Root Based Data Visualization and Analysis Utility Library
//
//Version 000
//
//RVegas source file, containing methods to calculate integrals via the Monte
//Technique.
/////////////////////////////////////////////////////////////////////////////////////

#ifndef RVEGAS_H
#define RVEGAS_H

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <RVegasDef.h>

class RVegas {

private:

  long idum2;
  long iy;
  long iv[NTAB];

  int i,it,j,k,mds,nd,ndo,ng,npg,ia[MXDIM+1],kg[MXDIM+1];
  double calls,dv2g,dxg,f,f2,f2b,fb,rc,ti,tsi,wgt,xjac,xn,xnd,xo;
  double d[NDMX+1][MXDIM+1],di[NDMX+1][MXDIM+1],dt[MXDIM+1];
  double dx[MXDIM+1], r[NDMX+1],x[MXDIM+1],xi[MXDIM+1][NDMX+1],xin[NDMX+1];
  double schi,si,swgt;

  int            dDim;
  long           dIdum;
  double         c;
  double        *dIntlimits;   //integration limits                             
  int            dInit;	       //initialization for VEGAS routine, see NUM. REC.
  unsigned long  dEvals;       //number of function evaluations                 
  unsigned long  dSeed;                 
  int            dItmax;       //max number of VEGAS iterations                 
  int            dNprn;	       //VEGAS diagnostic output, zero is normal operation
  double         dResult;      //result of the integration                      
  double         dSigma;       //standard deviation of the result               
  double         dChsqdf;      //chi squared per degree of freedom of the result

  void           vegas(double regn[], int ndim, double (*fxn)(double [], double), 
		       int init, unsigned long ncall, int itmx, int nprn, 
		       double *tgral, double *sd,double *chi2a);
  
  void           rebin(double rc, int nd, double r[], double xin[], double xi[]);
  
  double         ran2(long *idum);

  double         SQR(double arg) {return arg == 0.0 ? 0.0 : arg*arg;};
  int            IMAX(int a, int b) {return a > b ? a : b;};
  int            IMIN(int a, int b) {return a < b ? a : b;};

public:
  RVegas(int nprn);
  virtual ~RVegas();
  void           SetDimensions(int dim) {dDim = dim;};
  void           SetItmax(int itmax) {dItmax = itmax;};
  void           SetSeed(unsigned long s) {dSeed = s;};
  void           SetMaxNumOfEvals(unsigned long nevls) {dEvals = nevls;};
  void           Integrate(double (*integrand)(double [], double),double*);
  double         GetResult(){return dResult;};
  double         GetError(){return dSigma;};
  double         GetChiSq(){return dChsqdf;};
  
};

#endif 
