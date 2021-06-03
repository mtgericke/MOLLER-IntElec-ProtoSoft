/////////////////////////////////////////////////////////////////////////////////////
//NPDGamma Experiment Data Visualization and Analysis
//
//Version 2.01
//
//RComboBoxDialog source file, containing methods to monitor analyzing
//progress
/////////////////////////////////////////////////////////////////////////////////////


#include "RComboBoxDialog.h"



RComboBoxDialog::RComboBoxDialog(const TGWindow *p, const TGWindow *main,
				 const char* objname, const char *ownername,
				 const char* title, Int_t esize, UInt_t w, 
				 UInt_t h) :
  TGTransientFrame(p, main, w, h)
{
  dEntrycnt = 0;
  dListSize = esize;

  dEntryArray = new char[dListSize][NAME_STR_MAX];
  
  strcpy(dObjName,objname);
  strcpy(dOwnerName,ownername);
  strcpy(dTitle,title);
  
  // Dialog used to test the different supported progress bars.
  
  ChangeOptions((GetOptions() & ~kVerticalFrame) | kHorizontalFrame);
  
  frame = new TGVerticalFrame(this,200, 50);
  fCancel = new TGTextButton(frame, " &Cancel ", 10);
  fCancel->Associate(this);
  fComboBox = new TGComboBox(frame,20);
  fComboBox->Associate(this);
  fHint = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 
			    2, 2,  2, 2);
  fHint2 = new TGLayoutHints(kLHintsCenterX,2, 2, 3, 0);
  frame->AddFrame(fComboBox, fHint);
  fComboBox->Resize(300, 20);
  frame->AddFrame(fCancel, fHint2);

  fHint3 = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 0, 0, 0, 0);
  AddFrame(frame, fHint3);
  
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

  //fClient->WaitFor(this);
}

RComboBoxDialog::~RComboBoxDialog()
{
  delete frame;
  delete fHint; delete fHint2;
  delete fComboBox;

  delete[] dEntryArray;
}

void RComboBoxDialog::IsClosing(char *objname)
{
  Emit("IsClosing(char*)",(long)objname);
}

void RComboBoxDialog::SetSelectedEntry(char* entry)
{
  Emit("SetSelectedEntry(char*)",(long)entry);
}

void RComboBoxDialog::CloseWindow()
{
  IsClosing(dObjName);
 
  DeleteWindow();
}

void RComboBoxDialog::AddEntry(char* entry)
{
  if(dEntrycnt < dListSize){
    strcpy(dEntryArray[dEntrycnt],entry);
    fComboBox->AddEntry(dEntryArray[dEntrycnt],dEntrycnt+1);
    dEntrycnt++;
  }
}

Bool_t RComboBoxDialog::ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2)
{
  switch (GET_MSG(msg)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(msg)) {

    case kCM_BUTTON:
      {
	switch (parm1) {
	case 10:
	  CloseWindow();
	  break;
	}
      }
      break;

    case kCM_COMBOBOX:
      {
	switch (parm1) {
	case 20:
	  {
	    printf("Entry Selected\n");
	    SetSelectedEntry(dEntryArray[fComboBox->GetSelected()-1]);
	    CloseWindow();
	  }
	  break;
	}
      }
      break; 
    }
    break;
  }
  return kTRUE;
}
