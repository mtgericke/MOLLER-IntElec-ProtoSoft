/////////////////////////////////////////////////////////////////////////////////////
//NPDGamma Experiment Data Visualization and Analysis
//
//Version 001
//
//RProcessLog source file, containing methods to store and display a log
//of current process actions.
/////////////////////////////////////////////////////////////////////////////////////

#include "RProcessLog.h"

RProcessLog::RProcessLog(const TGWindow *p, const TGWindow *main,
			 char *objName, char *mainname,
			 char *logname, char *logtitle,
			 UInt_t w,UInt_t h, UInt_t options)
  : TGTransientFrame(p, main, w, h, options)
{

  //*******************Initialize Everything*****************************************

  fMrec = NULL;
  dMsgcnt = 0;
 
  fMenuLogFile = NULL;
  fMenuBar = NULL;
  fMenuBarItemLayout = NULL;
  fMenuBarLayout = NULL;
  fEditLayout = NULL; 
  fText = NULL;
  fEdit = NULL;

  memset(dObjName,'\0',NAME_STR_MAX);
  strcpy(dObjName,objName);

  memset(dReceiverName,'\0',sizeof(dReceiverName));
  strcpy(dReceiverName,"");

  memset(dLogname,'\0',NAME_STR_MAX);
  strcpy(dLogname,logname);
  
  memset(dLogtitle,'\0',NAME_STR_MAX);
  strcpy(dLogtitle,logtitle);  
  
  memset(dMainName,'\0',sizeof(dMainName));
  strcpy(dMainName,mainname);
  
  memset(dMiscbuffer,'\0',sizeof(dMiscbuffer));
  memset(dMiscbuffer2,'\0',sizeof(dMiscbuffer2));
  for(int i = 0; i < MSG_QUEUE_MAX; i++)
    memset(dMessage[i],'\0',sizeof(dMessage[i]));

  //*************Create And Position The Menu Bar And Its Items**********************
  
  fMenuBarLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,
				     0, 0, 1, 1);
  fMenuBarItemLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 4, 0, 0);

  fMenuLogFile = new TGPopupMenu(fClient->GetRoot());
  fMenuLogFile->AddEntry("&Open (Log file)...", M_LOG_FILE_OPEN);
  fMenuLogFile->AddEntry("&Save log", M_LOG_FILE_SAVE);
  fMenuLogFile->AddEntry("S&ave log as...", M_LOG_FILE_SAVEAS);
  fMenuLogFile->AddEntry("C&lose (log file)",M_LOG_FILE_CLOSE);
  fMenuLogFile->AddSeparator();
  fMenuLogFile->AddEntry("&Print", M_LOG_FILE_PRINT);
  fMenuLogFile->AddEntry("P&rint setup...", M_LOG_FILE_PRINTSETUP);
  fMenuLogFile->AddSeparator();
  fMenuLogFile->AddEntry("&Exit", M_LOG_FILE_EXIT);
  
  
  fMenuLogFile->Associate(this);
  
  fMenuBar = new TGMenuBar(this, 1, 1, kHorizontalFrame);
  fMenuBar->AddPopup("&File", fMenuLogFile, fMenuBarItemLayout);
  
  AddFrame(fMenuBar, fMenuBarLayout);

  //*************Create The Editor and Its Region************************************
  
  fEdit = new TGTextEdit(this, w, h, kSunkenFrame | kDoubleBorder);
  fEditLayout = new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 3, 3, 3, 3);
  AddFrame(fEdit, fEditLayout);
  
  //*************Position The Dialog Box And Make Widget and Windows Visible*********
  //Except for the positioning of the Dialog Window, these steps are always performed
  //at the end of every constructor that creates a visible entity.  

  MapSubwindows();
  Resize(GetDefaultSize());

  //printf("Total Width=%d, Total Height=%d\n",GetSize().fWidth,GetSize().fHeight);
  
  // position relative to the parent's window
  //Window_t wdum;
  //int ax, ay;
  //gVirtualX->TranslateCoordinates(main->GetId(), GetParent()->GetId(),
  //		     (Int_t)(((TGFrame *) main)->GetWidth() - fWidth) >> 1,
  //		     (Int_t)(((TGFrame *) main)->GetHeight() - fHeight) >> 1,
  //			  ax, ay, wdum);
  Move(0, 0);
  
  SetWindowName("Process Log");
  
  MapWindow();
  //fClient->WaitFor(this);    // otherwise canvas contextmenu does not work

  Connect("IsClosing(char*)",dMainName,(void*)main,"OnObjClose(char*)");

  fEdit->Connect("Closed()","RProcessLog",this,"ToggleOpenFlag()");
  fEdit->Connect("Opened()","RProcessLog",this,"ToggleOpenFlag()");

  //********************Set Some Initial Log Info************************************

  fText = fEdit->GetText();
  memset(dMiscbuffer,'\0',MSG_SIZE_MAX);
  sprintf(dMiscbuffer,"New %s Log Session.",dLogtitle);
  fEdit->AddLine(
  "*******************************************************************************");
  fEdit->AddLine(dMiscbuffer);
  memset(dMiscbuffer,'\0',MSG_SIZE_MAX);
  cutime = new time_t; 
  time(cutime);
  memset(dMiscbuffer2,'\0',MSG_SIZE_MAX);
  strncpy(dMiscbuffer2,asctime((const tm*)localtime((const time_t*)cutime)),
	  strlen(asctime((const tm*)localtime((const time_t*)cutime)))-1);
  sprintf(dMiscbuffer,"Started: %s",dMiscbuffer2);
  fEdit->AddLine(dMiscbuffer);
  fEdit->AddLine(
  "*******************************************************************************");
  fEdit->AddLine("");
  fEdit->AddLine("");
  fEdit->AddLine("");
  fEdit->Goto(fText->RowCount(),fText->GetLineLength((Long_t)(fText->RowCount()-1)));
  delete cutime;
  cutime = NULL;  
}


RProcessLog::~RProcessLog()
{
  
  delete fMenuLogFile;
  delete fMenuBar;
  delete fEditLayout; 
  delete fMenuBarItemLayout;
  delete fMenuBarLayout;
  delete fEdit;
}

void RProcessLog::ToggleOpenFlag()
{ 
  dOpenFlag==kTrue ? dOpenFlag=kFalse : dOpenFlag=kTrue;
}

void RProcessLog::FlushMessages()
{
  while(GetMessage() != NULL){}
}

char *RProcessLog::GetMessage()
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


Bool_t RProcessLog::SetMessage(const char *msg, const char *func, int TS, int MESSAGETYPE)
{
#ifdef RPL_DEBUG
  char str1[NAME_STR_MAX];
  char str2[NAME_STR_MAX];

  if(MESSAGETYPE == M_LOGFILE_ERROR_MSG){
    sprintf(str1,"Object: %s\n",GetLogTitle());
    sprintf(str2,"Function: %s\n",func);
  }
#endif

  if(dMsgcnt == MSG_QUEUE_MAX)
    {
      for(int i = 0; i < dMsgcnt-1; i++)
	strcpy(dMessage[i],dMessage[i+1]);
      memset(dMessage[MSG_QUEUE_MAX-1],'\0',sizeof(dMessage[MSG_QUEUE_MAX-1]));
      if(MESSAGETYPE == M_LOGFILE_ERROR_MSG){ 
#ifdef RPL_DEBUG
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
      if(MESSAGETYPE == M_LOGFILE_ERROR_MSG){ 
#ifdef RPL_DEBUG
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
	      MK_MSG((EWidgetMessageTypes)kC_LOGFILE_OBJECT,
		     (EWidgetMessageTypes)kCM_LOGFILE_MSG),
	      MESSAGETYPE,TS);
  gSystem->ProcessEvents();

  return kTrue;
}

void RProcessLog::IsClosing(char *name)
{
  Emit("IsClosing(char*)",(long)name);
}

void RProcessLog::ConnectWith(const TGWindow *rec, char *recname)
{
  if(fMrec == NULL && rec != NULL){
    fMrec = (TGWindow*)rec;
    strcpy(dReceiverName,recname);
  }
}

void RProcessLog::CloseConnection()
{
  if(fMrec != NULL){
    fMrec = NULL;
    strcpy(dReceiverName,"");
  }
}

void RProcessLog::CloseWindow()
{
  //Called when window is closed via the window manager.

  //Calls Class Destructor

  IsClosing(dObjName);
  FlushMessages();
  SetMessage(LOGFILE_CLOSED_MSG,"",1,M_LOGFILE_CLOSED);
  DeleteWindow();
}


void RProcessLog::PrintLogLatex()
{
  char *miscbuffer;

  miscbuffer = new char[GetLogSize()+1];
  memset(miscbuffer,'\0',GetLogSize()+1);
  //miscbuffer[GetLogSize(fEdit)]='\0';
  GetLog(miscbuffer,GetLogSize()+1);

  //do printing here...

  delete[] miscbuffer;
}

ULong_t RProcessLog::GetLineCnt()
{
  if(IsFileOpen())
    return  fEdit->GetText()->RowCount();

  return 0;
}

char* RProcessLog::AddLatexFigure(char *buffer, char *filename)
{
  return NULL;
}


int RProcessLog::Append(char *buffer,Bool_t T_Stamp)
{
  int size = strlen(buffer)+200;
  char *ptr;
  char *line;
  int  index;
  char *tmpbuffer = new char[size];

  if(!tmpbuffer){
    FlushMessages();
    SetMessage(BUF_MEM_ERROR,"Append(..)",1,
	       M_LOGFILE_ERROR_MSG);
    return LOG_PROCESS_ERROR;    
  }
  
  if(!fEdit){
    FlushMessages();
    SetMessage(EDITOR_NULL_ERROR,"Append(..)",1,
	       M_LOGFILE_ERROR_MSG);
    return LOG_PROCESS_ERROR;    
  }


  fText = fEdit->GetText();
  
  if(!fText){
    FlushMessages();
    SetMessage(TEXT_NULL_ERROR,"Append(..)",1,
	       M_LOGFILE_ERROR_MSG);
    return LOG_PROCESS_ERROR;    
  }
  

  memset(tmpbuffer,'\0',size);
  cutime = new time_t;
  time(cutime);
  ltime = localtime((const time_t*)cutime);
  if(ltime->tm_sec < 10)
    sprintf(tmpbuffer,"%d:%d:0%d     %s",ltime->tm_hour,ltime->tm_min,ltime->tm_sec,
	    "Log Entry");
  else if(ltime->tm_min < 10)
    sprintf(tmpbuffer,"%d:0%d:%d     %s",ltime->tm_hour,ltime->tm_min,ltime->tm_sec,
	    "Log Entry");
  else if(ltime->tm_hour < 10)
    sprintf(tmpbuffer,"0%d:%d:%d     %s",ltime->tm_hour,ltime->tm_min,ltime->tm_sec,
	    "Log Entry");
  else
    sprintf(tmpbuffer,"%d:%d:%d     %s",ltime->tm_hour,ltime->tm_min,ltime->tm_sec,
	    "Log Entry");

  if(T_Stamp)
    {
      fEdit->AddLine(tmpbuffer);
      fEdit->AddLine("");
    }
  ptr = buffer;
  index = 0;
    
  for(int i = 0; i <= (int)strlen(buffer); i ++)
    {
      if(buffer[i] == '\n' || i-index > 64)
	{
	  line = new char[i-index+1];
	  memset(line,'\0',i-index+1);
	  strncpy(line,ptr,i-index);
	  sprintf(tmpbuffer,"             %s",line);
	  fEdit->AddLine(tmpbuffer);
	  delete[] line;
	  if(buffer[i] == '\n'){
	    index = i+1;
	    ptr = &buffer[i+1];
	  }
	  else{
	    index = i;
	    ptr = &buffer[i];
	  }
	  //fEdit->ScrollCanvas(fEdit->GetText()->RowCount(),TGView:: kVertical);
	  gSystem->ProcessEvents();
	}
      else if(buffer[i] == '\0')
	{
	  sprintf(tmpbuffer,"             %s",ptr);
	  fEdit->AddLine(tmpbuffer);
	}
    }
  
  fEdit->AddLine("");
  fEdit->Goto(fText->RowCount(),
	      fText->GetLineLength((Long_t)(fText->RowCount()-1)));

  fText = NULL;
  return LOG_PROCESS_OK;
}

Long_t RProcessLog::GetLogSize()
{
  //Long_t lline = 0;
  ULong_t size = 0;
  
  if(fEdit)
    fText = fEdit->GetText();
  else{
    return -1;
  }

  if(!fText) return -1;
  
  Long_t cnt = fText->RowCount();
  //lline = text->GetLongestLine();

  for(int i = 0; i < cnt; i++){
    size += fText->GetLineLength((Long_t)i)+1;
  }
  
  
  //Long_t llength = text->GetLineLength(lline);
  
  //size = (ULong_t)(cnt*llength+cnt*2+2);

  fText = NULL;
  return size+3;
}

Int_t RProcessLog::GetLog(char *buffer, Long_t size)
{
  TGLongPosition pos;

  if(!fEdit){
    FlushMessages();
    SetMessage(EDITOR_NULL_ERROR,"GetLog(..)",1,
	       M_LOGFILE_ERROR_MSG);
    return LOG_PROCESS_ERROR;    
  }

  fText = fEdit->GetText();

  if(!fText){
    FlushMessages();
    SetMessage(TEXT_NULL_ERROR,"GetLog(..)",1,
	       M_LOGFILE_ERROR_MSG);
    return LOG_PROCESS_ERROR;    
  }


  Long_t cnt = fText->RowCount();
  ULong_t llength = 0;


  if(size >= GetLogSize()+1)
    {
      for(int i = 0; i < cnt; i++)
	{
	  llength = fText->GetLineLength((Long_t)i);
	  printf("Here\n");	  
	  pos.fX = 0;
	  pos.fY = (Long_t)i;
	  strncat(buffer,fText->GetLine(pos,llength),llength);
	  printf("%s",fText->GetLine(pos,llength));
	  strncat(buffer,"\n",2);
	}
    }
  else{
    FlushMessages();
    SetMessage(BUF_OVFLOW_ERROR,"GetLog(..)",1,
	       M_LOGFILE_ERROR_MSG);
    return LOG_PROCESS_ERROR;    
  }

  fText = NULL; 
  return LOG_PROCESS_OK;
}

Bool_t RProcessLog::ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2) 
{
  // Process messages coming from widgets associated with the dialog.
  TGFileInfo fi;

  //Int_t mb_button_id[9] = { kMBYes, kMBNo, kMBOk, kMBApply,
  //	    kMBRetry, kMBIgnore, kMBCancel,
  //	    kMBClose, kMBDismiss };
  
  //EMsgBoxIcon mb_icon[4] = { kMBIconStop, kMBIconQuestion,
  //	     kMBIconExclamation, kMBIconAsterisk };

  const char *logfiletypes[] = { "Log files",     "*.log",
				 "ROOT macros",   "*.C",
				 "Data files",    "*.dat",
				 "All files",     "*",
				 0,               0 };

  int retval;
  
  Bool_t untitled = kTrue;
  char lmsg[600];


  switch (GET_MSG(msg)) {

  case kC_COMMAND:
    switch (GET_SUBMSG(msg)) {
    case kCM_MENU:
      switch (parm1) {
      case M_LOG_FILE_OPEN:
	if (!fEdit->IsSaved()) 
	  {
	    fText = fEdit->GetText();
	    untitled = !strlen(fText->GetFileName()) ? kTRUE : kFALSE;
	    
	    sprintf(lmsg, "Save ""%s""?",
		    untitled ? "Untitled Log File" : fText->GetFileName());
	    new TGMsgBox(fClient->GetRoot(), this, "Editor", lmsg,
			 kMBIconExclamation, kMBYes|kMBNo|kMBCancel, &retval);
	    
	    if (retval == kMBCancel)
	      return kTRUE;
	    if (retval == kMBYes)
	      if (!fEdit->SaveFile(0))
		return kTRUE;
	  }
	fEdit->Clear();
	
	SendMessage(fEdit, MK_MSG(kC_TEXTVIEW, kTXT_CLOSE),
		    fEdit->WidgetId(), 0);
	fEdit->Closed();
	
	fi.fFileTypes = logfiletypes;
	new TGFileDialog(fClient->GetRoot(), this, kFDOpen, &fi);
	if (fi.fFilename && strlen(fi.fFilename)) {
	  fEdit->LoadFile(fi.fFilename);
	  SendMessage(fEdit, MK_MSG(kC_TEXTVIEW, kTXT_OPEN),
		      fEdit->WidgetId(), 0);
	  fEdit->Opened();
	}
	break;
	
      case M_LOG_FILE_SAVE:
	if (fEdit->SaveFile(0)) {
	  SendMessage(fEdit, MK_MSG(kC_TEXTVIEW, kTXT_SAVE),
		      fEdit->WidgetId(), 0);
	  fEdit->Saved();
	}
	break;
	
      case M_LOG_FILE_SAVEAS:
	if (fEdit->SaveFile(0, kTRUE)) {
	  SendMessage(fEdit, MK_MSG(kC_TEXTVIEW, kTXT_SAVE),
		      fEdit->WidgetId(), 0);
	  fEdit->SavedAs();
	}
	break;
	
      case M_LOG_FILE_CLOSE:
	if (!fEdit->IsSaved()) 
	  {
	    fText = fEdit->GetText();
	    untitled = !strlen(fText->GetFileName()) ? kTRUE : kFALSE;
	    
	    sprintf(lmsg, "Save ""%s""?",
		    untitled ? "Untitled Log File" : fText->GetFileName());
	    new TGMsgBox(fClient->GetRoot(), this, "Editor", lmsg,
			 kMBIconExclamation, kMBYes|kMBNo|kMBCancel, &retval);
	    
	    if (retval == kMBCancel)
	      return kTRUE;
	    if (retval == kMBYes)
	      if (!fEdit->SaveFile(0))
		return kTRUE;
	  }
	fEdit->Clear();
	
	SendMessage(fEdit, MK_MSG(kC_TEXTVIEW, kTXT_CLOSE),
		    fEdit->WidgetId(), 0);
	fEdit->Closed();
	break;

      case M_LOG_FILE_EXIT:
	if (!fEdit->IsSaved()) 
	  {
	    fText = fEdit->GetText();
	    untitled = !strlen(fText->GetFileName()) ? kTRUE : kFALSE;
	    
	    sprintf(lmsg, "Save ""%s""?",
		    untitled ? "Untitled Log File" : fText->GetFileName());
	    new TGMsgBox(fClient->GetRoot(), this, "Editor", lmsg,
			 kMBIconExclamation, kMBYes|kMBNo|kMBCancel, &retval);
	    
	    if (retval == kMBCancel)
	      return kTRUE;
	    if (retval == kMBYes)
	      if (!fEdit->SaveFile(0))
		return kTRUE;
	  }
	fEdit->Clear();
	
	SendMessage(fEdit, MK_MSG(kC_TEXTVIEW, kTXT_CLOSE),
		    fEdit->WidgetId(), 0);
	fEdit->Closed();
	CloseWindow();
	break;

	
      case M_LOG_FILE_PRINT:
	PrintLogLatex();
	break;
	
      case M_LOG_FILE_PRINTSETUP:
	//printf("M_FILE_PRINTSETUP\n");
	//printf("Enabling \"Print\"\n");
	//fMenuRunFile->EnableEntry(M_FILE_PRINT);
	break;
	
      default:
	break;
      }
    }
    break;
    
  default:
    break;
  }
  return kTRUE;
}
