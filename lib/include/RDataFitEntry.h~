/////////////////////////////////////////////////////////////////////////////////////
//NPDGamma Experiment Data Visualization and Analysis
//
//Version 001
//
//RDataFitEntry header file, containing methods to select fit options
/////////////////////////////////////////////////////////////////////////////////////

#ifndef RDATAFITENTRY_H
#define RDATAFITENTRY_H

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <TVirtualX.h>
#include <RQ_OBJECT.h>
#include <TGClient.h>
#include <TGFrame.h>
#include <TGLabel.h>
#include <TGButton.h>
#include <TGNumberEntry.h>
#include <TGTextEntry.h>
#include <TGTextBuffer.h>
#include <TGComboBox.h>
#include <TSystem.h>
#include <TEnv.h>
#include "RDataFitEntryDef.h"


typedef struct FitOptions {

  Int_t    fullRange;
  Int_t    rootFit;
  char     func[200];
  double   min[3];
  double   max[3];
  double   param[10];
  Int_t    parset[10];
  Int_t    npars;
  Int_t    plot;
  double   stepsize[10];     
  double   chisqBound; 
  Color_t  lineColor;
  Style_t  lineStyle;
  Width_t  lineWidth;
  Color_t  markerColor;
  Style_t  markerStyle;
  Size_t   markerSize;

} FitOptions;


class RDataFitEntry : public TGTransientFrame {

private:

  Bool_t                dModifyOptions;
  Bool_t               *dSetflag;
  Int_t                 dMsgcnt;        //Number of messages in queue
  char                  dMiscbuffer[MSG_SIZE_MAX]; //For random use
  char                  dMiscbuffer2[MSG_SIZE_MAX];
  char                  dMessage[MSG_QUEUE_MAX][MSG_SIZE_MAX]; //Message in queue
  char                  dClass[NAME_STR_MAX]; 
  FitOptions           *dOpts;

  TGComboBox           *fComboBox;
  TGCompositeFrame     *f1,*f2,*f3,*f4;
  TGLayoutHints        *fL1,*fL2,*fL3,*fL4,*fL5;
  TGGroupFrame         *fG1,*fG2;
  TGHorizontalFrame    *fF[15];
  TGLabel              *fLabel[15];
  TGNumberEntry        *fNumericEntries[15];
  TGTextEntry          *fUfuncE;
  TGTextBuffer         *fUfuncB;
  TGButton             *fSetButton;
  TGButton             *fExitButton;
  TGCheckButton        *fC[12];
  TQObject             *dMrec;

  EButtonState          fCState[12];
  Int_t                 fNState[15];
  static const char *const numlabel[15];
  double                numinit[15];
  static const char *const functions[17];

  void                  DoSettings();
  Bool_t                SetMessage(const char *msg, 
				   const char *func,
				   Int_t TS, 
				   Int_t MESSAGETYPE); //Add message to queue
  void                  FlushMessages();//Get rid of all messages in queue

public:
  RDataFitEntry(const TGWindow *p, const TGWindow *main, 
		TQObject *mrec, const char *classname,
		FitOptions *opts, Bool_t Mod, UInt_t w, UInt_t h,
		UInt_t options = kVerticalFrame | kHorizontalFrame);
  virtual ~RDataFitEntry();

  void                 FChangeFocus();
  void                 SetFitOptions(char *);
  char                *GetMessage();
  virtual void         CloseWindow();
  virtual Bool_t       ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2);

  ClassDef(RDataFitEntry,0);

};

#endif
