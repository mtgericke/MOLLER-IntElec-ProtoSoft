/////////////////////////////////////////////////////////////////////////////////////
//NPDGamma Experiment Data Visualization and Analysis
//
//Version 001
//
//RDataFitEntry source file, containing methods to select fit options
/////////////////////////////////////////////////////////////////////////////////////


#include "RDataFitEntry.h"

const char *const RDataFitEntry::numlabel[15] = {
   "p0",
   "p1",
   "p2",
   "p3",
   "p4",
   "p5",
   "p6",
   "p7",   
   "p8",   
   "p9",   
   "plot",
   "Range min",
   "Range max",
   "Step size",
   "Chisq bound",
};

const char *const RDataFitEntry::functions[17] = {
  "pol0",
  "pol1",
  "pol2",
  "pol3",
  "pol4",
  "pol5",
  "pol6",
  "pol7",
  "pol8",
  "pol9",
  "gaus",
  "landau",
  "expo",
  "User Def.",
  "Acos+Bsin",
  "Acos+Bsin+C",
  "A(1-cos(Bx))",
};
     
RDataFitEntry::RDataFitEntry(const TGWindow * p, const TGWindow * main,
			     TQObject *mrec, const char *classname,
			     FitOptions *fOptions, Bool_t Mod,
			     UInt_t w, UInt_t h, UInt_t options)
  : TGTransientFrame(p, main, w, h, options)
{
  dModifyOptions = Mod;
  dMsgcnt = 0;
  dOpts = fOptions;
  dMrec = mrec;
  memset(dClass,'\0',sizeof(dClass));
  strcpy(dClass,classname);
//   Connect("SetFitOptions(char*)",dClass,
// 	  (void*)dMrec,"PutFitOptions(char*)");

  memset(dMiscbuffer2,'\0',sizeof(dMiscbuffer2));
  for(int i = 0; i < MSG_QUEUE_MAX; i++)
    memset(dMessage[i],'\0',sizeof(dMessage[i]));
  memset(dMiscbuffer,'\0',sizeof(dMiscbuffer));

  Int_t ninit = 1;
  Int_t fLEntry = 0;
  if(!dModifyOptions){
    numinit[0]=0;
    numinit[1]=0;
    numinit[2]=0;
    numinit[3]=0;
    numinit[4]=0;
    numinit[5]=0;
    numinit[6]=0;
    numinit[7]=0;
    numinit[8]=0;
    numinit[9]=0;
    numinit[10]=0;
    numinit[11]=0;
    numinit[12]=1.0;
    numinit[13]=0.0001;
    numinit[14]=1;
    ninit = 1;
    fLEntry = 11;    

    fNState[0] = 0;
    fNState[1] = 0;
    fNState[2] = 0;
    fNState[3] = 0;
    fNState[4] = 0;
    fNState[5] = 0;
    fNState[6] = 0;
    fNState[7] = 0;
    fNState[8] = 0;
    fNState[9] = 0;
    fNState[10] = 1;
    fNState[11] = 0;
    fNState[12] = 0;
    fNState[13] = 0;
    fNState[14] = 0;

    fCState[0] = kButtonUp;
    fCState[1] = kButtonUp;
    fCState[2] = kButtonUp;
    fCState[3] = kButtonUp;
    fCState[4] = kButtonUp;
    fCState[5] = kButtonUp;
    fCState[6] = kButtonUp;
    fCState[7] = kButtonUp;
    fCState[8] = kButtonUp;
    fCState[9] = kButtonUp;
    fCState[10] = kButtonDown;
    fCState[11] = kButtonDown;
  }
  else{
   numinit[0]=dOpts->param[0];
   numinit[1]=dOpts->param[1];
   numinit[2]=dOpts->param[2];
   numinit[3]=dOpts->param[3];
   numinit[4]=dOpts->param[4];
   numinit[5]=dOpts->param[5];
   numinit[6]=dOpts->param[6];
   numinit[7]=dOpts->param[7];
   numinit[8]=dOpts->param[8];
   numinit[9]=dOpts->param[9];
   for(int i = 0; i < 10; i++){
     fNState[i] = dOpts->parset[i];
     if(fNState[i])
       fCState[i] = kButtonDown;
     else
       fCState[i] = kButtonUp;       
   }
   fNState[10] = 1;
   if(dOpts->fullRange){
     fCState[10] = kButtonDown;
     fNState[11] = 0;
     fNState[12] = 0;
     numinit[10]=0;
     numinit[11]=0;
   }
   else{
     fCState[10] = kButtonUp;
     numinit[10]= dOpts->min[0];
     numinit[11]= dOpts->max[0];
     fNState[11] = 1;
     fNState[12] = 1;
   }
   if(dOpts->rootFit){
     fCState[11] = kButtonDown;
     numinit[12]=1.0;
     numinit[13]=0.0001;
     fNState[13] = 0;
     fNState[14] = 0;
   }
   else{
     fCState[11] = kButtonUp;
     numinit[12]=dOpts->stepsize[0];
     numinit[13]=dOpts->chisqBound;
     fNState[13] = 1;
     fNState[14] = 1;
   }
   numinit[14]=1;
   ninit = dOpts->plot;

   for(int i = 0; i < 16; i++)
     if(!strcmp(dOpts->func,functions[i]))
       {fLEntry = i+1; break;}
  }


  TGNumberFormat::EStyle style[15] = {TGNumberFormat::kNESReal,
				      TGNumberFormat::kNESReal,
				      TGNumberFormat::kNESReal,
				      TGNumberFormat::kNESReal,
				      TGNumberFormat::kNESReal,
				      TGNumberFormat::kNESReal,
				      TGNumberFormat::kNESReal,
				      TGNumberFormat::kNESReal,
				      TGNumberFormat::kNESReal,
				      TGNumberFormat::kNESReal,
				      TGNumberFormat::kNESInteger,
				      TGNumberFormat::kNESReal,
				      TGNumberFormat::kNESReal,
				      TGNumberFormat::kNESReal,
				      TGNumberFormat::kNESReal};


  for (int i = 0; i < 15; i++) {
    fNumericEntries[i] = NULL;
    fLabel[i] = NULL;
    fF[i] = NULL;
  }

  for (int i = 0; i < 11; i++)
    fC[i] = NULL;
  
  fSetButton = NULL;
  fExitButton = NULL;
  f1 = NULL;
  f2 = NULL;
  f3 = NULL;
  f4 = NULL;
  fL1 = NULL;
  fL2 = NULL;
  fL3 = NULL;
  fL4 = NULL;
  fL5 = NULL;
  fG1 = NULL;
  fG2 = NULL;
  
  // build widgets
  
  //ChangeOptions((GetOptions() & ~kVerticalFrame) | kHorizontalFrame);

  f2 = new TGCompositeFrame(this, 60, 20, kVerticalFrame);
  f1 = new TGCompositeFrame(f2, 60, 20, kHorizontalFrame);
  //f3 = new TGCompositeFrame(this, 60, 20, kVerticalFrame);
  f4 = new TGCompositeFrame(f2, 60, 20, kHorizontalFrame);


  fL1 = new TGLayoutHints(kLHintsCenterX,2, 2, 3, 0);
  fL2 = new TGLayoutHints(kLHintsTop | kLHintsExpandX,2, 5, 0, 2);
  fL3 = new TGLayoutHints(kLHintsTop | kLHintsRight | kLHintsExpandX,2, 5, 10, 0);
  fL4 = new TGLayoutHints(kLHintsTop | kLHintsLeft |
			  kLHintsExpandX | kLHintsExpandY,
			  2, 2, 2, 2);
  fL5 = new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2);

  fSetButton = new TGTextButton(f1, " &Set ", M_FITENTRY_SET);
  fSetButton->Associate(this);
  f1->AddFrame(fSetButton, fL1);
  fExitButton = new TGTextButton(f1, " &Cancel ", M_FITENTRY_CANCEL);
  fExitButton->Associate(this);
  f1->AddFrame(fExitButton, fL1);
  
  fG1 = new TGGroupFrame(f4, new TGString("Fit Options"));
  fG2 = new TGGroupFrame(f4, new TGString("Parameters"));

  
  for(int i = 0; i < 10; i++)
    {
      fF[i] = new TGHorizontalFrame(fG2, 200, 30);
      fG2->AddFrame(fF[i], fL5);
      fNumericEntries[i] = new TGNumberEntry(fF[i], numinit[i],8,i+20,style[i]);
      fNumericEntries[i]->GetNumberEntry()->Connect("TabPressed()", 
						    "RDataFitEntry", 
						    this, "FChangeFocus()");
      fNumericEntries[i]->Associate(this);
      fF[i]->AddFrame(fNumericEntries[i], fL5);
      fC[i] = new TGCheckButton(fF[i], new TGHotString(numlabel[i]),i+30);
      fF[i]->AddFrame(fC[i], fL5);
      fC[i]->Associate(this);
      fC[i]->SetState(fCState[i]);
      fNumericEntries[i]->SetState(fNState[i]);
    }
 
  fC[10] = new TGCheckButton(fG1, new TGHotString("Full Range"),M_FITENTRY_FULLRANGE);
  fG1->AddFrame(fC[10], fL5);
  fC[10]->Associate(this);

  fC[11] = new TGCheckButton(fG1, new TGHotString("Use ROOT Fits"),M_FITENTRY_ROOTR);
  fG1->AddFrame(fC[11], fL5);
  fC[11]->Associate(this);


  fF[10] = new TGHorizontalFrame(fG1, 200, 30);
  fG1->AddFrame(fF[10], fL5);
  fNumericEntries[10] = new TGNumberEntry(fF[10],ninit,8,40,style[10]);
  fNumericEntries[10]->GetNumberEntry()->Connect("TabPressed()", 
						 "RDataFitEntry", 
						 this, "FChangeFocus()");
  fNumericEntries[10]->Associate(this);
  fF[10]->AddFrame(fNumericEntries[10], fL5);
  fLabel[10] = new TGLabel(fF[10], numlabel[10]);
  fF[10]->AddFrame(fLabel[10], fL5);



  for(int i = 11; i < 15; i++)
    {
      fF[i] = new TGHorizontalFrame(fG1, 200, 30);
      fG1->AddFrame(fF[i], fL5);
      fNumericEntries[i] = new TGNumberEntry(fF[i], numinit[i-1],8,i+30,style[i]);
      fNumericEntries[i]->GetNumberEntry()->Connect("TabPressed()", 
						    "RDataFitEntry", 
						    this, "FChangeFocus()");
      fNumericEntries[i]->Associate(this);
      fF[i]->AddFrame(fNumericEntries[i], fL5);
      fLabel[i] = new TGLabel(fF[i], numlabel[i]);
      fF[i]->AddFrame(fLabel[i], fL5);
    }

  fComboBox = new TGComboBox(fG1,M_FITENTRY_SELFUNC);
  fG1->AddFrame(fComboBox,fL5);

  fUfuncE   = new TGTextEntry(fG1, fUfuncB = new TGTextBuffer(100));
  fG1->AddFrame(fUfuncE,fL5);
  fUfuncE->SetState(1);

  for (int i = 0; i < 16; i++)
    fComboBox->AddEntry(functions[i], i+1);

  fComboBox->Resize(100, 20);
  fComboBox->Select(fLEntry);

  fUfuncE->Resize(100, fUfuncE->GetDefaultHeight());

  fC[11]->SetState(fCState[11]);
  fNumericEntries[13]->SetState(fNState[13]);
  fNumericEntries[14]->SetState(fNState[14]);
  

  fC[10]->SetState(fCState[10]);
  fNumericEntries[11]->SetState(fNState[11]);
  fNumericEntries[12]->SetState(fNState[12]);

  f4->AddFrame(fG2, fL4);
  f4->AddFrame(fG1, fL4);
  f2->AddFrame(f4, fL2);
  f2->AddFrame(f1, fL2);
   //AddFrame(f1, fL3);
  AddFrame(f2, fL2);


   // set dialog box title
   SetWindowName("Fit Options");
   SetIconName("Fit Options");
   SetClassHints("RDataFitEntry", "RDataFitEntry");
   // resize & move to center
   MapSubwindows();
   UInt_t width = GetDefaultWidth();
   UInt_t height = GetDefaultHeight();
   Resize(width, height);
   Int_t ax;
   Int_t ay;
   if (main) {
      Window_t wdum;
      gVirtualX->TranslateCoordinates(main->GetId(), GetParent()->GetId(),
                                      (Int_t)(((TGFrame *) main)->GetWidth() -
                                       fWidth) >> 1,
                                      (Int_t)(((TGFrame *) main)->GetHeight() -
                                       fHeight) >> 1, ax, ay, wdum);
   } else {
      UInt_t root_w, root_h;
      gVirtualX->GetWindowSize(fClient->GetRoot()->GetId(), ax, ay,
                               root_w, root_h);
      ax = (Int_t)(root_w - fWidth) >> 1;
      ay = (Int_t)(root_h - fHeight) >> 1;
   }
   Move(ax, ay);
   SetWMPosition(ax, ay);
   // make the message box non-resizable
   SetWMSize(width, height);
   SetWMSizeHints(width, height, width, height, 0, 0);
   SetMWMHints(kMWMDecorAll | kMWMDecorResizeH | kMWMDecorMaximize |
               kMWMDecorMinimize | kMWMDecorMenu,
               kMWMFuncAll | kMWMFuncResize | kMWMFuncMaximize |
               kMWMFuncMinimize, kMWMInputModeless);

   //gVirtualX->GrabButton(fId, kButton5, kAnyModifier,
   //                      kButtonPressMask | kButtonReleaseMask,
   //                      kNone, kNone);
   //gVirtualX->GrabKey(fId, kGKeyPress, kAnyModifier, kTRUE);

   MapWindow();

   fClient->WaitFor(this);

}

RDataFitEntry::~RDataFitEntry()
{
  
   for (int i = 0; i < 15; i++) {
      delete fNumericEntries[i];
      delete fLabel[i];
      delete fF[i];
   }
   for (int i = 0; i < 11; i++) {
      delete fC[i];
   }

   delete fSetButton;
   delete fExitButton;
   delete f1;
   delete f2;
   delete f3;
   delete f4;
   delete fL1;
   delete fL2;
   delete fL3;
   delete fL4;
   delete fL5;
   delete fG1;
   delete fG2;
  
   printf("In destructor RDataFitEntry\n");
}


void RDataFitEntry::FlushMessages()
{
  while(GetMessage() != NULL){}
}

char *RDataFitEntry::GetMessage()
{
  if(dMsgcnt != 0)
    {
      strcpy(dMiscbuffer,dMessage[0]);
      for(int i = 0; i < dMsgcnt-1; i++)
	strcpy(dMessage[i],dMessage[i+1]);
      memset(dMessage[dMsgcnt-1],'\0',sizeof(dMessage[dMsgcnt-1]));
      dMsgcnt--;

      return dMiscbuffer;
    }

  return NULL;
}


Bool_t RDataFitEntry::SetMessage(const char *msg, const char *func, int TS, int MESSAGETYPE)
{
#ifdef RDFE_DEBUG
  char str1[NAME_STR_MAX];
  char str2[NAME_STR_MAX];

  if(MESSAGETYPE == M_FITENTRY_ERROR_MSG){
    sprintf(str1,"Object: %s\n","RDataFit");
    sprintf(str2,"Function: %s\n",func);
  }
#endif

  if(dMsgcnt == MSG_QUEUE_MAX)
    {
      for(int i = 0; i < dMsgcnt-1; i++)
	strcpy(dMessage[i],dMessage[i+1]);
      memset(dMessage[MSG_QUEUE_MAX-1],'\0',sizeof(dMessage[MSG_QUEUE_MAX-1]));
      if(MESSAGETYPE == M_FITENTRY_ERROR_MSG){ 
#ifdef RDFE_DEBUG
	strcpy(dMessage[MSG_QUEUE_MAX-1],str1);
	strcat(dMessage[MSG_QUEUE_MAX-1],str2);
#endif
	strcat(dMessage[MSG_QUEUE_MAX-1],msg);
      }
      else
	strcpy(dMessage[MSG_QUEUE_MAX-1],msg);
    }
  else
    {
      if(MESSAGETYPE == M_FITENTRY_ERROR_MSG){ 
#ifdef RDFE_DEBUG
	strcpy(dMessage[dMsgcnt],str1);
	strcat(dMessage[dMsgcnt],str2);
#endif
	strcat(dMessage[dMsgcnt],msg);
      }
      else
	strcpy(dMessage[dMsgcnt],msg);
      dMsgcnt++;
    }
      
  SendMessage(GetMain(),
	      MK_MSG((EWidgetMessageTypes)kC_FITENTRY_OBJECT,
		     (EWidgetMessageTypes)kCM_FITENTRY_MSG),
	      MESSAGETYPE,TS);
  gSystem->ProcessEvents();

  return kTrue;
}



void RDataFitEntry::CloseWindow()
{
  
  FlushMessages();
  SetMessage(FITENTRY_CLOSED_MSG,"",1,M_FITENTRY_CLOSED);
  DeleteWindow();
}

void RDataFitEntry::SetFitOptions(char *opts)
{
  // Emits signal when double clicking on icon.

  if(dMrec != NULL){
    Emit("SetFitOptions(char*)",(long)opts);
  }
  else{
    
  }
}


void RDataFitEntry::DoSettings()
{
  dOpts->npars = 0;
  
  for(int n = 0; n < 10; n ++)
    {
      if (fC[n]->GetState() == kButtonDown)
	{
	  dOpts->param[n] = fNumericEntries[n]->GetNumber();
	  dOpts->parset[n] = 1;
	  dOpts->npars++;
	}
      else{
	dOpts->param[n] = 0;
	dOpts->parset[n] = 0;
      }
    }

  dOpts->plot = (int)fNumericEntries[10]->GetNumber();
  
  if (fC[10]->GetState() == kButtonDown)
    {
      dOpts->min[0] = -1;
      dOpts->max[0] = -1;
      dOpts->fullRange = kTrue;
    }
  else
    {
      dOpts->min[0] = fNumericEntries[11]->GetNumber();
      dOpts->max[0] = fNumericEntries[12]->GetNumber();
      dOpts->fullRange = kFalse;
    }

  if (fC[11]->GetState() == kButtonDown)
    dOpts->rootFit = kTrue;
  else
    dOpts->rootFit = kFalse;
  
  strcpy(dOpts->func,functions[fComboBox->GetSelected()-1]);
  if(!strcmp("User Def.",dOpts->func))
    {
      strcpy(dOpts->func,fUfuncB->GetString());
    }

  dOpts->stepsize[0]= fNumericEntries[13]->GetNumber();
  dOpts->chisqBound = fNumericEntries[14]->GetNumber();

  SetFitOptions((char*)dOpts);
}


void RDataFitEntry::FChangeFocus()
{
  int l = 0;
  for(int m = 0; m < 15; m++){
    if (gTQSender == fNumericEntries[m]->GetNumberEntry()){
      if(m == 14){l=0;}else{l = m+1;}
      while(fNumericEntries[l]->GetButtonUp()->GetState() 
	    == kButtonDisabled){
	if(l == m) break;
	l++;
	if(l == 15){l=0;}
      }
      fNumericEntries[l]->GetNumberEntry()->SetFocus();break;
    }
  }
}



Bool_t RDataFitEntry::ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2)
{
  switch (GET_MSG(msg)) {
  case kC_COMMAND:
    {
      switch (GET_SUBMSG(msg)) {
      case kCM_COMBOBOX:
	{
	  //printf("function = %s\n",functions[fComboBox->GetSelected()-1]);
	  switch (parm1) {
	  case M_FITENTRY_SELFUNC:
	    {
	      if(!strcmp("User Def.",functions[fComboBox->GetSelected()-1]))
		fUfuncE->SetState(1);
	    }
	    break;
	  }
	}
	break;

      case kCM_BUTTON:
	{
	  switch (parm1) {
	    // exit button
	  case M_FITENTRY_CANCEL:
	    {
	      dOpts = NULL;
	      CloseWindow();
	      break;
	    }
	    // set button
	  case M_FITENTRY_SET:
	    {
	      DoSettings();
	      CloseWindow();
	      break;
	    }
	  }
	  break;
	case kCM_CHECKBUTTON:
	  {
	    switch (parm1){
	    case 30:
	      {
		if(fC[0]->GetState() == kButtonDown)
		  fNumericEntries[0]->SetState(1);
		else
		  fNumericEntries[0]->SetState(0);
	      }
	      break;
	    case 31:
	      {
		if(fC[1]->GetState() == kButtonDown)
		  fNumericEntries[1]->SetState(1);
		else
		  fNumericEntries[1]->SetState(0);
	      }
	      break;
	    case 32:
	      {
		if(fC[2]->GetState() == kButtonDown)
		  fNumericEntries[2]->SetState(1);
		else
		  fNumericEntries[2]->SetState(0);
	      }
	      break;
	    case 33:
	      {
		if(fC[3]->GetState() == kButtonDown)
		  fNumericEntries[3]->SetState(1);
		else
		  fNumericEntries[3]->SetState(0);
	      }
	      break;
	    case 34:
	      {
		if(fC[4]->GetState() == kButtonDown)
		  fNumericEntries[4]->SetState(1);
		else
		  fNumericEntries[4]->SetState(0);
	      }
	      break;
	    case 35:
	      {
		if(fC[5]->GetState() == kButtonDown)
		  fNumericEntries[5]->SetState(1);
		else
		  fNumericEntries[5]->SetState(0);
	      }
	      break;
	    case 36:
	      {
		if(fC[6]->GetState() == kButtonDown)
		  fNumericEntries[6]->SetState(1);
		else
		  fNumericEntries[6]->SetState(0);
	      }
	      break;
	    case 37:
	      {
		if(fC[7]->GetState() == kButtonDown)
		  fNumericEntries[7]->SetState(1);
		else
		  fNumericEntries[7]->SetState(0);
	      }
	      break;
	    case 38:
	      {
		if(fC[8]->GetState() == kButtonDown)
		  fNumericEntries[8]->SetState(1);
		else
		  fNumericEntries[8]->SetState(0);
	      }
	      break;
	    case 39:
	      {
		if(fC[9]->GetState() == kButtonDown)
		  fNumericEntries[9]->SetState(1);
		else
		  fNumericEntries[9]->SetState(0);
	      }
	      break;
		      
	    case M_FITENTRY_FULLRANGE:
	      {
		if (fC[10]->GetState() == kButtonDown)
		  {
		    fNumericEntries[11]->SetState(0);
		    fNumericEntries[12]->SetState(0);
		  }
		else if(fC[10]->GetState() == kButtonUp)
		  {
		    fNumericEntries[11]->SetState(1);
		    fNumericEntries[12]->SetState(1);
		  }
	      }
	      break;

	    case M_FITENTRY_ROOTR:
	      {
		if (fC[11]->GetState() == kButtonDown)
		  {
		    fNumericEntries[13]->SetState(0);
		    fNumericEntries[14]->SetState(0);
		  }
		else if(fC[11]->GetState() == kButtonUp)
		  {
		    fNumericEntries[13]->SetState(1);
		    fNumericEntries[14]->SetState(1);
		  }
	      }
	      break;
	      
	    }
	    break;
	  }
	}
      }
      break;
    }
  }
  return kTRUE;
}

