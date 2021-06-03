/////////////////////////////////////////////////////////////////////////////////////
//NPDGamma Experiment Data Visualization and Analysis
//
//Version 2.01
//
//RComboBoxDialog header file, containing methods to monitor analyzing
//progress
/////////////////////////////////////////////////////////////////////////////////////


#ifndef RMSGBOX_H
#define RMSGBOX_H

#include <TGMsgBox.h>
#include <TSystem.h>
#include "TGButton.h"
#include "TGIcon.h"
#include "TGLabel.h"
#include "TMath.h"
#include "TList.h"
#include "RDataWindowDef.h"

class RMsgBox : public TGTransientFrame {

 private:

  char               dObjName[NAME_STR_MAX];
  char               dMainName[NAME_STR_MAX]; 

 protected:
   TGButton            *fYes, *fNo, *fOK, *fApply;   // buttons in dialog box
   TGButton            *fRetry, *fIgnore, *fCancel;  // buttons in dialog box
   TGButton            *fClose, *fDismiss;           // buttons in dialog box
   TGIcon              *fIcon;                       // icon
   TGHorizontalFrame   *fButtonFrame;                // frame containing buttons
   TGHorizontalFrame   *fIconFrame;                  // frame containing icon and text
   TGVerticalFrame     *fLabelFrame;                 // frame containing text
   TGLayoutHints       *fL1, *fL2, *fL3, *fL4, *fL5; // layout hints
   TList               *fMsgList;                    // text (list of TGLabels)
   Int_t               *fRetCode;                    // address to store return code

   void PMsgBox(const char *title, const char *msg, const TGPicture *icon,
                Int_t buttons, Int_t *ret_code);

 public:

  RMsgBox(const TGWindow *p, const TGWindow *main,
	  const char * objName, const char *mainname,
	  const char *title, const char *msg, EMsgBoxIcon icon,
	  Int_t buttons = kMBDismiss, Int_t *ret_code = 0,
	  UInt_t sleep = 0, UInt_t options = kVerticalFrame);
  
  virtual ~RMsgBox();

  virtual void   CloseWindow();
  virtual Bool_t ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2);
  void           IsClosing(char *objname);
  void           UpdateText(const char *text);

  ClassDef(RMsgBox,0);
};

#endif
