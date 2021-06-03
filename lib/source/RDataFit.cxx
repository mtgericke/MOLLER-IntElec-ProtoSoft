/////////////////////////////////////////////////////////////////////////////////////
//NPDGamma Experiment Data Visualization and Analysis
//
//Version 2.01
//
//RDataFit source file, containing methods to visualize data
//All methods specifying data visualization must be specified in
//the derived class.
/////////////////////////////////////////////////////////////////////////////////////


#include "RDataFit.h"

RDataFit::RDataFit(const TGWindow *p, const TGWindow *main,
		   char *objName, char *mainname,			 
		   char *datatitle)
  : TQObject()
{ 
  
  dFitcnt = 0;
  dFitlogoutput = FL_NORMAL;
  dFitEntry = NULL;
  dFitOptionsFlag = kFalse;
  dModify = kFalse;

  dFitOptions = new FitOptions;
  SetDefaultFitOpts();

  dMsgcnt = 0;
  fMrec = NULL;
  dMain = main;
  dParent = p;

  memset(dObjName,'\0',NAME_STR_MAX);
  strcpy(dObjName,objName);

  memset(dReceiverName,'\0',sizeof(dReceiverName));
  strcpy(dReceiverName,"");

  memset(dDatatitle,'\0',NAME_STR_MAX);
  strcpy(dDatatitle,datatitle);  

  memset(dFittitle,'\0',NAME_STR_MAX);
  strcpy(dFittitle,datatitle);  

  memset(dMainName,'\0',sizeof(dMainName));
  strcpy(dMainName,mainname);

  memset(dMiscbuffer,'\0',sizeof(dMiscbuffer));
  memset(dMiscbuffer2,'\0',sizeof(dMiscbuffer2));
  for(int i = 0; i < MSG_QUEUE_MAX; i++)
    memset(dMessage[i],'\0',sizeof(dMessage[i]));

  fGraph = NULL;      
  fGraphEr = NULL;    
  fGraphAsymEr = NULL;
  
  fS1Histo = NULL;    
  fF1Histo = NULL;    
  fD1Histo = NULL;    
  
  fS2Histo = NULL;    
  fF2Histo = NULL;    
  fD2Histo = NULL;    
  
  fS3Histo = NULL;    
  fF3Histo = NULL;    
  fD3Histo = NULL;    
  
  fMGraph  = NULL;       

  Connect("IsClosing(char*)",dMainName,
	  (void*)main,"OnObjClose(char*)");  
  Connect("SendMessageSignal(char*)",dMainName,(void*)main,
	  "OnReceiveMessage(char*)");
}

RDataFit::~RDataFit()
{
  // Delete dialog widgets.
  //everything that get dynamically allocated within this
  //class needs to be deleted here, unless it was
  //declared and allocated local to a function
  
  printf("In destructor RDataFit\n");
}

void RDataFit::IsClosing(char *objname)
{
  Emit("IsClosing(char*)",(long)objname);
}

void RDataFit::SendMessageSignal(char *objname)
{
  Emit("SendMessageSignal(char*)",(long)objname);
}

void RDataFit::ConnectWith(const TGWindow *rec, char *recname)
{
  if(fMrec == NULL && rec != NULL){
    fMrec = (TGWindow*)rec;
    strcpy(dReceiverName,recname);
  }
}

void RDataFit::CloseConnection()
{
  if(fMrec != NULL){
    fMrec = NULL;
    strcpy(dReceiverName,"");
  }
}


void RDataFit::FlushMessages()
{
  while(GetMessage() != NULL){}
}

char *RDataFit::GetMessage()
{
  if(dMsgcnt != 0)
    {
      strcpy(dMiscbuffer,dMessage[0]);
      for(int i = 0; i < dMsgcnt-1; i++)
	strcpy(dMessage[i],dMessage[i+1]);
      memset(dMessage[dMsgcnt-1],'\0',sizeof(dMessage[dMsgcnt-1]));
      dMsgcnt--;

      return dMiscbuffer;
    }

  return NULL;
}


Bool_t RDataFit::SetMessage(const char *msg, const char *func, int TS, int MESSAGETYPE)
{
#ifdef RDF_DEBUG
  char str1[NAME_STR_MAX];
  char str2[NAME_STR_MAX];

  if(MESSAGETYPE == M_DTFIT_ERROR_MSG){
    sprintf(str1,"Object: %s\n",GetDataTitle());
    sprintf(str2,"Function: %s\n",func);
  }
#endif

  if(dMsgcnt == MSG_QUEUE_MAX)
    {
      for(int i = 0; i < dMsgcnt-1; i++)
	strcpy(dMessage[i],dMessage[i+1]);
      memset(dMessage[MSG_QUEUE_MAX-1],'\0',sizeof(dMessage[MSG_QUEUE_MAX-1]));
      if(MESSAGETYPE == M_DTFIT_ERROR_MSG){ 
#ifdef RDF_DEBUG
	strcpy(dMessage[MSG_QUEUE_MAX-1],str1);
	strcat(dMessage[MSG_QUEUE_MAX-1],str2);
#endif
	strcat(dMessage[MSG_QUEUE_MAX-1],msg);
      }
      else
	strcpy(dMessage[MSG_QUEUE_MAX-1],msg);
    }
  else
    {
      if(MESSAGETYPE == M_DTFIT_ERROR_MSG){ 
#ifdef RDF_DEBUG
	strcpy(dMessage[dMsgcnt],str1);
	strcat(dMessage[dMsgcnt],str2);
#endif
	strcat(dMessage[dMsgcnt],msg);
      }
      else
	strcpy(dMessage[dMsgcnt],msg);
      dMsgcnt++;
    }
      
  SendMessageSignal(dObjName);
  gSystem->ProcessEvents();

  return kTrue;
}

void RDataFit::CloseWindow()
{
  // Called when window is closed via the window manager.

  //Calls Class Destructor
  IsClosing(dObjName);

  FlushMessages();
  SetMessage(FITFUNC_CLOSED_MSG,"",1,M_DTFIT_CLOSED);
  delete this;
}

void RDataFit::SetFitRange(Bool_t full, Double_t *min, Double_t *max)
{
  if(full){
    dFitOptions->fullRange = 1;
    for(int i = 0; i < dDim; i++){
      dFitOptions->min[i] = 0;
      dFitOptions->max[i] = 0;      
    }
  }
  else{
    for(int i = 0; i < dDim; i++){
      dFitOptions->min[i] = min[i];
      dFitOptions->max[i] = max[i];      
    }
  }
}

void RDataFit::SetFitParameters(Double_t *parms, Int_t npars)
{
  for(int i = 0; i < 10; i++){
    if(i < npars){
      dFitOptions->param[i]=parms[i];
      dFitOptions->parset[i] = 1;
    }
    else{
      dFitOptions->param[i]=0;
      dFitOptions->parset[i]=0;
    }
  }
  dFitOptions->npars = npars;
}

void RDataFit::SetInitialFitStepSize(Double_t *steps, Int_t npars)
{
  for(int i = 0; i < 10; i++){
    if(i < npars){
      dFitOptions->stepsize[i]=steps[i];
    }
    else{
      dFitOptions->stepsize[i]=0;
    }
  }
}

void RDataFit::SetChiSqPrecision(Double_t prec)
{
  dFitOptions->chisqBound = prec;
}

Double_t RDataFit::minv(Double_t *x, Int_t size)
{
  Double_t mini = x[0];

  for(int i = 0; i < size; i ++)
    {
      if(x[i] < mini) { mini = x[i];};
    }
      
  return mini;
}

void RDataFit::SetDefaultFitOpts()
{
  SetDimension();
  SetFitRange();
  SetFitType();
  SetFitFunc();
  SetFitParameters(NULL,0);
  SelectPlotToFit();
  SetInitialFitStepSize(NULL,0);
  SetChiSqPrecision();
  SetLineColor(kRed);
  SetLineWidth(1);
  SetLineStyle(1);
  SetMarkerColor(4);
  SetMarkerSize(0.8);
  SetMarkerStyle(20);
  dModify = kTrue;  
}

Double_t RDataFit::maxv(Double_t *x, Int_t size)
{
  Double_t maxi = x[0];

  for(int i = 0; i < size; i ++)
    {
      if(x[i] > maxi) { maxi = x[i];};
    }
      
  return maxi;
}

void RDataFit::GetObj(TObject* obj)
{

}

void RDataFit::PutFitOptions(char *opts)
{
  //printf("Got signal PutFitOptions\n");
  
  dFitOptionsFlag = kTrue;
}

Double_t RDataFit::Gaussian(Double_t *x, Double_t *parm)
{
  //Since this is a 1D fit function, only x[0] is used
  Double_t fitval = parm[0]*exp(-(x[0]-parm[1])*(x[0]-parm[1])/(2*parm[2]*parm[2]));
  return fitval;
}

Double_t RDataFit::ACosPlusBSin(Double_t *x, Double_t *parm)
{
  //Since this is a 1D fit function, only x[0] is used
  Double_t fitval = parm[0]*cos(x[0])+parm[1]*sin(x[0]);
  return fitval;
}

Double_t RDataFit::OneMinusCos(Double_t *x, Double_t *parm)
{
  //Since this is a 1D fit function, only x[0] is used
  Double_t fitval = parm[0]*(1-cos(parm[1]*x[0]));
  return fitval;
}

Double_t RDataFit::ACosPlusBSinPlusC(Double_t *x, Double_t *parm)
{
  //Since this is a 1D fit function, only x[0] is used
  Double_t fitval = parm[2]+parm[0]*cos(x[0])+parm[1]*sin(x[0]);
  return fitval;
}

FitOptions *RDataFit::SetGetFitOptionsUser()
{
  dFitEntry = new RDataFitEntry(dParent, dMain, this,
				"RDataFit",dFitOptions,dModify, 400, 200);

  if(dFitOptionsFlag){
    //printf("Did receive dFitOptionsFlag = kTrue;");
    dFitOptionsFlag = kFalse;
    return dFitOptions;
  }
  //printf("Did not receive dFitOptionsFlag = kTrue;");

  dFitOptionsFlag = kFalse;
  return NULL;
}

Int_t RDataFit::FitRoot(TH1D *hist)
{
  return 0;
}

Int_t RDataFit::FitRoot(TGraphErrors *gr, TF1 *f1)
{
  Double_t min = 0;
  Double_t max = 0;
  Double_t xpl = 0;
  Double_t ypl = 0;
  Double_t xph = 0;
  Double_t yph = 0;
  Int_t minbin = 0;
  Int_t maxbin = 0;
  Double_t parm[10]= {0,0,0,0,0,0,0,0,0,0};
  Int_t size       = 0;
  Double_t *binv   = NULL;
  Double_t *wgt    = NULL;
  Double_t *wgter  = NULL;
  Double_t *tmp1   = NULL;
  Double_t *tmp2   = NULL;
  Double_t *tmp3   = NULL;
  Double_t chisqb  = 0;

  printf("Line 369\n");
  if(f1 != NULL && gr != NULL){
    
    
    dFitcnt++;
    strcat(dFitlog,"\n**********************************\n");
    sprintf(dMiscbuffer,"Starting Fit For: %s :\n",GetFitTitle());
    strcat(dFitlog,dMiscbuffer);
    strcat(dFitlog,"\n**********************************\n");
    
    printf("Line 379\n");
    size  = gr->GetN();
    binv  = new Double_t[size];
    wgt   = new Double_t[size];
    wgter = new Double_t[size];	    
    
    tmp1 = gr->GetX();
    tmp2 = gr->GetY();
    tmp3 = gr->GetEY();
    
    for(int i = 0; i < gr->GetN(); i++){ 
      binv[i] = tmp1[i];
      wgt[i]  = tmp2[i];
      wgter[i]= tmp3[i];
    }

    printf("Line 395\n");
    
    if(dFitOptions->fullRange){
      minbin = 0;
      maxbin = gr->GetN()-1;
    }
    else{
      gr->GetPoint(0,xpl,ypl);
      gr->GetPoint(gr->GetN()-1,xph,yph);		
      
      if(dFitOptions->min[0] < xpl)
	minbin = 0;
      if(dFitOptions->max[0] > xph)
	maxbin = gr->GetN()-1;
      if(dFitOptions->max[0] < xpl ||
	 dFitOptions->min[0] > xph)
	return DATA_FIT_ERROR;
      
      for(int i = 0; i < gr->GetN()-1; i++){ 
	if(dFitOptions->min[0] > binv[i] && 
	   dFitOptions->min[0] < binv[i+1])
	  minbin = i;
	if(dFitOptions->max[0] > binv[i] && 
	   dFitOptions->max[0] < binv[i+1])
	  maxbin = i;
      }
      if(maxbin <= minbin)
	return DATA_FIT_ERROR;
    }
    
    gr->GetPoint(minbin,min,ypl);
    gr->GetPoint(maxbin,max,yph);
    
    if(!dFitOptions->parset[0]){
      dFitOptions->param[0] = max-min;
      dFitOptions->parset[0] = kTrue;	      
    }
    
    if(!dFitOptions->parset[1]){
      dFitOptions->param[1] = max-min;
      dFitOptions->parset[1] = kTrue;
    }
    
    parm[0] = dFitOptions->param[0];
    parm[1] = dFitOptions->param[1];
    
    chisqb = dFitOptions->chisqBound;
    f1->SetRange(min,max);
    
    dFitOptions->param[0] = parm[0];
    dFitOptions->param[1] = parm[1];
    dFitOptions->parset[0] = kTrue;
    dFitOptions->parset[1] = kTrue;
    
    f1->SetParameters(parm[0],parm[1]);
    
    gr->Fit(f1->GetName(),"RN");
  }
  return FIT_PROCESS_OK;
}



// Int_t RDataFit::FitUser(double(*fitfunc)(double*,double*),Int_t size, 
// 			Double_t *param, Int_t pars, Int_t start, Int_t stop, 
// 			Double_t bound, Int_t dim, Double_t *binv, Double_t *wgt, 
// 			Double_t *wgte)
// {
//   dim = 1;
//   Double_t lastchisq  = 5000;  
//   int i, n = 1;
//   Double_t chi1 = 0;
//   Double_t chi2 = 0;
//   Double_t chi3 = 0;
//   Double_t par1[10] = {0,0,0,0,0,0,0,0,0,0};
//   Int_t dgf = 0;

//   if(dim < 1) {
//     FlushMessages();
//     SetMessage(DIMENSION_ERROR,"FitUser(..)",2,M_DTFIT_ERROR_MSG);
//     return DATA_FIT_ERROR;
//   }
//   if(pars < 1 || pars > 10){
//     FlushMessages();
//     SetMessage(PARM_CNT_ERROR,"FitUser(..)",2,M_DTFIT_ERROR_MSG);
//     return DATA_FIT_ERROR;
//   }
  
//   dFitcnt++;
//   strcat(dFitlog,"\n**********************************\n");
//   sprintf(dMiscbuffer,"Starting Fit For: %s :\n",GetFitTitle());
//   strcat(dFitlog,dMiscbuffer);
//   strcat(dFitlog,"\n**********************************\n");

//   dEntries = 0;
//   dDegfree = 0;
//   dNumpars = pars;
//   dDim = dim;
//   dChisq = 0.0;
//   dChisqdeg = 0.0;
//   dChisqlowbnd = bound;

//   dLFitrange[0] = binv[start];
//   dHFitrange[0] = binv[stop];    
//   dNumbins[0] = size;
//   dStartbin[0] = start;
//   dStopbin[0] = stop;
//   dRNumbins[0] = 0;
  
//   dRNumbins[0] = 0;    
// //   for(i = 0; i < dNumbins[0]; i++){
// //     if(flag) {dRNumbins[0]++; dEntries += (Int_t)floor(wgt[i]);}
// //     if(binv[i] == dLFitrange[0]) {dStartbin[0] = i; flag = 1;}
// //     if(binv[i] == dHFitrange[0]) {dStopbin[0] = i; flag = 0;break;}
// //   }
//   for(i = start; i <= stop; i++)
//     {dRNumbins[0]++; dEntries += (Int_t)floor(sqrt(pow(wgt[i],2)));}
//   if(dEntries == 0) {dEntries = size;}
//   if(dEntries < dNumpars) {
//     FlushMessages();
//     SetMessage(RANGE_ERROR,"FitUser(..)",2,M_DTFIT_ERROR_MSG);
//     return DATA_FIT_ERROR;
//   }
  
//   if(dStartbin[0] == dStopbin[0] || dRNumbins[0] < 4){
//     FlushMessages();
//     SetMessage(RANGE_ERROR,"FitUser(..)",2,M_DTFIT_ERROR_MSG);
//     return DATA_FIT_ERROR;
//   }
  
//   for(i = 0; i < dNumpars; i++){
//     dParms[i] = param[i];
//   }

//   strcat(dFitlog,"\n\nInitial values:\n\n");
//   sprintf(dMiscbuffer,"Chi squared: %lf\n",dChisq);
//   strcat(dFitlog,dMiscbuffer);  
//   sprintf(dMiscbuffer,"Chi squared per degrees of freedom: %lf\n",dChisqdeg);
//   strcat(dFitlog,dMiscbuffer);
//   sprintf(dMiscbuffer,"Chisq lower bound: %lf\n",dChisqlowbnd);
//   strcat(dFitlog,dMiscbuffer);
//   sprintf(dMiscbuffer,"Dimensions: %d\n",dDim);  
//   strcat(dFitlog,dMiscbuffer);
//   sprintf(dMiscbuffer,"Entries: %d\n",dEntries);  
//   strcat(dFitlog,dMiscbuffer);  
//   sprintf(dMiscbuffer,"Fit range min = %lf\n",dLFitrange[0]);
//   strcat(dFitlog,dMiscbuffer);
//   sprintf(dMiscbuffer,"Fit range max = %lf\n",dHFitrange[0]);
//   strcat(dFitlog,dMiscbuffer);
//   sprintf(dMiscbuffer,"Total number of bins = %d\n",dNumbins[0]);  
//   strcat(dFitlog,dMiscbuffer);
//   sprintf(dMiscbuffer,"Fit range start bin = %d\n",dStartbin[0]);  
//   strcat(dFitlog,dMiscbuffer);
//   sprintf(dMiscbuffer,"Fit range stop bin = %d\n",dStopbin[0]);  
//   strcat(dFitlog,dMiscbuffer);
//   sprintf(dMiscbuffer,"Number of bins in fit range = %d\n",dRNumbins[0]);  
//   strcat(dFitlog,dMiscbuffer);
  
//   strcat(dFitlog,"\n\nParamters:\n\n");
//   for(i = 0; i < dNumpars; i++){
//     sprintf(dMiscbuffer,"%d) = % 2.5e\n",i,dParms[i]);
//     strcat(dFitlog,dMiscbuffer);
//   }
  
//   if(dFitlogoutput >= FL_BARF){ 
//     strcat(dFitlog,"\n\nFit range data:\n\n");  
//     for(i = dStartbin[0]; i <= dStopbin[0]; i++){
//       sprintf(dMiscbuffer,"Bin val = %lf; wgt = %lf\n",
// 	      binv[i],wgt[i]);
//       strcat(dFitlog,dMiscbuffer);
//     }
//   }
 
//   strcat(dFitlog,"\n\nNow Starting Minimization:\n\n");  
  
//   while(1){    
    
//     if(dFitlogoutput >= FL_VERBOSE){ 
//       sprintf(dMiscbuffer,"\n\nIteration %d\n\n",n);
//       strcat(dFitlog,dMiscbuffer);
//     }
//     for(i = 0; i < dNumpars; i++)
//       if(dParms[i] == 0)
// 	dSteps[i] = 1;
//       else
// 	dSteps[i] = sqrt(pow(dParms[i]/2,2));

//     for(i = 0; i < dNumpars; i++){
      
//       dChisq = MinimizeChisq(fitfunc,dDim,binv,wgt,wgte,dStartbin[0],
// 			     dStopbin[0],dParms,i,&dSteps[i],
// 			     dChisqlowbnd,&dDegfree);
//       dDegfree = dDegfree - dNumpars;
//       dChisqdeg = dChisq/dDegfree;
      
//       if(dFitlogoutput >= FL_SUPERVERBOSE){ 
// 	for(int l = 0; l < dNumpars; l++){
// 	  sprintf(dMiscbuffer,"Par. %d = %lf; Step %d = %lf\n",
// 		  l,dParms[l],l,dSteps[l]);
// 	  //printf(dMiscbuffer);
// 	  strcat(dFitlog,dMiscbuffer);
// 	}
// 	sprintf(dMiscbuffer,"\nDegrees of freedom = %d\n",dDegfree);
// 	//printf(dMiscbuffer);
// 	strcat(dFitlog,dMiscbuffer);
// 	sprintf(dMiscbuffer,"chisq = %lf; chisq/deg = %lf\n",dChisq,dChisqdeg);
// 	//printf(dMiscbuffer);
// 	strcat(dFitlog,dMiscbuffer);
//       }
//     }
    
//     if(dFitlogoutput >= FL_VERBOSE){ 

//       strcat(dFitlog,"\nResults for this iteration:\n\n");  

//       for(int l = 0; l < dNumpars; l++){
// 	sprintf(dMiscbuffer,"Par. %d = %lf; Step %d = %lf\n",
// 		l,dParms[l],l,dSteps[l]);
// 	//printf(dMiscbuffer);
// 	strcat(dFitlog,dMiscbuffer);
//       }

//       sprintf(dMiscbuffer,"\nDegrees of freedom = %d\n",dDegfree);
//       //printf(dMiscbuffer);
//       strcat(dFitlog,dMiscbuffer);
//       sprintf(dMiscbuffer,"chisq = %lf; chisq/deg = %lf\n\n",dChisq,dChisqdeg);
//       //printf(dMiscbuffer);    
//       strcat(dFitlog,dMiscbuffer);
//     }

//     if((dChisq < lastchisq + bound) && (dChisq > lastchisq - bound)){break;}
//     n++;
//     lastchisq = dChisq;
//   }

  

//   strcat(dFitlog,"\nFinal results for this fit:\n\n");  
//   sprintf(dMiscbuffer,"                  Value           Error            Step\n");
//   //printf(dMiscbuffer);
//   strcat(dFitlog,dMiscbuffer);
//   for(int l = 0; l < dNumpars; l++){
//     //sprintf(dMiscbuffer,"Par. %d = %lf; Step %d = %lf\n",l,dParms[l],l,dSteps[l]);

//     for(int n = 0; n < dNumpars; n++){
//       par1[n] = dParms[n];
//     }

//     par1[l] = dParms[l]-9*dSteps[l]/2;
//     ChiSquared(fitfunc,binv,wgt,wgte,par1,dStartbin[0],dStopbin[0],&chi1,&dgf);
//     par1[l] += 4*dSteps[l];
//     ChiSquared(fitfunc,binv,wgt,wgte,par1,dStartbin[0],dStopbin[0],&chi2,&dgf);
//     par1[l] += 4*dSteps[l];
//     ChiSquared(fitfunc,binv,wgt,wgte,par1,dStartbin[0],dStopbin[0],&chi3,&dgf);
    
//     dParmerrors[l] = 4*dSteps[l]*sqrt(2/(pow(chi1,2)-2*pow(chi2,2)+pow(chi3,2)));

//     sprintf(dMiscbuffer,"%-10s %2d    % 2.5e    % 2.5e    % 2.5e\n",
// 	    "Parameter",l,dParms[l],dParmerrors[l],dSteps[l]);
//     strcat(dFitlog,dMiscbuffer);
//   }
//   sprintf(dMiscbuffer,"\nDegrees of freedom = %d\n",dDegfree);
//   strcat(dFitlog,dMiscbuffer);
//   sprintf(dMiscbuffer,"chisq = %lf; chisq/deg = %lf\n\n\n",dChisq,dChisqdeg);
//   strcat(dFitlog,dMiscbuffer);
  
//   for(i = 0; i < dNumpars; i++){
//     param[i] = dParms[i];
//   }

//   return FIT_PROCESS_OK;
// }


// Int_t RDataFit::ChiSquared(Double_t(*fitfunc)(Double_t*,Double_t*),
// 			  Double_t *x, Double_t *y, Double_t *ye,
// 			  Double_t *parm,Int_t start, Int_t stop,
// 			  Double_t *chi2, Int_t *degfree)
// {
//   Double_t chisq = 0.0;
//   Double_t ffunc = 0;
//   Int_t dgf = 0;
  
//   for(Int_t i = start; i <= stop; i++){
//     //used to be if(y[i] > 1){

//     if(sqrt(pow(y[i],2)) > 0){
//       ffunc = fitfunc(&x[i],parm);
//       //printf("(%d): x = %lf; y = %lf; ye = %lf; ffunc = %lf;\n",
//       //     i,x[i],y[i],ye[i],ffunc);
//     //printf("par1 = %lf; par2 = %lf; par3 = %lf\n",
//     //     parm[0],parm[1],parm[2]);
//       chisq += (y[i]-ffunc)*(y[i]-ffunc)/(ye[i]*ye[i]);
//       dgf++;
//     }
//   }
//   chi2[0] = chisq;
//   degfree[0] = dgf;
  
//   return 1;
// }

// Double_t RDataFit::MinimizeChisq(Double_t(*fitfunc)(Double_t*,Double_t*),
// 				 Int_t dim,Double_t *x, Double_t *y, 
// 				 Double_t *ye, Int_t start, Int_t stop, 
// 				 Double_t *parm, Int_t parind, 
// 				 Double_t *stepsize, Double_t bnd, 
// 				 Int_t *degfree)
// {
  
//   Int_t stepdir = 1;
//   Double_t bound = bnd;
//   Double_t lchisq = 5000;
//   Double_t rchisq = 5000;
//   Double_t chisq = 0.0;
//   Double_t step = stepsize[0];
//   //Double_t ffunc = 0;
//   Int_t dgf = 0;

//   if(dFitlogoutput >= FL_VERBOSE){
//     sprintf(dMiscbuffer,"\n\nVarying paramter %d:\n\n",parind+1);
//     //printf(dMiscbuffer);
//     strcat(dFitlog,dMiscbuffer);
//   }

//   while(1){

// //     for(Int_t i = start; i <= stop; i++){
// //       //used to be if(y[i] > 1){
// //       if(sqrt(pow(y[i],2)) > 0){
// // 	ffunc = fitfunc(&x[i],parm);
// // 	chisq += (y[i]-ffunc)*(y[i]-ffunc)/(ye[i]*ye[i]);
// // 	dgf++;
// //       }
// //     }

//     ChiSquared(fitfunc,x,y,ye,parm,start,stop,&chisq,&dgf);
//     //printf("chisq = %lf\n",chisq);

//     if(dFitlogoutput >= FL_SUPERVERBOSE){
//       sprintf(dMiscbuffer,
//       "\nParameter = %lf; Chisq = %lf; Left Chisq = %lf; Right Chisq = %lf\n",
// 	      parm[parind],chisq,lchisq,rchisq);
//       //printf(dMiscbuffer);
//       strcat(dFitlog,dMiscbuffer);
//       strcat(dFitlog,"Now determinig next step direction and size:\n");
//     }
    
//     if(stepdir > 0){
//       if(chisq < lchisq) { 
// 	parm[parind] += step; stepdir = 1;
// 	lchisq = chisq;

// 	if(dFitlogoutput == FL_BARF){
// 	  sprintf(dMiscbuffer,"Step direction =  %s, Step size = %lf\n",
// 		  stepdir > 0 ? "right" : "left",step);
// 	  //printf(dMiscbuffer);

// 	  strcat(dFitlog,dMiscbuffer);
// 	}

//       }
//       else if(chisq >= lchisq) { 
// 	rchisq = chisq;
// 	//step = step/3; parm[parind] -= step; stepdir = -1;
// 	//used to be here
// 	if(rchisq <= lchisq + bound){ 
// 	  step = step/2; //didn't used to be here

// 	  //parm[parind] -= 2*step; this used to be here instead of the
// 	  //following line
// 	  parm[parind] -= step;

// 	  //chisq = lchisq; this used to be here instead of the
// 	  //following two lines
// 	  ChiSquared(fitfunc,x,y,ye,parm,start,stop,
// 		      &chisq,&dgf);
// 	  break;
// 	}

// 	//used be above
// 	step = step/3; parm[parind] -= step; stepdir = -1;

// 	if(dFitlogoutput == FL_BARF){
// 	  sprintf(dMiscbuffer,"Step direction =  %s, Step size = %lf\n",
// 		  stepdir > 0 ? "right" : "left",step);
// 	  //printf(dMiscbuffer);
// 	  strcat(dFitlog,dMiscbuffer);
// 	}
//       }
//     }
//     else if(stepdir < 0){
//       if(chisq < rchisq) { 
// 	parm[parind] -= step; stepdir = -1;
// 	rchisq = chisq;

// 	if(dFitlogoutput == FL_BARF){
// 	  sprintf(dMiscbuffer,"Step direction =  %s, Step size = %lf\n",
// 		  stepdir > 0 ? "right" : "left",step);
// 	  //printf(dMiscbuffer);
// 	  strcat(dFitlog,dMiscbuffer);
// 	}

//       }
//       else if(chisq >= rchisq) { 
// 	lchisq = chisq;
// 	//step = step/3; parm[parind] += step; stepdir = 1;
// 	//used to be here

// 	if(lchisq <= rchisq + bound){
// 	  step = step/2; //didn't used to be here

// 	  //parm[parind] += 2*step; this used to be here instead of the
// 	  //following line
// 	  parm[parind] += step;

// 	  //chisq = rchisq; this used to be here instead of the
// 	  //following two lines
// 	  ChiSquared(fitfunc,x,y,ye,parm,start,stop,
// 		      &chisq,&dgf);
// 	  break;
// 	}

// 	//used to be above
// 	step = step/3; parm[parind] += step; stepdir = 1;
	
// 	if(dFitlogoutput == FL_BARF){
// 	  sprintf(dMiscbuffer,"Step direction =  %s, Step size = %lf\n",
// 		  stepdir > 0 ? "right" : "left",step);
// 	  //printf(dMiscbuffer);
// 	  strcat(dFitlog,dMiscbuffer);
// 	}

//       }
//     }   
//   }

//   degfree[0]  = dgf;
//   stepsize[0] = step;
//   return chisq;
// }
