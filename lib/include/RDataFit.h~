/////////////////////////////////////////////////////////////////////////////////////
//Root Based Data Visualization and Analysis Utility Library
//
//Version 000
//
//RDataFit header file, declaring basic methods to fit data
/////////////////////////////////////////////////////////////////////////////////////


#ifndef RDATAFIT_H
#define RDATAFIT_H

#include <stdlib.h>
#include <math.h>
#include <TVirtualX.h>
#include <TGClient.h>
#include <TGFrame.h>
#include <TString.h>
#include <TSystem.h>
#include <TGWidget.h>
#include <TKey.h>
//#include <TMath.h>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TGraphAsymmErrors.h>
#include <TMultiGraph.h>
#include <TF1.h>
#include <TH1S.h>
#include <TH1F.h>
#include <TH1D.h>
#include <TH2S.h>
#include <TH2F.h>
#include <TH2D.h>
#include <TH3S.h>
#include <TH3F.h>
#include <TH3D.h>
#include "RDataFitEntry.h"
#include "RDataFitDef.h"

class RDataFit : public TQObject {

private:

  Double_t             dChisq;
  Double_t             dChisqdeg;
  Double_t             dChisqlowbnd;
  Double_t             dParms[10];
  Double_t             dSteps[10];
  Double_t             dParmerrors[10];
  Double_t             dLFitrange[3];
  Double_t             dHFitrange[3];
  Int_t                dNumpars;
  Int_t                dNumbins[3];     //Number of bins in data
  Int_t                dDegfree;
  Int_t                dRNumbins[3];    //Number of bins in the fit range
  Int_t                dStartbin[3];    //Start and stop bins for fit range
  Int_t                dStopbin[3];
  Int_t                dDim;
  Int_t                dEntries;

  Int_t                dFitcnt;
  Int_t                dMsgcnt;        //Number of messages in queue
  char                 dMainName[NAME_STR_MAX]; 
  char                 dReceiverName[NAME_STR_MAX]; 
  char                 dObjName[NAME_STR_MAX]; 
  char                 dMiscbuffer[MSG_SIZE_MAX]; //For random use
  char                 dMiscbuffer2[MSG_SIZE_MAX];
  char                 dDatatitle[NAME_STR_MAX];
  char                 dFittitle[NAME_STR_MAX];
  char                 dMessage[MSG_QUEUE_MAX][MSG_SIZE_MAX]; //Message in queue
  char                 dFitlog[1000000];
  FitOptions          *dFitOptions;
  ERFitLogOutput       dFitlogoutput;
  RDataFitEntry       *dFitEntry;
  Bool_t               dFitOptionsFlag;
  Bool_t               dModify;

  const TGWindow      *dMain;
  const TGWindow      *dParent;
  TGWindow            *fMrec;
  TF1                 *fFitFunc;
  //*****************************************************************
  TGraph              *fGraph;      
  TGraphErrors        *fGraphEr;
  TGraphAsymmErrors   *fGraphAsymEr;

  TH1S                *fS1Histo;    //These are the root objects
  TH1F                *fF1Histo;    //that users define through
  TH1D                *fD1Histo;    //the various DrawData() routines

  TH2S                *fS2Histo;
  TH2F                *fF2Histo;
  TH2D                *fD2Histo;

  TH3S                *fS3Histo;
  TH3F                *fF3Histo;
  TH3D                *fD3Histo;     

  TMultiGraph         *fMGraph;     
  //****************************************************************

  Bool_t               SetMessage(const char *msg, 
				  const char *func,
				  Int_t TS, 
				  Int_t MESSAGETYPE); //Add message to queue
  void                 FlushMessages();//Get rid of all messages in queue
  Double_t             minv(Double_t *x, Int_t size);
  Double_t             maxv(Double_t *x, Int_t size);
/*   Double_t             MinimizeChisq(Double_t(*fitfunc)(Double_t*,Double_t*),  */
/* 				     Int_t dim, Double_t *x, Double_t *y,  */
/* 				     Double_t *ye, Int_t start, Int_t stop,  */
/* 				     Double_t *parm, Int_t parind, */
/* 				     Double_t *stepsize, Double_t bnd,  */
/* 				     Int_t *dgf); */
public:
  RDataFit(const TGWindow *p, const TGWindow *main, 
	   char *objName, char *mainanme,
	   char *datatitle);
  virtual ~RDataFit();

  FitOptions     *SetGetFitOptionsUser();
  FitOptions     *GetFitOptions() {return dFitOptions;}
  void            SetFitOptions(FitOptions *opts){dFitOptions=opts;dModify=kTrue;}
  void            SetDefaultFitOpts();
  void            SetDimension(Int_t dim = 1){dDim = dim;};
  void            SetFitRange(Bool_t full=kTrue,Double_t*min=0,Double_t*max=0);
  void            SetFitType(Bool_t ROOTF=kTrue){dFitOptions->rootFit = ROOTF;};
  void            SetFitFunc(const char *func="gaus"){strcpy(dFitOptions->func,func);};
  void            SetFitParameters(Double_t*,Int_t);
  void            SelectPlotToFit(Int_t plot=1){dFitOptions->plot = plot;};
  void            SetInitialFitStepSize(Double_t *step, Int_t);
  void            SetChiSqPrecision(Double_t prec = 0.0001);
  char           *GetDataTitle() {return dDatatitle;};
  char           *GetFitTitle() {return dFittitle;};
  char           *GetMessage();
  char           *GetFitLog() {return dFitlog;};
  void            SetOutputType(ERFitLogOutput outp){dFitlogoutput = outp;};
  void            SetFitTitle(char *name) {strcpy(dFittitle,name);};
  void            SetLineColor(Color_t col) {dFitOptions->lineColor = col;};
  void            SetLineStyle(Style_t sty) {dFitOptions->lineStyle = sty;};
  void            SetLineWidth(Width_t wid) {dFitOptions->lineWidth = wid;};
  void            SetMarkerColor(Color_t col){dFitOptions->markerColor= col;};
  void            SetMarkerStyle(Style_t sty){dFitOptions->markerStyle= sty;};
  void            SetMarkerSize(Size_t sze) {dFitOptions->markerSize = sze;};  
/*   Int_t            ChiSquared(Double_t(*fitfunc)(Double_t*,Double_t*), */
/* 			     Double_t *x, Double_t *y, Double_t *ye, */
/* 			     Double_t *parm,Int_t start, Int_t stop, */
/* 			     Double_t *chi2, Int_t *degfree); */
/*   Int_t           FitUser(double(*fitfunc)(double*,double*),Int_t size,  */
/* 			 Double_t *param, Int_t pars, Int_t start,  */
/* 			 Int_t stop, Double_t bound, Int_t dim, */
/* 			 Double_t *binv, Double_t *wgt, Double_t *wgte); */
  Int_t           FitRoot(TH1D *);
  Int_t           FitRoot(TGraphErrors *, TF1 *);
  virtual void    CloseWindow();
  virtual void    GetObj(TObject* obj);
  virtual void    PutFitOptions(char* opts);
  void            SendMessageSignal(char*);//Emit signal when message is posted
  void            IsClosing(char *); //Emit signal that this object is closing
  void            ConnectWith(const TGWindow *, char *); 
  void            CloseConnection(); 
  static Double_t ACosPlusBSin(Double_t *x, Double_t *parm);
  static Double_t ACosPlusBSinPlusC(Double_t *x, Double_t *parm);
  static Double_t Gaussian(Double_t *x, Double_t *parm);
  static Double_t OneMinusCos(Double_t *x, Double_t *parm);


  ClassDef(RDataFit,0);
};

#endif 
