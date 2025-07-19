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

struct runData {

  queue<rawPkt*>  dQue;
  int run;
  uint32_t convClk;
  string FName;
  int NSamples;
 
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

enum SockType {CNTRL,INTEG,STREAM};
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
  runData *rDat;
  rawPkt* pkt;
  SockType socktype;
  char dType[4];
};

class CMData;
struct fArgs{
  //string FName;
  //int NSamples;
  //void *sock;
  queue<rawPkt*> *mQue;
  queue<runData*> *rQue;
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
  uint32_t currentData0;
  uint32_t currentData1;
  uint32_t PreScFactor;
  double RunLength;
  int ReversalFreq;
  uint32_t SamplingDelay;
  uint32_t IntMode;   //toggles integration and streaming mode : 1 = integration on
  uint32_t IntModeSingle;   //enables single integration region  (e.g. the enitre helicity window)
  uint32_t streamAll;   //enable streaming all 16 channels, but at much reduced sample factor
  uint32_t pockSettle;  //sets the pockels cell settle time delay in  8 ns intervals
  uint32_t totalSamplingDelay;   //sum of the pockles cell and additional adc/sampling delay in 8 ns intervals
  uint32_t numBlocks;   //number of blocks in helcity window
  uint32_t blockSize;   // length of each block currently in terms of 8 ns clock cycles
  

  //example:
  //with window  = 510 us (currently hard coded in FW)
  //totalSamplingDelay = 1250 ( = 10 us )
  //numBlocks  = 4
  //blockSize  = 15625 (max)  from 500000 ns / 4 / 8 ns

  //but this means that we would have 125000 ns / 68 ns -> 1838 samples per block = 124984 ns (two clock cycles less)
  //do we have to shift by 16 ns in this case, before taking the next block of data ?
  //do we want to specify the block length in terms of number of samples and then calculate how long of a delay
  //before starting the next block of averaging?

  //No gap between blocks
  //Pockels cell delay handled by TI
  //Blocks the same size each time!
  //Use number of samples to specify block length as well as the entire helicity window.
  //We don't want any additional delay within the ADC board (unless we explicitly set it up that way)
  //If so desired we may want to have an additional delay specified at the beginning of the first block in terms of number of samples (if possible).

  

};

/***********************************************************************************
// stream_ctrl 0x44 0x80002000   Ena[31]ratediv[30:24]ch1[23:20]ch0[19:16]nsamp[15:0]
wire [47:0] spare_reg1; // 0x104 // capture/stream modifications controls
                        //          bit0:  set to enable Averaging          
                        //          bit1:  set to enable fixed single[~4k] region, otherwise use spare-reg2
                        //          bit2:  set to enable fixed enable/start delays, otherwise use spare-reg1
                        //          bit3:  set to enable streaming all adcs, otherwise just two selected adcs
                        //          bits 19- 8: enable-delay [from trigger to enable going high]
                        //          bits 31-20: start-delay  [from trigger to start first going high]
wire [31:0] spare_reg2; // 0x108 // average #regions and region-size
                        //          bits  3: 0 - number of averaging regions
                        //          bits 31:15 - averaging region size
/***********************************************************************************/


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
  queue<runData*>        runQue;

  
  rawPkt                 *aData;
  //uint32_t                ReadNSamples;
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
  //  void                    FillRootTree();
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
  long                   GetPacketFrameSize();
  
public:
  CMData(int *argc, char **argv);
  virtual ~CMData();
  //ClassDef(CMData,0);
};


#endif
