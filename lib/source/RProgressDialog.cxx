#include "RProgressDialog.h"

RProgressDialog::RProgressDialog(const TGWindow *p, const TGWindow *main,
				 const char* objname, const char *mainname,
				 const char* title, const char* macrotext, 
				 const char* microtext,
				 int runcnt, int mpcnt, UInt_t w, 
				 UInt_t h, Bool_t stopable) :
    TGTransientFrame(p, main, w, h)
{
  kStopable = stopable;
  dContinue = kTrue;
  dStop = kFalse;
  dRuncnt = runcnt;
  dMpcnt = mpcnt;

  strcpy(dObjName,objname);
  strcpy(dMainName,mainname);
  
  
  // Dialog used to test the different supported progress bars.
  
  ChangeOptions((GetOptions() & ~kVerticalFrame) | kHorizontalFrame);
  
  fVframe1 = new TGVerticalFrame(this, 0, 0, 0);
  fHframe1 = new TGHorizontalFrame(this,60, 20, kFixedWidth);

  fHProg1 = new TGHProgressBar(fVframe1, 300);
  fHProg1->SetBarColor("lightblue");
  fHProg1->SetRange(0,dRuncnt);
  fHProg1->SetPosition(0);
  sprintf(dTemp,"%%.0f %s",macrotext);
  fHProg1->ShowPosition(kTRUE, kFALSE, dTemp);
  fHProg2 = new TGHProgressBar(fVframe1, TGProgressBar::kFancy, 300);
  fHProg2->SetBarColor("lightblue");
  fHProg2->SetRange(0,dMpcnt);
  sprintf(dTemp,"%%.0f %s",microtext);
  fHProg2->ShowPosition(kTRUE, kFALSE, dTemp);
  fHProg2->SetPosition(0);

  fStop = new TGTextButton(fHframe1, "Stop", M_PR_STOP);
  fStop->Associate(this);

  fResume = new TGTextButton(fHframe1, "Resume", M_PR_RESUME);
  fResume->Associate(this);

  fPause = new TGTextButton(fHframe1, "Pause", M_PR_PAUSE);
  fPause->Associate(this);
  
  fVframe1->Resize(300, 300);
  
  fHint2 = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 
			     5, 5,  5, 10);
  fHint3 = new TGLayoutHints(kLHintsLeft  | kLHintsCenterX, 1, 1, 1, 1);
  fHint4 = new TGLayoutHints(kLHintsBottom | 
			     kLHintsExpandX | 
			     kLHintsRight, 0, 0, 0, 0);
  fHint5 = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 0, 0, 0, 0);
  
  fVframe1->AddFrame(fHProg1, fHint2);
  fVframe1->AddFrame(fHProg2, fHint2);
  fHframe1->AddFrame(fStop, fHint3);
  fHframe1->AddFrame(fPause, fHint3);
  fHframe1->AddFrame(fResume, fHint3);
  fVframe1->AddFrame(fHframe1,fHint4);

  AddFrame(fVframe1, fHint5);
  
  SetWindowName(title);
  TGDimension size = GetDefaultSize();
  Resize(size);
  
  // position relative to the parent's window
  Window_t wdummy;
  int ax, ay;
  gVirtualX->TranslateCoordinates(main->GetId(), GetParent()->GetId(),
			     (Int_t)(((TGFrame *) main)->GetWidth() - fWidth) >> 1,
			     (Int_t)(((TGFrame *) main)->GetHeight() - fHeight) >> 1,
				  ax, ay, wdummy);
  Move(ax, ay);
  
  MapSubwindows();
  MapWindow();

  fHframe1->HideFrame(fStop);

  Connect("IsClosing(char*)",dMainName,(void*)main,"OnObjClose(char*)");
//   Connect("HaltProcess()",dMainName,(void*)main,"OnHaltProcess()");
  
  //fClient->WaitFor(this);
}

RProgressDialog::~RProgressDialog()
{
  delete fVframe1;
  delete fHProg1; delete fHProg2;
  delete fHint2; delete fHint5;delete fHint3;
  delete fStop;
  delete fPause;
  delete fResume;
}

void RProgressDialog::ResetRange(int runcnt, int mpcnt)
{
  if(runcnt > 0){
    double pos = fHProg1->GetPosition();
    fHProg1->Reset();
    fHProg1->SetRange(0,runcnt);
    fHProg1->SetPosition(pos);
    dRuncnt = runcnt;
    gSystem->ProcessEvents();
  }
  if(mpcnt > 0){
    fHProg2->Reset();
    fHProg2->SetRange(0,mpcnt);
    fHProg2->SetPosition(0);
    dMpcnt = mpcnt;
    gSystem->ProcessEvents();
  }
}

void RProgressDialog::SetTitle(char *title)
{
  SetWindowName(title);
}

void RProgressDialog::IsClosing(char *objname)
{
  Emit("IsClosing(char*)",(long)objname);
}

void RProgressDialog::HaltProcess()
{
  Emit("HaltProcess()");
}

void RProgressDialog::CloseWindow()
{
  // Called when window is closed via the window manager.

  IsClosing(dObjName); 
  delete this;//DeleteWindow();
}


Bool_t RProgressDialog::ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2)
{
  // Process slider messages.
  
  switch (GET_MSG(msg)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(msg)) {
    case kCM_BUTTON:
      switch (parm1){
      case M_PR_STOP:
	//dStop = kTrue;
	dContinue = kTrue;
	CloseWindow();
	break;
      case M_PR_PAUSE:
	dContinue = kFalse;
	if(kStopable)
	  fHframe1->ShowFrame(fStop);
	while(!Continue()){gSystem->ProcessEvents();};
	break;
      case M_PR_RESUME:
	fHframe1->HideFrame(fStop);
	dContinue = kTrue;
	break;
      }
      break;
    }
    break;

  case kC_PR_DIALOG:
    switch (GET_SUBMSG(msg)) {
    case kCM_PR_MSG:
      switch (parm1) {
      
      case M_PR_RUN:
	{
	  if(fHProg1->GetPosition() < dRuncnt-1)
	    fHProg2->Reset();
	  fHProg1->Increment(parm2);
	  //gSystem->ProcessEvents();
	}
	break;

      case M_PR_SEQ:
	{
	  fHProg2->Increment(parm2);
	  //gSystem->ProcessEvents();
	}
	break;
	
      default:
	break;
      }
      break;
    default:
      break;
    }
    break;
  default:
    break;
  }
  return kTRUE;
}
