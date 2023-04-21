////////////////////////////////////////////////////////////////////////////////
//
// name: CMMonitor.h
// date: 12-22-2022
// auth: Michael Gericke
// mail: Michael.Gericke@umanitoba.ca
//
// desc: This is a simple application that monitors and handles data from the
//       MOLLER ADC in pre-production diagnostic mode (meaning no special acqusition
//       software, such as the JLab CODA system has been implemented). The data is
//       sent by the ADC over ethernet and received by this application, utilizing the
//       the ZeroMQ library to handle the network traffic.
//
//       The application needs the ip address of the ADC board and the port (which is
//       fixed at 5555 on the ADC).
//
//       The application makes use of the ROOT analysis framework to display the data
//       and the RData library to provide a basic interface.
//
////////////////////////////////////////////////////////////////////////////////



#ifndef CMMONITOR_H
#define CMMONITOR_H

#include <TROOT.h>
#include <TApplication.h>
#include <TGMenu.h>
#include <TGClient.h>
#include <RDataContainer.h>
#include <RVegas.h>
#include <RDataWindow.h>
#include <RDataFit.h>
#include <RProcessLog.h>
#include <TString.h>
#include <TText.h>
#include <TH1D.h>
#include <TF1.h>
#include <TGraph.h>
#include <TRootEmbeddedCanvas.h>
#include <TGTab.h>
#include <TG3DLine.h>
#include <TVirtualFFT.h>
#include <TSlider.h>
#include <TFrame.h>

#include <string.h>
#include <time.h>
#include <iostream>
#include <iomanip>
#include <zmq.h>
#include <fstream>
#include <pthread.h>
#include <assert.h>
#include <signal.h>
#include <queue>
#include "CMMonitorDef.h"

using namespace std;

//*********************************************************************************
//The following are needed to convert from big-endian on ADC side, to little-endian on computer side
//This is no longer needed (MG, May 2022)

#define Swap4Bytes(val) ( (((val) >> 24) & 0x000000FF) | (((val) >>  8) & 0x0000FF00) |	(((val) <<  8) & 0x00FF0000) | (((val) << 24) & 0xFF000000) )

#define Swap8Bytes(val) ( (((val) >> 56) & 0x00000000000000FF) | (((val) >> 40) & 0x000000000000FF00) | (((val) >> 24) & 0x0000000000FF0000) | (((val) >>  8) & 0x00000000FF000000) | (((val) <<  8) & 0x000000FF00000000) | (((val) << 24) & 0x0000FF0000000000) | (((val) << 40) & 0x00FF000000000000) | (((val) << 56) & 0xFF00000000000000) )

//Package as send by ADC board (in big-endian)
// struct pktBigE{
//   uint64_t ts;
//   uint64_t ch0cnt;
//   uint64_t ch0sum;
//   uint64_t ch1cnt;
//   uint64_t ch1sum;
//   uint32_t ch0smp;
//   uint32_t ch1smp;
// };

//little-endian converted ADC data
// struct pkt{
//   uint64_t ts;
//   uint64_t ch0cnt;
//   int64_t ch0sum;
//   uint64_t ch1cnt;
//   int64_t ch1sum;
//   int32_t ch0smp;
//   int32_t ch1smp;
// };

//*********************************************************************************


struct rawPkt{

  uint8_t *data;
  size_t length;

};

struct pkt{
  uint64_t ts;
  int32_t ch0;            
  int32_t ch1;            
  int32_t ch0_data;       
  int32_t ch1_data;       
  uint32_t ch0_num;       
  uint32_t ch1_num;       
  uint32_t PreSc;
};


class DataSamples{

public:

  DataSamples(){};
  virtual ~DataSamples(){};
  
  vector<double> tStmp;
  vector<double> ch0_data;
  vector<double> ch1_data;
  vector<uint32_t> gate1;
  vector<uint32_t> gate2;
  vector<double> ch0_asym;
  vector<double> ch0_asym_num;
  vector<double> ch0_asym_den;  
  vector<double> ch1_asym;
  vector<double> ch1_asym_num;
  vector<double> ch1_asym_den;
  uint32_t PreScF;
  uint32_t ch0_num;
  uint32_t ch1_num;
  double ch0_sum;
  double ch1_sum;
  double ch0_ssq;
  double ch1_ssq;
  double ch0_mean;
  double ch1_mean;
  double ch0_sig;
  double ch1_sig;
  double RunLength;
  uint64_t NSamples;
  int Run;

  ClassDef(DataSamples,1)
} ;


struct rArgs{

  string FName;
  int NSamples;
  void *sock;
  rawPkt* pkt;

};

struct Settings{

  string IP;
  int currentRun;
  int currentData0;
  int currentData1;
  int PreScFactor;
  double RunLength;
  
};


static volatile int wait_for_shared_socket = 0;


class CMMonitor : public TGMainFrame {

  RQ_OBJECT("CMMonitor");

private:

  int                     dMWWidth;
  int                     dMWHeight;

  TGMenuBar              *dMenuBar;
  TGPopupMenu            *dMenuFile;
  TGPopupMenu            *dMenuSettings;
  TGPopupMenu            *dMenuTools;
  TGPopupMenu            *dMenuHelp;

  TGTab                  *dTab;

  TGHorizontalFrame      *dUtilityFrame;
  TGHorizontalFrame      *dGateFrame;
  TGNumberEntry          *dRunEntry;
  TGLabel                *dRunEntryLabel;
  TGNumberEntry          *dSmplDivEntry;
  TGNumberEntry          *dNumRunSeqEntry;
  TGLabel                *dSmplDivEntryLabel;
  // TGHorizontalFrame      *dRunTimeInfoFrame;
  TGNumberEntry          *dRunTimeEntry;
  TGLabel                *dRunTimeEntryLabel;
  TGLabel                *dChanLabel[NUM_CHANNELS];
  TGLabel                *dAsymChanLabel[NUM_CHANNELS];
  TGComboBox             *Ch0ListBox;
  TGComboBox             *Ch1ListBox;
  TGCheckButton          *dGChkB1;
  TGCheckButton          *dGChkB2;
  
  
  TRootEmbeddedCanvas    *dCurrMSmplGrCv[NUM_CHANNELS];
  TRootEmbeddedCanvas    *dCurrMSmplHstCv[NUM_CHANNELS];
  TRootEmbeddedCanvas    *dCurrMSmplHstHRCv[NUM_CHANNELS];
  TRootEmbeddedCanvas    *dCurrMSmplFFTCv[NUM_CHANNELS];

  TRootEmbeddedCanvas    *dAsymMHstCv[NUM_CHANNELS];
  TRootEmbeddedCanvas    *dAsymMGrCv[NUM_CHANNELS];
    
  TGVerticalFrame        *vLabF[NUM_CHANNELS];
  TGVerticalFrame        *vAsymLabF[NUM_CHANNELS];
  TGLabel                *dRateCounter[NUM_CHANNELS];
  TGHorizontalFrame      *dRateFrame[NUM_CHANNELS];

  RProcessLog            *processLog;
  RDataWindow            *dataWindow;
  RDataContainer         *dataCont, *dataCont2;
  RDataFit               *dataFit;

  ifstream               *SettingsFile;
  ofstream               *SettingsOutFile;
  
  TH1D                   *ChSigHst[NUM_CHANNELS];
  TH1D                   *ChSigHstHR[NUM_CHANNELS];
  TGraph                 *ChSigGr[NUM_CHANNELS];
  TProfile               *ChSigPr[NUM_CHANNELS];

  TH1D                   *ChAsymHst[NUM_CHANNELS];
  TGraph                 *ChAsymGr[NUM_CHANNELS];
  
  TRootEmbeddedCanvas    *dGateCv[2];  //The two TTL gates at the back of the ADC module 
  TGraph                 *GateGr[2];


  TH1D                   *fftTmpCh0;
  TH1D                   *fftTmpCh1;
  
  TProfile               *fftCh0;
  TProfile               *fftCh1;

  
  TTree                  *DataTree;
  TFile                  *DataRooFile;
  TString                 ROOTFileName;
  // TProfile               *fftGainCh0;
  // TProfile               *fftGainCh1;
    
  string                  IP;
  string                  server;
  void                   *context;
  void                   *socket;
  pthread_t               thread_cap_id;
  pthread_t               thread_plot_id;

  queue<rawPkt*>          dataQue;
  rawPkt                 *aData;
  uint32_t                ReadNSamples;
  vector<DataSamples*>    PlotData;
  DataSamples            *tmpDataSmpl;

  double                  RunLength;
  int                     CurrentRun;
  int                     dNRunsSeq;
  int                     dNRunSeqCnt;

  char                    Data0;
  char                    Data1;

  string                  SamplesOutFileName;
  string                  DataFileName;

  rArgs                  *readThreadArgs;  

  Bool_t                  RUN_START;
  Bool_t                  RUN_STOP;
  Bool_t                  RUN_PAUSE;
  Bool_t                  RUN_ON;
  
  Int_t                   PrescaleFac;
  Double_t                RunStartTime;
  Double_t                RunStartIndex;
  

  Settings                iSettings;
  
  Bool_t                  dDataFileOpen;
  Bool_t                  dRootFileOpen;
  Bool_t                  dGateOvl1;
  Bool_t                  dGateOvl2;
  Bool_t                  dHightResHR;

  TSlider                *xslider1;
  TSlider                *xslider2;
  
  void                    SetIP();
  void                    MakeCurrentModeTab();
  void                    MakeAsymmetryTab();
  void                    MakeMenuLayout();
  void                    MakeUtilityLayout();
  void                    MakeGateLayout();
  

  Bool_t                  ConnectBoard();
  // void                    GetServerData(queue<pkt*>*, pkt*, Bool_t*);
  static void            *GetServerData(void *vargp);
  void                    DisconnectBoard();
  void                    FillDataPlots();
  void                    FillRootTree();
  void                    StartDataCollection();
  void                    PlotDataGraph(int ovl = 0);
  void                    PlotDataHRHst();
  Int_t                   CalculateFFT();
  void                    WriteSettings();
  Int_t                   OpenDataFile(ERFileStatus status = FS_OLD, const char* file = NULL);
  Int_t                   OpenRootFile(ERFileStatus status = FS_OLD, const char* file = NULL);
  Bool_t                  AddFFT(TH1D *fftTmp, DataSamples *data, int smplSt, double mean, double RATE, int ch, int smpls, int bins);
  void                    PlotDataFFT();
  Int_t                   GetFilenameFromDialog(char *, const char *,
						ERFileStatus status = FS_OLD,
						Bool_t kNotify = kFalse,
						 const char *notifytext = NULL);
  void                    SetDataFileOpen(Bool_t open = kFalse){dDataFileOpen = open;};
  Bool_t                  IsDataFileOpen(){return dDataFileOpen;};
  Bool_t                  IsRootFileOpen(){return dRootFileOpen;};
  void                    SetRootFileOpen(Bool_t open = kFalse){dRootFileOpen = open;};
  void                    CloseRootFile();

  void                    SetDataFileName(const char *name){DataFileName = name;};
  void                    ClearData();
  void                    ClearPlots();
  void                    CloseDataFile();
  Int_t                   SaveDataFile(ERFileStatus status, const char* file);
  virtual Bool_t          ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2);

  void                    ExecuteEvent(Int_t event, Int_t px, Int_t py);
  
public:
  CMMonitor(const TGWindow *p, UInt_t w, UInt_t h);
  virtual ~CMMonitor();
  
  virtual void CloseWindow();

  void OnObjClose(char *obj);
  void OnReceiveMessage(char *obj){};
  void PadIsPicked(TPad* selpad, TObject* selected, Int_t event);
  void MainTabEvent(Int_t,Int_t,Int_t,TObject*);
  
  //std::thread spawn() {
  //  return std::thread(&CMMonitor::GetServerData,this);
  //};
  
  ClassDef(CMMonitor,0);
};


#endif
