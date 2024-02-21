////////////////////////////////////////////////////////////////////////////////
//
// name: CMData.h
// date: 11-13-2023
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



#ifndef CMDATA_H
#define CMDATA_H

#include <TROOT.h>
#include <TApplication.h>
#include <TSystem.h>
#include <TFile.h>
#include <TString.h>
#include <TTree.h>
#include <cerrno>

#include <string.h>
#include <time.h>
#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include "/usr/local/include/zmq.h"
#include <fstream>
#include <pthread.h>
#include <assert.h>
#include <signal.h>
#include <queue>
#include <vector>
#include "CMMonitorDef.h"

using namespace std;

//*********************************************************************************


struct rawPkt{

  uint8_t *data;
  size_t length;
  uint32_t convClk;
  Int_t run;

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

enum SockType {CNTRL,DATA};
enum ActType {READ,WRITE};


class tDataSamples{

public:

  tDataSamples(){ };
  virtual ~tDataSamples(){ };
  
  vector<double> tStmp;
  vector<double> tStmpDiff;
  vector<double> tStmpDiffTime;
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

  ClassDef(tDataSamples,1)
} ;


struct rArgs{
  string FName;
  int NSamples;
  void *sock;
  rawPkt* pkt;
};

class CMData;
struct fArgs{
  //string FName;
  //int NSamples;
  //void *sock;
  queue<rawPkt*> *mQue;
  CMData *mExe;
  Bool_t wReduced;
  tDataSamples *dSamples;
  TTree *tree;
  int nRuns;
  // Int_t  *rStartInd;
  // uint64_t  rStartTime;
};

struct Settings{

  string IP;
  int currentRun;
  int currentData0;
  int currentData1;
  int PreScFactor;
  double RunLength;
  int SamplingDelay;
  
};


static volatile int wait_for_shared_socket = 0;

//#define ZMQ_HAVE_POLLER
//#define ZMQ_BUILD_DRAFT_API


class CMData {

private:
  
  ifstream               *SettingsFile;
  ofstream               *SettingsOutFile;
  
  TTree                  *DataTree;
  TFile                  *DataRootFile;
  TString                 ROOTFileName;
    
  string                  IP;
  string                  server;
  void                   *context;
  void                   *cntr_socket;
  void                   *data_socket;
  pthread_t               thread_cap_id;
  pthread_t               thread_plot_id;

  queue<rawPkt*>          dataQue;
  rawPkt                 *aData;
  uint32_t                ReadNSamples;
  vector<tDataSamples*>    PlotData;
  tDataSamples            *tmpDataSmpl;

  double                  RunLength;
  int                     CurrentRun;
  int                     dNRunsSeq;
  int                     dNRunSeqCnt;

  char                    Data0;
  char                    Data1;

  string                  SamplesOutFileName;
  string                  DataFileName;

  rArgs                  *readThreadArgs;  
  fArgs                  *fillThreadArgs;  

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
  Bool_t                  dRootFileWriteReduced;
  
  uint32_t               cntrMsg[3];
  
  void*                   GetSocket(SockType type);
  Bool_t                  ADCMessage(ActType type, void* socket, uint32_t addr, uint32_t data, uint32_t *msgret);         
  //Bool_t                  ConnectBoard();
  // void                    GetServerData(queue<pkt*>*, pkt*, Bool_t*);
  static void            *GetServerData(void *vargp);
  void                    DisconnectBoard();
  static void            *FillRootTreeThread(void *vargp);
  void                    FillRootTree();
  void                    StartDataCollection();
  void                    WriteSettings();
  Int_t                   OpenRootFile(const char* file = NULL);
  Bool_t                  IsDataFileOpen(){return dDataFileOpen;};
  Bool_t                  IsRootFileOpen(){return dRootFileOpen;};
  void                    SetRootFileOpen(Bool_t open = kFalse){dRootFileOpen = open;};
  void                    CloseRootFile();

  void                    SetDataFileName(const char *name){DataFileName = name;};
  void                    CloseDataFile();
  Int_t                   SaveDataFile(ERFileStatus status, const char* file);
  TTree                  *GetDataTree() {return DataTree;};
  
public:
  CMData(int *argc, char **argv);
  virtual ~CMData();
  //ClassDef(CMData,0);
};


#endif
