#ifndef RPROGRESSDIALOG_H
#define RPROGRESSDIALOG_H

#include <stdlib.h>
#include <math.h>
#include <TVirtualX.h>
#include <TGClient.h>
#include <TGFrame.h>
#include <TSystem.h>
#include <TGButton.h>
#include <TEnv.h>
#include <TGProgressBar.h>
#include "RProgressDialogDef.h"
#include <string.h>

class RProgressDialog : public TGTransientFrame {

private:

  char               dObjName[NAME_STR_MAX];
  char               dMainName[NAME_STR_MAX]; 
  int                dRuncnt;
  int                dMpcnt;
  char               dTemp[500];
  Bool_t             dContinue;
  Bool_t             dStop;
  Bool_t             kStopable;

  TGHorizontalFrame *fHframe1;
  TGVerticalFrame   *fVframe1;
  TGLayoutHints     *fHint2, *fHint3, *fHint4, *fHint5;
  TGHProgressBar    *fHProg1, *fHProg2;
  TGTextButton      *fStop, *fPause, *fResume;
  
 public:
  RProgressDialog(const TGWindow *p, const TGWindow *main, 
		  const char* objname, const char *mainname,
		  const char* title, const char* macrotext, 
		  const char* microtext,
		  int runcnt, int mpcnt ,UInt_t w, UInt_t h, 
		  Bool_t stopable = kTrue);
  virtual ~RProgressDialog();
  
  Bool_t            Continue() {return dContinue;}
  Bool_t            Stop() {return dStop;}
  virtual void      CloseWindow();
  virtual Bool_t    ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2);
  void              IsClosing(char *);
  void              HaltProcess();
  void              ResetRange(int,int);
  void              SetTitle(char *title);

  ClassDef(RProgressDialog,0);
};

#endif
