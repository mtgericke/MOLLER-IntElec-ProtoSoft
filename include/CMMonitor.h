////////////////////////////////////////////////////////////////////////////////
//
// name: CMMonitor.h
// date: 5-24-2021
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
//       The application make suse of the ROOT analysis framework to display the data
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

#include <string.h>
#include <time.h>
#include <iostream>
#include <zmq.hpp>
#include <fstream>
#include <thread>
#include <signal.h>
#include "CMMonitorDef.h"

using namespace std;

//*********************************************************************************
//The following are needed to convert from big-endian on ADC side, to little-endian on computer side

#define Swap4Bytes(val) ( (((val) >> 24) & 0x000000FF) | (((val) >>  8) & 0x0000FF00) |	(((val) <<  8) & 0x00FF0000) | (((val) << 24) & 0xFF000000) )

#define Swap8Bytes(val) ( (((val) >> 56) & 0x00000000000000FF) | (((val) >> 40) & 0x000000000000FF00) | (((val) >> 24) & 0x0000000000FF0000) | (((val) >>  8) & 0x00000000FF000000) | (((val) <<  8) & 0x000000FF00000000) | (((val) << 24) & 0x0000FF0000000000) | (((val) << 40) & 0x00FF000000000000) | (((val) << 56) & 0xFF00000000000000) )

//*********************************************************************************

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
		        
  TRootEmbeddedCanvas    *dCurrMSmplGrCv[NUM_CHANNELS];
  TRootEmbeddedCanvas    *dCurrMSmplHstCv[NUM_CHANNELS];
  TRootEmbeddedCanvas    *dCurrMSmplFFTCv[NUM_CHANNELS];
  
  TGVerticalFrame        *vLabF[NUM_CHANNELS];
  TGLabel                *dRateCounter[NUM_CHANNELS];
  TGHorizontalFrame      *dRateFrame[NUM_CHANNELS];


  RProcessLog            *processLog;
  RDataWindow            *dataWindow;
  RDataContainer         *dataCont, *dataCont2;
  RDataFit               *dataFit;
  TH1D                   *hst,*hstf;

  void                    CountTest();
  void                    MakeCurrentModeTab();
  void                    MakeMenuLayout();
  virtual Bool_t          ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2);

public:
  CMMonitor(const TGWindow *p, UInt_t w, UInt_t h);
  virtual ~CMMonitor();
  
  virtual void CloseWindow();

  void OnObjClose(char *obj);
  void OnReceiveMessage(char *obj){};
  void PadIsPicked(TPad* selpad, TObject* selected, Int_t event);
  void MainTabEvent(Int_t,Int_t,Int_t,TObject*);

  ClassDef(CMMonitor,0);
};


#endif
