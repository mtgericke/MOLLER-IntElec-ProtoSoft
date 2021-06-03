/////////////////////////////////////////////////////////////////////////////////////
//NPDGamma Experiment Data Visualization and Analysis
//
//Version 2.01
//
//RNumberEntryDialog source file, containing methods to monitor analyzing
//progress
/////////////////////////////////////////////////////////////////////////////////////


#include "RNumberEntryDialog.h"

ClassImp(RNumberEntryDialog)

RNumberEntryDialog::RNumberEntryDialog(const TGWindow *p, const TGWindow *main,
				       const char* objname, const char *ownername, 
				       const char *title, const char *msg,
				       Double_t* number, Int_t *retval, UInt_t w, 
				       UInt_t h) :
  TGTransientFrame(p, main, w, h)
{
  UInt_t width, height;
  dNumber = number;
  dRetVal = retval;

  strcpy(dObjName,objname);
  strcpy(dOwnerName,ownername);
  strcpy(dTitle,title);
  
  ChangeOptions((GetOptions() & ~kVerticalFrame) | kHorizontalFrame);
  
  fMsgList   = new TList;
  frame = new TGVerticalFrame(this,200, 100);
  fBttnframe = new TGHorizontalFrame(frame,500, 20);
  fEntrframe = new TGHorizontalFrame(frame,500, 30);
  fLabelFrame = new TGVerticalFrame(frame,500, 20);
  fHint = new TGLayoutHints(kLHintsTop | kLHintsCenterX, 2, 2,  2, 2);
  fHint2 = new TGLayoutHints(kLHintsCenterX,2, 2, 3, 0);
  fHint3 = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 2, 2,  2, 2);
  fHint4 = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 0, 0, 0, 0);
  fHint5 = new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,
			     4, 2, 2, 2);

  if(msg){
    TGLabel *textLabel;

    char *line;
    char *tmpMsg, *nextLine;
    
    tmpMsg = new char[strlen(msg) + 1];
    nextLine = tmpMsg;
    
    line = tmpMsg;
    strcpy(nextLine, msg);
    while ((nextLine = strchr(line, '\n'))) {
      *nextLine = '\0';
      textLabel = new TGLabel(fLabelFrame, line);
      fMsgList->Add(textLabel);
      fLabelFrame->AddFrame(textLabel, fHint5);
      line = nextLine + 1;
    }
    
    textLabel = new TGLabel(fLabelFrame, line);
    fMsgList->Add(textLabel);
    fLabelFrame->AddFrame(textLabel, fHint5);
    delete [] tmpMsg;
    
    frame->AddFrame(fLabelFrame, fHint5);
  }

  fNumEntry = new TGNumberEntry(fEntrframe,1.0,8,30,TGNumberFormat::kNESReal); 
  fNumEntry->Associate(this);
  fEntrframe->AddFrame(fNumEntry,fHint);
  frame->AddFrame(fEntrframe,fHint3);
  frame->AddFrame(fBttnframe,fHint3);

  fOk = new TGTextButton(fBttnframe, " &Ok ", 10);
  fOk->Associate(this);
  fBttnframe->AddFrame(fOk,fHint2);
  fCancel = new TGTextButton(fBttnframe, " &Cancel ", 20);
  fCancel->Associate(this);
  fBttnframe->AddFrame(fCancel, fHint2);

  AddFrame(frame, fHint4);

//   TGDimension size = GetDefaultSize();
//   Resize(size);
  width  = GetDefaultWidth();
  height = GetDefaultHeight();
  
  Resize(width, height);
  
  // position relative to the parent's window
    
  // make the message box non-resizable
  
  SetWMSize(width, height);
  SetWMSizeHints(width, height, width, height, 0, 0);
  
  
  SetName(dObjName);
  SetWindowName(dTitle);
  SetIconName(dTitle);
  SetClassHints("NumEntryBox", "NumEntryBox");

  SetMWMHints(kMWMDecorAll | kMWMDecorResizeH  | kMWMDecorMaximize |
	      kMWMDecorMinimize | kMWMDecorMenu,
	      kMWMFuncAll  | kMWMFuncResize    | kMWMFuncMaximize |
	      kMWMFuncMinimize,
	      kMWMInputModeless);
  
  // position relative to the parent's window
  Window_t wdummy;
  int ax, ay;
  gVirtualX->TranslateCoordinates(main->GetId(), GetParent()->GetId(),
			     (Int_t)(((TGFrame *) main)->GetWidth() - fWidth) >> 1,
			     (Int_t)(((TGFrame *) main)->GetHeight() - fHeight) >> 1,
				  ax, ay, wdummy);
  Move(ax, ay);

  CenterOnParent();
  

  MapSubwindows();
//   MapWindow();
  MapRaised();

  Connect("IsClosing(const char*)",dOwnerName,(void*)main,"OnObjClose(char*)");
  
  fClient->WaitFor(this);
}

RNumberEntryDialog::~RNumberEntryDialog()
{
  delete frame;
  delete fBttnframe;
  delete fEntrframe;
  delete fHint; 
  delete fHint2;
  delete fHint3;
  delete fHint4;
  delete fHint5;
  delete fNumEntry;
  delete fLabelFrame;
  fMsgList->Delete();
  delete fMsgList;
}

void RNumberEntryDialog::IsClosing(const char *objname)
{
  Emit("IsClosing(const char*)",(long)objname);
}

void RNumberEntryDialog::CloseWindow()
{
  IsClosing(dObjName);
 
  DeleteWindow();
}

Bool_t RNumberEntryDialog::ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2)
{
  switch (GET_MSG(msg)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(msg)) {

    case kCM_BUTTON:
      {
	switch (parm1) {
	case 10:
	  *dNumber = fNumEntry->GetNumber();
	  *dRetVal = 1;
	  CloseWindow();
	  break;
	case 20:
	  *dRetVal = 0;
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
