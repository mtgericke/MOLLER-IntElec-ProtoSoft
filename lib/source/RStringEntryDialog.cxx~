/////////////////////////////////////////////////////////////////////////////////////
//NPDGamma Experiment Data Visualization and Analysis
//
//Version 2.01
//
//RStringEntryDialog source file, containing methods to monitor analyzing
//progress
/////////////////////////////////////////////////////////////////////////////////////


#include "RStringEntryDialog.h"

RStringEntryDialog::RStringEntryDialog(const TGWindow *p, const TGWindow *main,
				       char* objname, char *ownername, Bool_t *retStat,
				       char* Func, const char *title, UInt_t w, 
				       UInt_t h) :
  TGTransientFrame(p, main, w, h)
{
  dFunc = Func;
  dRetStat = retStat;
  strcpy(dObjName,objname);
  strcpy(dOwnerName,ownername);
  strcpy(dTitle,title);
  
  ChangeOptions((GetOptions() & ~kVerticalFrame) | kHorizontalFrame);
  
  frame = new TGVerticalFrame(this,500, 50);
  fBttnframe = new TGHorizontalFrame(frame,500, 20);
  fEntrframe = new TGHorizontalFrame(frame,500, 30);
  fHint = new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2,  2, 2);
  fHint2 = new TGLayoutHints(kLHintsCenterX,2, 2, 3, 0);
  fHint3 = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 2, 2,  2, 2);
  fHint4 = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 0, 0, 0, 0);


  fOk = new TGTextButton(fBttnframe, " &Ok ", 10);
  fOk->Associate(this);
  fBttnframe->AddFrame(fOk,fHint2);
  fCancel = new TGTextButton(fBttnframe, " &Cancel ", 20);
  fCancel->Associate(this);
  fBttnframe->AddFrame(fCancel, fHint2);

  fFuncEntry = new TGTextEntry(fEntrframe, fFuncBuffer = new TGTextBuffer(200));
  fEntrframe->AddFrame(fFuncEntry,fHint);
  fFuncEntry->SetState(1);

//   fNumEntry = new TGNumberEntry(fEntrframe,1.0,8,30,TGNumberFormat::kNESReal); 
//   fNumEntry->Associate(this);
//   fEntrframe->AddFrame(fNumEntry,fHint);
  frame->AddFrame(fEntrframe,fHint3);
  frame->AddFrame(fBttnframe,fHint3);
  AddFrame(frame, fHint4);

  fFuncEntry->Resize(200, fFuncEntry->GetDefaultHeight());
  
  SetWindowName(dTitle);
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

  fClient->WaitFor(this);
}

RStringEntryDialog::~RStringEntryDialog()
{
//   delete frame;
//   delete fBttnframe;
//   delete fEntrframe;
//   delete fHint; 
//   delete fHint2;
//   delete fHint3;
//   delete fHint4;
//   delete fFuncEntry;
//   delete fFuncBuffer;
}

void RStringEntryDialog::IsClosing(char *objname)
{
  Emit("IsClosing(char*)",(long)objname);
}

void RStringEntryDialog::CloseWindow()
{
  IsClosing(dObjName);
 
  DeleteWindow();
}

Bool_t RStringEntryDialog::ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2)
{
  switch (GET_MSG(msg)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(msg)) {

    case kCM_BUTTON:
      {
	switch (parm1) {
	case 10:
	  strcpy(dFunc,fFuncBuffer->GetString());
	  *dRetStat = kTrue;
	  CloseWindow();
	  break;
	case 20:
	  *dRetStat = kFalse;
	  CloseWindow();
	  break;
	}
      }
      break;
    }
    break;
  }
  return kTRUE;
}
