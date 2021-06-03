/////////////////////////////////////////////////////////////////////////////////////
//Root Based Data Visualization and Analysis Utility Library
//
//Version 000
//
//RPlotContainer source file with basic methods to contain and access
//graphs histograms etc..
/////////////////////////////////////////////////////////////////////////////////////

#include "RPlotContainer.h"

RPlotContainer::RPlotContainer(const TGWindow *p, const TGWindow *main,
			       char *objName, char *mainname, char *conttitle)
  : TQObject()
{  
  fPlotList = new TList();
  fGraphList = new TList();
  fGraphErList = new TList();
  fGraphAsymErList = new TList();
  // fMultiGraph = new TMultiGraph();
  fD1HistoList = new TList();
  fD1ProfileList = new TList();
  fD2HistoList = new TList();
  fD3HistoList = new TList();
  fFuncList    = new TList();
  // fPlotList->Add(fMultiGraph);

  dPlotOptions = new PlotContainerOptions;
  SetDefaultPlotOptions();

  dPlotCount = 0;
  dMsgcnt = 0;

  memset(dObjName,'\0',NAME_STR_MAX);
  strcpy(dObjName,objName);

  memset(dContTitle,'\0',NAME_STR_MAX);
  strcpy(dContTitle,conttitle);
  
  memset(dMainName,'\0',sizeof(dMainName));
  strcpy(dMainName,mainname);

  memset(dMiscbuffer,'\0',sizeof(dMiscbuffer));
  memset(dMiscbuffer2,'\0',sizeof(dMiscbuffer2));
  for(int i = 0; i < MSG_QUEUE_MAX; i++)
    memset(dMessage[i],'\0',sizeof(dMessage[i]));

  Connect("IsClosing(char*)",dMainName,(void*)main,"OnObjClose(char*)");  
  Connect("SendMessageSignal(char*)",dMainName,(void*)main,
	  "OnReceiveMessage(char*)");
}

RPlotContainer::~RPlotContainer()
{
  IsClosing(dObjName);

  //  ClearPlots();
  if(fPlotList)
    delete fPlotList;  
  if(fGraphList)
    delete fGraphList;  
  if(fGraphErList)
    delete fGraphErList;    
  if(fGraphAsymErList)
    delete fGraphAsymErList; 
  if(fMultiGraph)
    delete fMultiGraph;
  if(fD1HistoList)
    delete fD1HistoList;
  if(fD1ProfileList)
    delete fD1ProfileList;
  if(fD2HistoList)
    delete fD2HistoList;
  if(fD3HistoList)
    delete fD3HistoList;
  if(fFuncList)
    delete fFuncList;          
}

void RPlotContainer::IsClosing(char *objname)
{
  Emit("IsClosing(char*)",(long)objname);
}

void RPlotContainer::SendMessageSignal(char *objname)
{
  Emit("SendMessageSignal(char*)",(long)objname);
}

void RPlotContainer::SetPlotOptions(PlotContainerOptions *options)
{
  *dPlotOptions = *options;
}

Int_t RPlotContainer::GetPlotCount()
{
  if(fPlotList)return fPlotList->GetSize();

  return 0;
}

Int_t RPlotContainer::GetAsymErGraphCount()
{
  if(fGraphAsymErList)return fGraphAsymErList->GetSize();

  return 0;
}

Int_t RPlotContainer::GetErGraphCount()
{
  if(fGraphErList)return fGraphErList->GetSize();

  return 0;
}

Int_t RPlotContainer::GetGraphCount()
{
  if(fGraphList)return fGraphList->GetSize();

  return 0;
}

Int_t RPlotContainer::GetFuncCount()
{
  if(fFuncList)return fFuncList->GetSize();

  return 0;
}

Int_t RPlotContainer::Get1DHistoCount()
{
  if(fD1HistoList)return fD1HistoList->GetSize();

  return 0;
}

Int_t RPlotContainer::Get1DProfileCount()
{
  if(fD1ProfileList)return fD1ProfileList->GetSize();

  return 0;
}

Int_t RPlotContainer::Get2DHistoCount()
{
  if(fD2HistoList)return fD2HistoList->GetSize();

  return 0;
}

Int_t RPlotContainer::GetNewLineColor(Color_t col)
{
  Color_t objcol = 0;
  TObject *obj = NULL;
  Color_t colors[50] = {1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

  for(int i = 0; i < GetPlotCount(); i++){
    obj = fPlotList->At(i);
    if(obj){
      if(obj->InheritsFrom("TH1")) objcol = ((TH1*)obj)->GetLineColor();
      else if(obj->InheritsFrom("TGraph")) objcol = ((TGraph*)obj)->GetLineColor();
    }
    // if(objcol == col) flag = 0;
    if(objcol < 50)
      colors[objcol] = 1;
  }

  if(col < 50 && colors[col] == 0) return col; 

  for(int j = 0; j < 50; j++){
    if(!colors[j]) return j;
  }
  

  // Color_t colors[10] = {0,1,2,3,4,5,6,7,8,9};
  // Color_t objcol = 0;
  // Int_t   flag = 1;
  // TObject *obj = NULL;
  // for(int i = 0; i < GetPlotCount(); i++){
  //   obj = fPlotList->At(i);
  //   if(obj){
  //     if(obj->InheritsFrom("TH1")) objcol = ((TH1*)obj)->GetLineColor();
  //     else if(obj->InheritsFrom("TGraph")) objcol = ((TGraph*)obj)->GetMarkerColor();
  //   }
  //   if(objcol == col) flag = 0;
  //   if(objcol < 10)
  //     colors[objcol] = 0;
  // }

  // if(flag) return col;

  // for(int j = 0; j < 10; j++){
  //   if(colors[j]) return j;
  // }

  return 1; //all basic colors are used, make it black
}

Int_t RPlotContainer::GetNewMarkerColor(Color_t col)
{
  Color_t objcol = 0;
  TObject *obj = NULL;
  Color_t colors[50] = {1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  for(int i = 0; i < GetPlotCount(); i++){
    obj = fPlotList->At(i);
    if(obj){
      if(obj->InheritsFrom("TH1")) objcol = ((TH1*)obj)->GetMarkerColor();
      else if(obj->InheritsFrom("TGraph")) objcol = ((TGraph*)obj)->GetMarkerColor();
    }
    // if(objcol == col) flag = 0;
    if(objcol < 50)
      colors[objcol] = 1;
  }

  if(col < 50 && colors[col] == 0) return col; 

  for(int j = 0; j < 50; j++){
    if(!colors[j]) return j;
  }

//   Color_t colors[10] = {0,1,2,3,4,5,6,7,8,9};
//   Color_t objcol = 0;
//   Int_t   flag = 1;

//   TObject *obj = NULL;
// //   printf("Plot Count = %d\n",GetPlotCount());
//   for(int i = 0; i < GetPlotCount(); i++){
//     obj = fPlotList->At(i);
//     if(obj){
//       if(obj->InheritsFrom("TH1")) objcol = ((TH1*)obj)->GetMarkerColor();
//       else if(obj->InheritsFrom("TGraph")) objcol = ((TGraph*)obj)->GetMarkerColor();
//     }
//     if(objcol == col) flag = 0;
//     if(objcol < 10)
//       colors[objcol] = 0;
//   }

//   if(flag) return col;

//   for(int j = 0; j < 10; j++){
//     if(colors[j]) return j;
//   }

  return 1; //all basic colors are used, make it black
}

void RPlotContainer::SetNewLineColor()
{
  SetLineColor(GetNewLineColor());

  // for(Int_t i = 0; i < 10; i++){
  //   if(GetNewLineColor(i) == i){
  //     SetLineColor(i);
  //     return;
  //   }
  //   if(GetNewLineColor(i) == 11){
  //     SetLineColor(11);
  //     return;
  //   }
  // }
}

void RPlotContainer::SetNewMarkerColor()
{
  SetMarkerColor(GetNewMarkerColor());
  // for(Int_t i = 0; i < 10; i++){
  //   if(GetNewMarkerColor(i) == i){
  //     SetMarkerColor(i);
  //     return;
  //   }
  //   if(GetNewMarkerColor(i) == 11){
  //     SetMarkerColor(11);
  //     return;
  //   }
  // }
}

void RPlotContainer::RemovePlot(TObject *obj)
{
  if(!obj) return; 

  if(obj->InheritsFrom("TGraph"))
    fGraphList->Remove(obj);
  if(obj->InheritsFrom("TH1"))
    fD1HistoList->Remove(obj);
  if(obj->InheritsFrom("TProfile"))
    fD1ProfileList->Remove(obj);
  if(obj->InheritsFrom("TGraphErrors"))
    fGraphErList->Remove(obj);
  if(obj->InheritsFrom("TGraphAsymmErrors"))
    fGraphAsymErList->Remove(obj);
  if(obj->InheritsFrom("TH2"))
    fD2HistoList->Remove(obj);
  if(obj->InheritsFrom("TH3"))
    fD3HistoList->Remove(obj);
  if(obj->InheritsFrom("TF1"))
    fFuncList->Remove(obj);
  
  fPlotList->Remove(obj);
}


void RPlotContainer::ClearPlots()
{
  if(fGraphList)
    fGraphList->Clear();
  if(fGraphErList)
    fGraphErList->Clear();
  if(fGraphAsymErList)
    fGraphAsymErList->Clear();
  if(fD1HistoList)
    fD1HistoList->Clear();
  if(fD1ProfileList)
    fD1ProfileList->Clear();
  if(fD3HistoList)
    fD3HistoList->Clear();
  if(fFuncList)
    fFuncList->Clear();

  if(fPlotList){
    fPlotList->Remove(fMultiGraph);
     TList *graphs = NULL;
    if(fMultiGraph) graphs = fMultiGraph->GetListOfGraphs();
     if(graphs){
      for(int i = 0; i < graphs->GetSize(); i++){
	fPlotList->Remove(graphs->At(i));
      }	 
    }    
    fPlotList->Delete("slow");
  }
  if(fMultiGraph) delete fMultiGraph; 

  dPlotCount = 0;
}

void RPlotContainer::FlushMessages()
{
  while(GetMessage() != NULL){}
}

char *RPlotContainer::GetMessage()
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


Bool_t RPlotContainer::SetMessage(char *msg, char *func, int TS, int MESSAGETYPE)
{
  if(dMsgcnt == MSG_QUEUE_MAX)
    {
      for(int i = 0; i < dMsgcnt-1; i++)
	strcpy(dMessage[i],dMessage[i+1]);
      memset(dMessage[MSG_QUEUE_MAX-1],'\0',sizeof(dMessage[MSG_QUEUE_MAX-1]));
      strcpy(dMessage[MSG_QUEUE_MAX-1],msg);
    }
  else
    {
      strcpy(dMessage[dMsgcnt],msg);
      dMsgcnt++;
    }
      
  SendMessageSignal(dObjName);
  gSystem->ProcessEvents();

  return kTrue;
}

void RPlotContainer::SetDefaultPlotOptions()
{
  dPlotOptions->lineColor = 2;
  dPlotOptions->lineWidth = 1;
  dPlotOptions->lineStyle = 1;
  dPlotOptions->markerColor = 1;
  dPlotOptions->markerSize = 0.3;
  dPlotOptions->markerStyle = 20;
}

TF1 *RPlotContainer::GetNewFunction(const TF1& func)
{
  char temp[NAME_STR_MAX];
  TF1 *f1 = new TF1(func);
  if(f1){
    sprintf(temp,"%s_cp",f1->GetName());
    f1->SetName(temp);
    f1->SetLineWidth(dPlotOptions->lineWidth);
    f1->SetLineStyle(dPlotOptions->lineStyle);
    f1->SetLineColor(dPlotOptions->lineColor);
    fFuncList->Add(f1);
    fPlotList->Add(f1);
    return f1;
  }
  return NULL;
}

TH1D *RPlotContainer::GetNew1DHistogram(const TH1D& h1d)
{
  char temp[NAME_STR_MAX];
  TH1D *hist = new TH1D(*(TH1D*)h1d.Clone());
  if(hist) {
    hist->SetCanExtend(TH1::kXaxis); 
    //hist->SetBit(TH1::kCanRebin);
    hist->SetDirectory(0);
    sprintf(temp,"%s_cp",hist->GetName());
    hist->SetName(temp);
    hist->SetLineWidth(dPlotOptions->lineWidth);
    hist->SetLineStyle(dPlotOptions->lineStyle);
    hist->SetLineColor(dPlotOptions->lineColor);
    hist->SetMarkerSize(dPlotOptions->markerSize);
    hist->SetMarkerStyle(dPlotOptions->markerStyle);
    hist->SetMarkerColor(dPlotOptions->markerColor);
    fD1HistoList->Add(hist);
    fPlotList->Add(hist);
    return hist;
  }
  return NULL;
}

TProfile *RPlotContainer::GetNew1DProfile(const TProfile& prf)
{
  char temp[NAME_STR_MAX];
  TProfile *prof = new TProfile(*(TProfile*)prf.Clone());
  if(prof) {
    prof->SetCanExtend(TH1::kXaxis);
    //prof->SetBit(TH1::kCanRebin);
    prof->SetDirectory(0);
    sprintf(temp,"%s_cp",prof->GetName());
    prof->SetName(temp);
    prof->SetLineWidth(dPlotOptions->lineWidth);
    prof->SetLineStyle(dPlotOptions->lineStyle);
    prof->SetLineColor(dPlotOptions->lineColor);
    prof->SetMarkerSize(dPlotOptions->markerSize);
    prof->SetMarkerStyle(dPlotOptions->markerStyle);
    prof->SetMarkerColor(dPlotOptions->markerColor);
    fD1ProfileList->Add(prof);
    fPlotList->Add(prof);
    return prof;
  }
  return NULL;
}

TH1D *RPlotContainer::GetNew1DHistogram(Char_t* name, Char_t* title, 
					Int_t numbins, Double_t min, Double_t max)
{
  TH1D *hist = new TH1D(name,title,numbins,min,max);
  if(hist) {
    hist->SetCanExtend(TH1::kXaxis);
    //hist->SetBit(TH1::kCanRebin);
    hist->SetDirectory(0);
    hist->SetLineWidth(dPlotOptions->lineWidth);
    hist->SetLineStyle(dPlotOptions->lineStyle);
    hist->SetLineColor(dPlotOptions->lineColor);
    hist->SetMarkerSize(dPlotOptions->markerSize);
    hist->SetMarkerStyle(dPlotOptions->markerStyle);
    hist->SetMarkerColor(dPlotOptions->markerColor);
    fD1HistoList->Add(hist);
    fPlotList->Add(hist);
    return hist;
  }
  return NULL;
}

TH2D *RPlotContainer::GetNew2DHistogram(const TH2D& h2d)
{
  char temp[NAME_STR_MAX];
  TH2D *hist = new TH2D(*(TH2D*)h2d.Clone());
  if(hist) {
    hist->SetDirectory(0);
    sprintf(temp,"%s_cp",hist->GetName());
    hist->SetName(temp);
//     hist->SetLineWidth(dPlotOptions->lineWidth);
//     hist->SetLineStyle(dPlotOptions->lineStyle);
//     hist->SetLineColor(dPlotOptions->lineColor);
//     hist->SetMarkerSize(dPlotOptions->markerSize);
//     hist->SetMarkerStyle(dPlotOptions->markerStyle);
//     hist->SetMarkerColor(dPlotOptions->markerColor);
    fD2HistoList->Add(hist);
    fPlotList->Add(hist);
    return hist;
  }
  return NULL;
}

TH2D *RPlotContainer::GetNew2DHistogram(Char_t* name, Char_t* title, 
					Int_t numbins1, Double_t min1, Double_t max1,
					Int_t numbins2, Double_t min2, Double_t max2)
{
  TH2D *hist = new TH2D(name,title,numbins1,min1,max1,numbins2,min2,max2);
  if(hist) {
    hist->SetDirectory(0);
//     hist->SetLineWidth(dPlotOptions->lineWidth);
//     hist->SetLineStyle(dPlotOptions->lineStyle);
//     hist->SetLineColor(dPlotOptions->lineColor);
//     hist->SetMarkerSize(dPlotOptions->markerSize);
//     hist->SetMarkerStyle(dPlotOptions->markerStyle);
//     hist->SetMarkerColor(dPlotOptions->markerColor);
    fD2HistoList->Add(hist);
    fPlotList->Add(hist);
    return hist;
  }
  return NULL;
}

TMultiGraph *RPlotContainer::AddMultiGraphObject(TObject *obj, const char *title, const char *type, const char *opts)
{
  char temp[NAME_STR_MAX];

  if(!obj) return NULL;
  if(!obj->InheritsFrom("TGraph")) return NULL;
  if(!type) return NULL;

  if(!fMultiGraph){
    fMultiGraph = new TMultiGraph();
    if(!fMultiGraph) return NULL;
    fPlotList->Add(fMultiGraph);
  }

  if(!strcmp(type,"TGraph")){
    TGraph *gr = new TGraph(*(TGraph*)obj->Clone());
    if(gr){
      sprintf(temp,"%s_cp",gr->GetName());
      gr->SetName(temp);
      fGraphList->Add(gr);
      // fPlotList->Add(gr);
      fMultiGraph->Add(gr,opts);
      return fMultiGraph;
    }
  }
  else if(!strcmp(type,"TGraphErrors")){
    TGraphErrors *gr = new TGraphErrors(*(TGraphErrors*)obj->Clone());
    if(gr){
      sprintf(temp,"%s_cp",gr->GetName());
      gr->SetName(temp);
      fGraphErList->Add(gr);
      // fPlotList->Add(gr);
      fMultiGraph->Add(gr,opts);
      return fMultiGraph;
    }
  }
  else if(!strcmp(type,"TGraphAsymmErrors")){
    TGraphAsymmErrors *gr = new TGraphAsymmErrors(*(TGraphAsymmErrors*)obj->Clone());
    if(gr){
      sprintf(temp,"%s_cp",gr->GetName());
      gr->SetName(temp);
      fGraphAsymErList->Add(gr);
      // fPlotList->Add(gr);
      fMultiGraph->Add(gr,opts);
      return fMultiGraph;
    }
  }
  return NULL;
}

TMultiGraph *RPlotContainer::AddMultiGraphObject(const char* name, const char *title,
						 Int_t n, Double_t *x, Double_t *y,
						 Double_t *exl, Double_t *eyl,
						 Double_t *exh, Double_t *eyh,
						 const char *type, const char *opts)
{
  if(!x || !y) return NULL;
  if(!type) return NULL;

  if(!fMultiGraph){
    fMultiGraph = new TMultiGraph();
    if(!fMultiGraph) return NULL;
    fPlotList->Add(fMultiGraph);
  }

  if(!strcmp(type,"TGraph")){
    TGraph *gr = new TGraph(n,x,y);
    if(gr){
      gr->SetName(name);
      gr->SetTitle(title);
      gr->SetLineWidth(dPlotOptions->lineWidth);
      gr->SetLineStyle(dPlotOptions->lineStyle);
      gr->SetLineColor(dPlotOptions->lineColor);
      gr->SetMarkerSize(dPlotOptions->markerSize);
      gr->SetMarkerStyle(dPlotOptions->markerStyle);
      gr->SetMarkerColor(dPlotOptions->markerColor);    
      fGraphList->Add(gr);
      // fPlotList->Add(gr);
      fMultiGraph->Add(gr,opts);
      return fMultiGraph;
    }
  }
  if(!strcmp(type,"TGraphErrors")){
    TGraphErrors *gr = new TGraphErrors(n,x,y,exl,eyl);
    if(gr){
      gr->SetName(name);
      gr->SetTitle(title);
      gr->SetLineWidth(dPlotOptions->lineWidth);
      gr->SetLineStyle(dPlotOptions->lineStyle);
      gr->SetLineColor(dPlotOptions->lineColor);
      gr->SetMarkerSize(dPlotOptions->markerSize);
      gr->SetMarkerStyle(dPlotOptions->markerStyle);
      gr->SetMarkerColor(dPlotOptions->markerColor);    
      fGraphErList->Add(gr);
      // fPlotList->Add(gr);
      fMultiGraph->Add(gr,opts);
      return fMultiGraph;
    }
  }
  if(!strcmp(type,"TGraphAsymmErrors")){
    TGraphAsymmErrors *gr = new TGraphAsymmErrors(n,x,y,exl,exh,eyl,eyh);
    if(gr){
      gr->SetName(name);
      gr->SetTitle(title);
      gr->SetLineWidth(dPlotOptions->lineWidth);
      gr->SetLineStyle(dPlotOptions->lineStyle);
      gr->SetLineColor(dPlotOptions->lineColor);
      gr->SetMarkerSize(dPlotOptions->markerSize);
      gr->SetMarkerStyle(dPlotOptions->markerStyle);
      gr->SetMarkerColor(dPlotOptions->markerColor);    
      fGraphAsymErList->Add(gr);
      // fPlotList->Add(gr);
      fMultiGraph->Add(gr,opts);
      return fMultiGraph;
    }
  }
  return NULL;
}

TGraphAsymmErrors *RPlotContainer::GetNewAsymErrorGraph(const TGraphAsymmErrors& grph)
{
  char temp[NAME_STR_MAX];
  TGraphAsymmErrors *gr = new TGraphAsymmErrors(*(TGraphAsymmErrors*)grph.Clone());

  if(!gr) return NULL;  

  if(!fMultiGraph){
    fMultiGraph = new TMultiGraph();
    if(!fMultiGraph) {delete gr; return NULL;}
    fPlotList->Add(fMultiGraph);
  }

  sprintf(temp,"%s_cp",gr->GetName());
  gr->SetName(temp);
  gr->SetLineWidth(dPlotOptions->lineWidth);
  gr->SetLineStyle(dPlotOptions->lineStyle);
  gr->SetLineColor(dPlotOptions->lineColor);
  gr->SetMarkerSize(dPlotOptions->markerSize);
  gr->SetMarkerStyle(dPlotOptions->markerStyle);
  gr->SetMarkerColor(dPlotOptions->markerColor);
  fGraphAsymErList->Add(gr);
  fMultiGraph->Add(gr,"p");
  // fPlotList->Add(gr);
  return gr;
}


TGraphAsymmErrors *RPlotContainer::GetNewAsymErrorGraph(Char_t* name, Char_t* title,
							Int_t n, Double_t *x, 
							Double_t *y, Double_t *exl,
							Double_t *exh, Double_t *eyl,
							Double_t *eyh)
{
  TGraphAsymmErrors *gr = new TGraphAsymmErrors(n,x,y,exl,exh,eyl,eyh);

  if(!gr) return NULL;  

  if(!fMultiGraph){
    fMultiGraph = new TMultiGraph();
    if(!fMultiGraph) {delete gr; return NULL;}
    fPlotList->Add(fMultiGraph);
  }

  gr->SetName(name);
  gr->SetLineWidth(dPlotOptions->lineWidth);
  gr->SetLineStyle(dPlotOptions->lineStyle);
  gr->SetLineColor(dPlotOptions->lineColor);
  gr->SetMarkerSize(dPlotOptions->markerSize);
  gr->SetMarkerStyle(dPlotOptions->markerStyle);
  gr->SetMarkerColor(dPlotOptions->markerColor);
  fGraphAsymErList->Add(gr);
  // fPlotList->Add(gr);
  fMultiGraph->Add(gr,"p");
  return gr;
}

TGraphErrors *RPlotContainer::GetNewErrorGraph(const TGraphErrors& grph)
{
  char temp[NAME_STR_MAX];
  TGraphErrors *gr = new TGraphErrors(*(TGraphErrors*)grph.Clone());
  if(!gr) return NULL;  

  if(!fMultiGraph){
    fMultiGraph = new TMultiGraph();
    if(!fMultiGraph) {delete gr; return NULL;}
    fPlotList->Add(fMultiGraph);
  }
  sprintf(temp,"%s_cp",gr->GetName());
  gr->SetName(temp);
  gr->SetLineWidth(dPlotOptions->lineWidth);
  gr->SetLineStyle(dPlotOptions->lineStyle);
  gr->SetLineColor(dPlotOptions->lineColor);
  gr->SetMarkerSize(dPlotOptions->markerSize);
  gr->SetMarkerStyle(dPlotOptions->markerStyle);
  gr->SetMarkerColor(dPlotOptions->markerColor);
  fGraphErList->Add(gr);
  // fPlotList->Add(gr);
  fMultiGraph->Add(gr,"p");
  return gr;
}

TGraphErrors *RPlotContainer::GetNewErrorGraph(Char_t* name, Char_t* title, 
					       Int_t n, Double_t *x, 
					       Double_t *y, Double_t *ex, 
					       Double_t *ey)
{
  TGraphErrors *gr = new TGraphErrors(n,x,y,ex,ey);
  if(!gr) return NULL;  

  if(!fMultiGraph){
    fMultiGraph = new TMultiGraph();
    if(!fMultiGraph) {delete gr; return NULL;}
    fPlotList->Add(fMultiGraph);
  }
  gr->SetName(name);
  gr->SetTitle(title);
  gr->SetLineWidth(dPlotOptions->lineWidth);
  gr->SetLineStyle(dPlotOptions->lineStyle);
  gr->SetLineColor(dPlotOptions->lineColor);
  gr->SetMarkerSize(dPlotOptions->markerSize);
  gr->SetMarkerStyle(dPlotOptions->markerStyle);
  gr->SetMarkerColor(dPlotOptions->markerColor);
  fGraphErList->Add(gr);
  // fPlotList->Add(gr);
  fMultiGraph->Add(gr,"p");
  return gr;
}

TGraph *RPlotContainer::GetNewGraph(const TGraph& grph)
{
  char temp[NAME_STR_MAX];
  TGraph *gr = new TGraph(*(TGraph*)grph.Clone());
  if(!gr) return NULL;  

  if(!fMultiGraph){
    fMultiGraph = new TMultiGraph();
    if(!fMultiGraph) {delete gr; return NULL;}
    fPlotList->Add(fMultiGraph);
  }
  sprintf(temp,"%s_cp",gr->GetName());
  gr->SetName(temp);
  gr->SetMarkerColor(dPlotOptions->markerColor);
  gr->SetMarkerStyle(dPlotOptions->markerStyle);
  gr->SetMarkerSize(dPlotOptions->markerSize);
  gr->SetLineWidth(dPlotOptions->lineWidth);
  gr->SetLineStyle(dPlotOptions->lineStyle);	
  gr->SetLineColor(dPlotOptions->lineColor);
  fGraphList->Add(gr);
  // fPlotList->Add(gr);
  fMultiGraph->Add(gr,"p");
  return gr;
}

TGraph *RPlotContainer::GetNewGraph(Char_t *name, Char_t *title, Int_t n, 
				    Double_t *x, Double_t *y)
{
  TGraph *gr = new TGraph(n,x,y);
  if(!gr) return NULL;  

  if(!fMultiGraph){
    fMultiGraph = new TMultiGraph();
    if(!fMultiGraph) {delete gr; return NULL;}
    fPlotList->Add(fMultiGraph);
  }
  gr->SetName(name);
  gr->SetTitle(title);
  gr->SetLineWidth(dPlotOptions->lineWidth);
  gr->SetLineStyle(dPlotOptions->lineStyle);
  gr->SetLineColor(dPlotOptions->lineColor);
  gr->SetMarkerSize(dPlotOptions->markerSize);
  gr->SetMarkerStyle(dPlotOptions->markerStyle);
  gr->SetMarkerColor(dPlotOptions->markerColor);
  fGraphList->Add(gr);
  // fPlotList->Add(gr);
  fMultiGraph->Add(gr,"p");
  return gr;
}

TObject *RPlotContainer::GetPlot(Int_t index)
{
  return GetObject(index, NULL);
}

TObject *RPlotContainer::GetObject(Int_t index, Char_t *name)
{
  TObject *obj = NULL;
  if(!fPlotList) return NULL;
  if(fPlotList->GetSize() < 1) return NULL;
  if(index >= 0 && index < fPlotList->GetSize()){
    obj = fPlotList->At(index);
    if(obj){
      if(!name) return obj;
      if(name && !strcmp(obj->GetName(),name)) return obj;
      return NULL;
    }
    return NULL;
  }
  if(name){
    for(int i = 0; i < fPlotList->GetSize(); i++){
      obj = fPlotList->At(i);
      if(obj && !strcmp(obj->GetName(),name)) return obj;
    }
    return NULL;
  }
  return NULL;
}

TObject *RPlotContainer::GetHistogram(Int_t index, const Char_t *type, 
				      const Char_t *name)
{
  if(!type) return NULL;

  TObject *obj = NULL;
  if(!strcmp(type,"TH1D")){
    if(!fD1HistoList) return NULL;
    if(fD1HistoList->GetSize() < 1) return NULL;
    if(index >= 0 && index < fD1HistoList->GetSize()){
      obj = fD1HistoList->At(index);
      if(obj){
	if(!name) return obj;
	if(name && !strcmp(obj->GetName(),name)) return obj;
	return NULL;
      }
      return NULL;
    }
    if(name){
      for(int i = 0; i < fD1HistoList->GetSize(); i++){
	obj = fD1HistoList->At(i);
	if(obj && !strcmp(obj->GetName(),name)) return obj;
      }
      return NULL;
    }
    return NULL;
  }
  return NULL;
}

TObject *RPlotContainer::GetProfile(Int_t index, Char_t *type, 
				    Char_t *name)
{
  if(!type) return NULL;

  TObject *obj = NULL;
  if(!strcmp(type,"TProfile")){
    if(!fD1ProfileList) return NULL;
    if(fD1ProfileList->GetSize() < 1) return NULL;
    if(index >= 0 && index < fD1ProfileList->GetSize()){
      obj = fD1ProfileList->At(index);
      if(obj){
	if(!name) return obj;
	if(name && !strcmp(obj->GetName(),name)) return obj;
	return NULL;
      }
      return NULL;
    }
    if(name){
      for(int i = 0; i < fD1ProfileList->GetSize(); i++){
	obj = fD1ProfileList->At(i);
	if(obj && !strcmp(obj->GetName(),name)) return obj;
      }
      return NULL;
    }
    return NULL;
  }
  return NULL;
}

TObject *RPlotContainer::GetGraph(Int_t index, Char_t *type, 
				  Char_t *name)
{
  if(!type) return NULL;

  TObject *obj = NULL;
  if(!strcmp(type,"TGraph")){
    if(!fGraphList) return NULL;
    if(fGraphList->GetSize() < 1) return NULL;
    if(index >= 0 && index < fGraphList->GetSize()){
      obj = fGraphList->At(index);
      if(obj){
	if(!name) return obj;
	if(name && !strcmp(obj->GetName(),name)) return obj;
	return NULL;
      }
      return NULL;
    }
    if(name){
      for(int i = 0; i < fGraphList->GetSize(); i++){
	obj = fGraphList->At(i);
	if(obj && !strcmp(obj->GetName(),name)) return obj;
      }
      return NULL;
    }
    return NULL;
  }
  else if(!strcmp(type,"TGraphErrors")){
    if(!fGraphErList) return NULL;
    if(fGraphErList->GetSize() < 1) return NULL;
    if(index >= 0 && index < fGraphErList->GetSize()){
      obj = fGraphErList->At(index);
      if(obj){
	if(!name) return obj;
	if(name && !strcmp(obj->GetName(),name)) return obj;
	return NULL;
      }
      return NULL;
    }
    if(name){
      for(int i = 0; i < fGraphErList->GetSize(); i++){
	obj = fGraphErList->At(i);
	if(obj && !strcmp(obj->GetName(),name)) return obj;
      }
      return NULL;
    }
    return NULL;    
  }
  else if(!strcmp(type,"TGraphAsymmErrors")){
    if(!fGraphAsymErList) return NULL;
    if(fGraphAsymErList->GetSize() < 1) return NULL;
    if(index >= 0 && index < fGraphAsymErList->GetSize()){
      obj = fGraphAsymErList->At(index);
      if(obj){
	if(!name) return obj;
	if(name && !strcmp(obj->GetName(),name)) return obj;
	return NULL;
      }
      return NULL;
    }
    if(name){
      for(int i = 0; i < fGraphAsymErList->GetSize(); i++){
	obj = fGraphAsymErList->At(i);
	if(obj && !strcmp(obj->GetName(),name)) return obj;
      }
      return NULL;
    }
    return NULL;    
  }
  else if(!type){
    if(!fPlotList) return NULL;
    if(fPlotList->GetSize() < 1) return NULL;
    if(index >= 0 && index < fPlotList->GetSize()){
      obj = fPlotList->At(index);
      if(obj && obj->InheritsFrom("TGraph")){
	if(!name) return obj;
	if(name && !strcmp(obj->GetName(),name)) return obj;
	return NULL;
      }
      return NULL;
    }
    if(name){
      for(int i = 0; i < fPlotList->GetSize(); i++){
	obj = fPlotList->At(i);
	if(obj && obj->InheritsFrom("TGraph") && 
	   !strcmp(obj->GetName(),name)) return obj;
      }
      return NULL;
    }
    return NULL;    
  }
  return NULL;
}
