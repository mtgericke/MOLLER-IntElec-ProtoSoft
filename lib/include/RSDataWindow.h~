/////////////////////////////////////////////////////////////////////////////////////
//Root Based Data Visualization and Analysis Utility Library
//
//Version 000
//
//RSDataWindow header file, declaring basic methods to display data in a window.
//The actual window for data viewing needs to be derived from this class.
//Methods to display data in a specific way need to be specified in derived class.
/////////////////////////////////////////////////////////////////////////////////////


#ifndef RSDATAWINDOW_H
#define RSDATAWINDOW_H

#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <vector>
using std::vector;

#include <TVirtualX.h>
#include <TGClient.h>
#include <TGFrame.h>
#include <TGButton.h>
#include <TGMenu.h>
#include <TGCanvas.h>
#include <TGMsgBox.h>
#include <TGFileDialog.h>
#include <TRootEmbeddedCanvas.h>
#include <TLegend.h>
#include <TString.h>
#include <TRootCanvas.h>
#include <TCanvas.h>
#include <TSystem.h>
#include <TKey.h>
#include <TList.h>
#include <TTimer.h>
#include <TView.h>
//#include <TMath.h>
#include "RComboBoxDialog.h"
#include "RNumberEntryDialog.h"
#include "RStringEntryDialog.h"
#include "RDataWindowDef.h"
#include "RDataContainer.h"
#include "RDataFit.h"
#include "RPlotContainer.h"
#include <string.h>

class RSDataWindow : public TGTransientFrame { //TQObject {

 private:
  
  Int_t                dDrawpat;
  Int_t                dMsgcnt;        //Number of messages in queue
  Int_t                dFitHistocnt;
  char                 dMainName[NAME_STR_MAX]; 
  char                 dReceiverName[NAME_STR_MAX]; 
  char                 dObjName[NAME_STR_MAX]; 
  char                 dMiscbuffer[MSG_SIZE_MAX]; //For random use
  char                 dMiscbuffer2[MSG_SIZE_MAX];
  char                 dDatatitle[NAME_STR_MAX];
  char                 dPlottitle[NAME_STR_MAX];
  char                 dLegEntry[NAME_STR_MAX];  
  char                 dLegHeader[NAME_STR_MAX];
  char                 dPlottitleX[NAME_STR_MAX];
  char                 dPlottitleY[NAME_STR_MAX];
  char                 dPlottitleZ[NAME_STR_MAX];
  char                 dPlotname[NAME_STR_MAX];
  char                 dMessage[MSG_QUEUE_MAX][MSG_SIZE_MAX]; //Message in queue
  char                 dDrawOptions[NAME_STR_MAX];
/*   char                 dSelectedRootFileObj[NAME_STR_MAX]; */
  Bool_t               dDrawOptsSet;
  Bool_t               dAddGraphs;
  Bool_t               dUpdateHisto;
  Bool_t               dIntRootCont;
  Bool_t               dLimits;
  Double_t             dMin[3];
  Double_t             dMax[3];
  Double_t             dTitleOffset[3];
  Int_t                dNumbins[3];  

  ERPlotTypes          dPtype;
  RPlotContainer      *dPlotCont;
  RDataContainer      *dRootCont;
  RDataContainer      *dMiscCont;
  RDataFit            *dDatafit;
  RComboBoxDialog     *dROOTFileObjDlg;
  FitOptions          *dFitOptions;

  TGPopupMenu         *fMenuFile;
  TGPopupMenu         *fMenuTools;
  TGMenuBar           *fMenuBar;
  TGCompositeFrame    *fFrame1;
  TGCompositeFrame    *fDlgFrame, *fCnvFrame;
  TGButton            *fOkButton, *fCancelButton;
  TGLayoutHints       *fL1, *fL2, *fDlgLayout, *fCnvLayout;
  TGLayoutHints       *fMenuBarItemLayout, *fMenuBarLayout;
  TCanvas             *fPlotCanvas;
  TGWindow            *fMrec;
  const TGWindow            *fParent;
  const TGWindow            *fMain;
  TLegend             *fLegend;
  TObject             *fCurrPlot;

  TH1D                *fFitHisto[10];
  TH1D                *dDummyPlot;
  vector <TBox*>       dErrorBoxArray;

  void                 SleepWithEvents(int sec);

 protected:

  void                 ClearPlots();

  void                 DivideWindowData();
  void                 FlushMessages();//Get rid of all messages in queue
  Int_t                FitData();
  Int_t                GetGraphBinRange(TGraph *,Double_t min, Double_t max, 
					Int_t *minbin, Int_t *maxbin);
  void                 GetGraphData(TGraph *,Double_t *x,Double_t *y);
  void                 GetGraphData(TGraphErrors *,Double_t *x,Double_t *y, 
				    Double_t *xe, Double_t *ye);
  void                 GetGraphMinMax(TGraph *,Double_t *xmin,Double_t *xmax,
				      Double_t *ymin,Double_t *ymax);
  Int_t                GetNewLineColor(Color_t col = 0);
  Int_t                GetNewMarkerColor(Color_t col = 0);
  TCanvas             *GetPlotCanvas() {return fPlotCanvas;};

  Bool_t               IsRootContInternal(){return dIntRootCont;};
  Bool_t               IsUserLimitSet(){return dLimits;};

  Double_t             minv(Double_t *x, Int_t size);
  Double_t             maxv(Double_t *x, Int_t size);
  void                 NormalizeWindowData();
  Int_t                OpenContainer();
  Int_t                OpenRootFile();

  void                 PrintCanvas();

  void                 SaveCanvas(const char* file = NULL);
  void                 SavePlotObjects();
  void                 ScaleWindowData();
  Bool_t               SetMessage(const char *msg, const char *func,Int_t TS,Int_t MESSAGETYPE);
  
 public:

  RSDataWindow(const TGWindow *p, const TGWindow *main,
	      const char * objName, const char *mainname,
	      const char *datatitle, ERPlotTypes type,
	      UInt_t w, UInt_t h,
	      UInt_t options = kVerticalFrame);
  virtual ~RSDataWindow();

  Bool_t               AddGraphs(){return dAddGraphs;};
  void                 AddMenuPopup(const char*,TGPopupMenu*);
  void                 AddPopupEntry(const char*,Int_t ID, TGPopupMenu* pop = 0);

  void                 CloseConnection(); 
  virtual void         CloseWindow();
  void                 ConnectWith(const TGWindow *, char *); 

  Int_t                DrawData(const TH2D&);
  Int_t                DrawData(const TProfile&);
  Int_t                DrawData(const TF1&, Bool_t add = kFalse);
  Int_t                DrawData(const TH1D&, Bool_t add = kFalse);
  Int_t                DrawData(const TGraph&, Bool_t add = kFalse, TLegend *leg = NULL);
  Int_t                DrawData(const TGraphErrors&, Bool_t add = kFalse, TLegend *leg = NULL);
  Int_t                DrawData(const TGraphAsymmErrors&, Bool_t add = kFalse, TLegend *leg = NULL);
  Int_t                DrawData(const TMultiGraph&, Bool_t add = kFalse, TLegend *leg = NULL);
  Int_t                DrawData(Double_t*,Int_t, Bool_t add = kFalse, TLegend *leg = NULL);
  Int_t                DrawData(Double_t*,Double_t*,Int_t, Bool_t add = kFalse, TLegend *leg = NULL);
  Int_t                DrawData(Double_t*,Double_t*,Double_t*,Int_t, Bool_t add = kFalse, TLegend *leg = NULL);
  Int_t                DrawLegend(TLegend *leg = NULL);
  Int_t                DrawBox(const TBox&);
  Bool_t               DrawOptionsSet(){return dDrawOptsSet;};

  Double_t             GetAxisMax(Int_t a = 0);
  Double_t             GetAxisMin(Int_t a = 0);
  char                *GetDataTitle() {return dDatatitle;};
  char                *GetFitLog();
  char                *GetMainName(){return dMainName;};
  char                *GetMessage();
  Int_t                GetNumBins(Int_t a = 0);
  Int_t                GetNumOfPlots(){return dPlotCont->GetPlotCount();};
  char                *GetObjectName(){return dObjName;};
  void                 GetObj(TObject* obj);
  Int_t                GetPlotCount();
  char                *GetPlotName() {return dPlotname;};
  char                *GetPlotTitle()  {return dPlottitle; };
  char                *GetPlotTitleX() {return dPlottitleX;};
  char                *GetPlotTitleY() {return dPlottitleY;};
  char                *GetPlotTitleZ() {return dPlottitleZ;};
  Int_t                GetPlotType() {return dPtype;};
  void                 GetRootObjectList(RDataContainer*);

  void                 IntegrateWindowData();
  void                 IsClosing(char *); //Emit signal that this object is closing

  void                 OnObjClose(char *obj);
  void                 OnReceiveMessage(char *obj){};
  void                 OnSelectedEntry(char*);

  virtual Bool_t       ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2);
  void                 ViewMiscData();

  void                 SendMessageSignal(char*);//Emit signal when message is posted
  void                 SetAddGraphs(Bool_t add = kTrue){dAddGraphs = add;};
  void                 SetAxisMin(Double_t xmin=0, Double_t ymin=0, Double_t zmin=0);
  void                 SetAxisMax(Double_t xmax=0, Double_t ymax=0, Double_t zmax=0);
  void                 SetDefaultPlotOptions();
  void                 SetDrawOptions(const char *opts = NULL);
  void                 SetLegendEntry(char* entry ){strcpy(dLegEntry,  entry);};  
  void                 SetLegendHeader(char* header ){strcpy(dLegHeader,header);};
  void                 SetLimits(Int_t,Int_t,Int_t,TObject*);
  void                 SetLimitsFlag(Bool_t lim){dLimits = lim;};
  void                 SetLineColor(Color_t col);
  void                 SetLineStyle(Style_t sty);
  void                 SetLineWidth(Width_t wid);
  void                 SetMarkerColor(Color_t col);
  void                 SetMarkerSize(Size_t sze);  
  void                 SetMarkerStyle(Style_t sty);
  void                 SetNumBins(Int_t xbins=10000, Int_t ybins=0, Int_t zbins=0);
  void                 SetPlotName(char *name) {strcpy(dPlotname,name);};
  void                 SetPlotTitle(const char* title ) {strcpy(dPlottitle, title);}; 
  void                 SetPlotTitleX(const char* title) {strcpy(dPlottitleX,title);};
  void                 SetPlotTitleY(const char* title) {strcpy(dPlottitleY,title);};
  void                 SetPlotTitleZ(const char* title) {strcpy(dPlottitleZ,title);};
  void                 SetPlotTitleOffset(Double_t xoffs = 0, Double_t yoffs = 0, Double_t zoffs = 0);
  void                 SetRootContainer(RDataContainer *cont = NULL);
  void                 SetUpdateFalse(){dUpdateHisto = kFalse;};
  void                 SetUpdateTrue(){dUpdateHisto = kTrue;};

  Int_t                UpdateDrawData(TH1D*);

  void                 WriteMiscData();

  static Double_t      OneMinusCos(Double_t *x, Double_t *parm);
  static Double_t      APlusBExp(Double_t *x, Double_t *parm);

  ClassDef(RSDataWindow,0);
};

#endif 
