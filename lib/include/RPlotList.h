/////////////////////////////////////////////////////////////////////////////////////
//Root Based Data Visualization and Analysis Utility Library
//
//Version 000
//
//RPlotList header file
/////////////////////////////////////////////////////////////////////////////////////


#ifndef RPLOTLIST_H
#define RPLOTLIST_H

#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <TObject.h>
#include <TList.h>
#include <TString.h>
#include <string.h>
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
#include "RPlotListDef.h"

struct PlotOptions {
  Double_t min[3];        /*axis-range min*/
  Double_t max[3];        /*axis-range max*/
  Double_t hmax;          /*histogram max y range*/
  Int_t    numbins[3];    /*axis-range number of bins*/
  Bool_t   limitsFlag;    /*raise if limits have been changed*/
  char     options[50];
  Color_t  lineColor;
  Style_t  lineStyle;
  Width_t  lineWidth;
  Color_t  markerColor;
  Style_t  markerStyle;
  Size_t   markerSize;
};

struct ListElement {

  ListElement         *PrevElement;
  ListElement         *NextElement;
  
  TGraph              *fGraph;      
  TGraphErrors        *fGraphEr;
  TGraphAsymmErrors   *fGraphAsymEr;
  TH1S                *fS1Histo;
  TH1F                *fF1Histo;
  TH1D                *fD1Histo;
  TH2S                *fS2Histo;
  TH2F                *fF2Histo;
  TH2D                *fD2Histo;
  TH3S                *fS3Histo;
  TH3F                *fF3Histo;
  TH3D                *fD3Histo;     
  TF1                 *fF1Func;
  TMultiGraph         *fMGraph;     

  char                 dType[50];
  int                  dIndex;
};

class RPlotList {

 public:

  RPlotList();
  ~RPlotList();
  TObject      *GetNewPlotObjectPtr(Char_t *type);
  TObject      *operator[](Int_t i){ return GetPlotObject(i);};
  TObject      *GetPlotObject(Int_t);
  TObject      *GetPlotObject(Char_t*);
  int           ObjCount() {return dCount;};
  void          EmptyList();

 private:

  void          Cleanup(ListElement*);
  void          SetElement(ListElement*,Char_t *type);
  TObject      *GetObject(ListElement*);
  int           dCount;
  
  int           dErrVal;

  ListElement*  FirstElement;
  ListElement*  LastElement;
  ListElement*  CurrentElement;
  ListElement*  NewElement;
};

#endif 
