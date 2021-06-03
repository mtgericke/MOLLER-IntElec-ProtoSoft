/////////////////////////////////////////////////////////////////////////////////////
//Root Based Data Visualization and Analysis Utility Library
//
//Version 000
//
//RDataContainer source file, containing methods to open and read data
//Stores a pointer to a currently open file
//All file formats are specified here.
/////////////////////////////////////////////////////////////////////////////////////


#include <RDataContainer.h>
#include <math.h>



RDataContainer::RDataContainer(const TGWindow *p, const TGWindow *main, 
			       const char *objName, const char *mainname, const char *dataname, 
			       ERFileModes mode, ERFileTypes type, 
			       UInt_t w,UInt_t h, UInt_t options)
  :TQObject()
{
  fMrec = NULL;
  dMode = mode;
  dType = type;
  dDatasize = 0;
  dFilesize = 0;
  dMsgcnt = 0;
  dFp = NULL;
  fRfile = NULL;
  dOpen = kFalse;
  fRootBrowser = NULL;
  SetBrowserFlag(kFalse);

  memset(dObjName,'\0',NAME_STR_MAX);
  strcpy(dObjName,objName);

  memset(dReceiverName,'\0',sizeof(dReceiverName));
  strcpy(dReceiverName,"");

  memset(dMainName,'\0',sizeof(dMainName));
  memset(dMiscbuffer2,'\0',sizeof(dMiscbuffer2));
  memset(dFilename,'\0',sizeof(dFilename));
  memset(dDataname,'\0',sizeof(dDataname));
  for(int i = 0; i < MSG_QUEUE_MAX; i++)
      memset(dMessage[i],'\0',sizeof(dMessage[i]));
  memset(dMiscbuffer,'\0',sizeof(dMiscbuffer));
  strcpy(dDataname,dataname);
  strcpy(dMainName,mainname);

  Connect("IsClosing(char*)",dMainName,(void*)main,"OnObjClose(char*)");  
  Connect("SendMessageSignal(char*)",dMainName,(void*)main,
	  "OnReceiveMessage(char*)");
}

// RDataContainer::RDataContainer(const RDataContainer &copy)
//   :TQObject(copy)
// {
//   dFp = NULL;
//   dDatasize = 0;
//   dFilesize = 0;

//   strcpy(dFilename,copy.dFilename);
//   strcpy(dDataname,copy.dDataname);
//   dType = copy.dType;            
//   dMode = copy.dMode;      
// }

void RDataContainer::operator=(const RDataContainer &copy)
{

}


RDataContainer::~RDataContainer()
{
  IsClosing(dObjName);

#ifdef RDC_DEBUG
  printf("In destructor RDataContainer\n");
#endif
}

void RDataContainer::IsClosing(char *objname)
{
  Emit("IsClosing(char*)",(long)objname);
}

void RDataContainer::ConnectWith(const TGWindow *rec, const char *recname)
{
  if(fMrec == NULL && rec != NULL){
    fMrec = (TGWindow*)rec;
    strcpy(dReceiverName,recname);
  }
}

void RDataContainer::CloseConnection()
{
  if(fMrec != NULL){
    fMrec = NULL;
    strcpy(dReceiverName,"");
  }
}

void RDataContainer::MakeCurrent()
{
  if(fRfile){
    if(fRfile->IsOpen())
      fRfile->cd();
  }
}

void RDataContainer::CloseFile(Bool_t kWrite)
{  
  if(dFp && dOpen)
    {
      fclose(dFp);
      dFp = NULL;
      dOpen = kFalse;
    }

  if(dType == FT_ROOT && fRfile != NULL){
    if(fRfile->IsOpen())
      {
	if(kWrite) fRfile->Write();
	fRfile->Close();
	delete fRfile;
	fRfile = NULL;
	dOpen = kFalse;
	if(fRootBrowser != NULL){
	  delete fRootBrowser;
	  fRootBrowser = NULL;
	}
      }
  }
}

void RDataContainer::OpenBrowser()
{
  if(dType == FT_ROOT && fRfile != NULL && fRfile->IsOpen()) {
    fRootBrowser = new TBrowser("fRootBrowser",fRfile,GetDataName());
    fImp = (TRootBrowser *)fRootBrowser->GetBrowserImp();	      
    fImp->Connect("Destroyed()","RDataContainer",
		  this,"BrowserClosed()");
    if(fMrec != NULL){
      fImp->Connect("DoubleClicked(TObject*)",dReceiverName,
		    (void*)fMrec,"GetObj(TObject*)");
    }
  }
}

void RDataContainer::CloseBrowser()
{
  if(fRootBrowser){
    delete fRootBrowser;
    fRootBrowser = NULL;
  }
}

void RDataContainer::BrowserClosed()
{
  fRootBrowser = NULL;  
}

void RDataContainer::FlushMessages()
{
  while(GetMessage() != NULL){}
}

char *RDataContainer::GetMessage()
{
  if(dMsgcnt != 0 && dMsgcnt <= MSG_QUEUE_MAX)
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

void RDataContainer::SendMessageSignal(char*objname)
{
  Emit("SendMessageSignal(char*)",(long)objname);
}

Bool_t RDataContainer::SetMessage(const char *msg, const char *func, int TS, int MESSAGETYPE)
{
  char str[NAME_STR_MAX];
  
  if(dMsgcnt == MSG_QUEUE_MAX){
    for(int i = 0; i < dMsgcnt-1; i++)
      strcpy(dMessage[i],dMessage[i+1]);
    memset(dMessage[MSG_QUEUE_MAX-1],'\0',sizeof(dMessage[MSG_QUEUE_MAX-1]));
    if(MESSAGETYPE == M_CONT_ERROR_MSG){ 
      strcat(dMessage[MSG_QUEUE_MAX-1],msg);
    }
    else
      strcpy(dMessage[MSG_QUEUE_MAX-1],msg);
  }
  else{
    if(MESSAGETYPE == M_CONT_ERROR_MSG){ 
      strcat(dMessage[dMsgcnt],msg);
    }
    else
      strcpy(dMessage[dMsgcnt],msg);
    dMsgcnt++;
  }

  if(MESSAGETYPE == M_CONT_ERROR_MSG) 
    sprintf(str,"%sEORROR",dObjName);
  else
    strcpy(str,dObjName);

  SendMessageSignal(str);
    
  return kTrue;
}

int RDataContainer::GetNumOfColumns()
{
  char l,prev='a';
  int flag = 0;
  int pos = 0;
  dColumns = 0;

  if(dMode != FM_WRITE){
    dColumns = 0;
    
    if(dFp)
      {
	pos = ftell(dFp);
	fseek(dFp,0,SEEK_SET);
	while(1){
	  flag = fscanf(dFp,"%c",&l);
	  if(flag < 0 ) break;
	  if((l == ' ' && (prev != ' ' && prev != ',')) ||
	     (l == ',' && (prev != ' ' && prev != ','))) dColumns++;
	  if(l == '\n') {dColumns++; break;};
	  prev = l;
	}
	fseek(dFp,pos,SEEK_SET);
	return dColumns;
      }
    else
      return -1;
  }
  return dColumns;
}

int RDataContainer::SetRowColumn(int row, int clmn)
{
  char l;
  int flag = 0;
  int rowcnt = 0;
  int clmncnt = 0;
  char prev = 'a';

  if(row < 1 || clmn < 1){
    FlushMessages();
    SetMessage(FILE_RCPOS_ERROR,"SetRowColumn",(int)dType,M_CONT_ERROR_MSG);
    return FILE_READ_ERROR;
  }
  else if(row > GetNumOfRows() || clmn > GetNumOfColumns()){
    FlushMessages();
    SetMessage(FILE_RCOFLOW_ERROR,"SetRowColumn",(int)dType,M_CONT_ERROR_MSG);
    return FILE_READ_ERROR;
  }
  else
    if(dMode != FM_WRITE){
      if(dFp)
	{
	  fseek(dFp,0,SEEK_SET);
	  while(1){
	    if(row-1 == rowcnt) break;
	    flag = fscanf(dFp,"%c",&l);
	    if (flag < 0) break;
	    if(l == '\n') rowcnt++;
	  }
	  while(1){
	    if(clmn-1 == clmncnt) break;
	    flag = fscanf(dFp,"%c",&l);
	    if(flag < 0 ) break;
	    if((l == ' ' && (prev != ' ' && prev != ',')) ||
	       (l == ',' && (prev != ' ' && prev != ','))) clmncnt++;
	    prev = l;
	  }
	  return FILE_PROCESS_OK;
	}
      else{
	FlushMessages();
	SetMessage(PNTR_NULL_ERROR,"SetRowColumn",(int)dType,M_CONT_ERROR_MSG);
	return FILE_READ_ERROR;
      }
    }
  FlushMessages();
  SetMessage(WRITE_MODE_ERROR,"SetRowColumn",(int)dType,M_CONT_ERROR_MSG);
  return FILE_READ_ERROR;
}


int RDataContainer::GetNumOfRows()
{
  char l;
  int flag = 0;
  dRows = 0;
  int pos = 0;

  if(dFp)
    {
      pos = ftell(dFp);
      fseek(dFp,0,SEEK_SET);
      while (1) {
	flag = fscanf(dFp,"%c",&l);
	if (flag < 0) break;
	
	if(l == '\n') dRows++;
      }     
      fseek(dFp,pos,SEEK_SET);
      return dRows;
    }
  else
    return -1;
}

int RDataContainer::GetRowLength(int row)
{
  if(row > GetNumOfRows()) return -1;

  char l;
  int flag = 0;
  int dLength = 0;
  int pos = 0;

  if(dFp)
    {
      pos = ftell(dFp);
      SetRowColumn(row,1);
      while (1) {
	flag = fscanf(dFp,"%c",&l);
	if(l == '\n') break;
	dLength++;
      }     
      fseek(dFp,pos,SEEK_SET);
      return dLength;
    }
  else
    return -1;

}

long int RDataContainer::GetFileSize()
{
  if(dType == FT_ROOT && fRfile != NULL)
    {
      if(fRfile->IsOpen())
	dFilesize = fRfile->GetEND();
      else
	dFilesize = -1;

      return dFilesize;
    }
  if(dFp)
    {
      fseek(dFp,0L,SEEK_END);
      dFilesize = ftell(dFp);
      fseek(dFp,0,SEEK_SET);
      return dFilesize;
    }
  
  return -1;
}

long int RDataContainer::GetCurrentPos()
{
  if(dType != FT_ROOT && dFp != NULL)
    return ftell(dFp);
  else
    return -1;
}

void RDataContainer::OnBrowserObject(TObject *obj)
{
  Emit("OnBrowserObject(TObject*)", (Long_t)obj);
}

TObject* RDataContainer::GetObjFromMapFile(const Char_t* name){
  //Get a TObject from the map file
  TObject* obj = 0;
  obj = fMapFile->Get(name,obj);
  return obj; 
};

TObject* RDataContainer::GetObjFromFile(const Char_t* name){
  //Get a TObject from the map file
  if(fRfile->IsOpen()){
    TObject* obj = NULL;
    obj = fRfile->FindObjectAny(name);
    if(obj) return obj;
    return NULL; 
  }
  return NULL;
};

int RDataContainer::OpenFile(const char *filename)
{
  if(!dOpen){
    strcpy(dFilename,filename);
    if(dType == FT_ROOT)
      {
	switch (dMode){
	case FM_WRITE:
	  fRfile = new TFile(GetFileName(),"RECREATE");
	  break;
	case FM_READ:
	  fRfile = new TFile(GetFileName(),"READ");
	  break;
	case FM_UPDATE:
	  fRfile = new TFile(GetFileName(),"UPDATE");
	  break;
	}
	
	if(fRfile != NULL && fRfile->IsOpen())
	  {
	    memset(dMiscbuffer2,'\0',sizeof(dMiscbuffer2));
	    sprintf(dMiscbuffer2,"Open: %s\nFilesize: %ld\n",
		    strrchr(GetFileName(),'/') ? strrchr(GetFileName(),'/') : 
		    GetFileName(),
		    GetFileSize());
	    sprintf(dMiscbuffer,"Number of objects in file: %d\n",
		    fRfile->GetNkeys());
	    strcat(dMiscbuffer2,dMiscbuffer);
	    SetMessage(dMiscbuffer2,"",(int)dType,M_CONT_LOGTXTTS);

	    if(dBrowserFlag) OpenBrowser();

	    dOpen = kTrue;
	    return FILE_PROCESS_OK;
	  }
	else{
	  FlushMessages();
	  SetMessage(FILE_OPEN_ERROR_MSG,"OpenFile",(int)dType,M_CONT_ERROR_MSG);
	  dOpen = kFalse;
	  return FILE_OPEN_ERROR;
	}
      }
    else if(dType == FT_ROWCOLUMN || dType == FT_MATHCAD) //Text Files arranged as columns of data
      {
	switch (dMode){
	case FM_WRITE:
	  dFp = fopen(GetFileName(),"w");
	  if(dFp)
	    {
	      if(GetNumOfColumns() < 0){return FILE_READ_ERROR;};
	      if(GetNumOfRows() < 0){return FILE_READ_ERROR;};
	      memset(dMiscbuffer2,'\0',sizeof(dMiscbuffer2));
	      sprintf(dMiscbuffer2,"Open: %s\nFilesize: %ld\n",
		      strrchr(GetFileName(),'/') ? strrchr(GetFileName(),'/') : 
		      GetFileName(),
		      GetFileSize());
	      sprintf(dMiscbuffer,"Rows = %d;Columns = %d\n",GetNumOfRows(),
		      GetNumOfColumns());
	      strcat(dMiscbuffer2,dMiscbuffer);
	      SetMessage(dMiscbuffer2,"",(int)dType,M_CONT_LOGTXTTS);
	      dOpen = kTrue;		
	      return FILE_PROCESS_OK;
	    }
	  else
	    {
	      FlushMessages();
	      SetMessage(FILE_OPEN_ERROR_MSG,"OpenFile",(int)dType,M_CONT_ERROR_MSG);
	      dOpen = kFalse;
	      dFp = NULL;
	      return FILE_OPEN_ERROR;
	    }
	  break;
	case FM_READ:
	  dFp = fopen(GetFileName(),"r");
	  if(dFp)
	    {
	      if(GetNumOfColumns() < 1){return FILE_READ_ERROR;};
	      if(GetNumOfRows() < 1){return FILE_READ_ERROR;};
	      memset(dMiscbuffer2,'\0',sizeof(dMiscbuffer2));
	      sprintf(dMiscbuffer2,"Open: %s\nFilesize: %ld\n",
		      strrchr(GetFileName(),'/') ? strrchr(GetFileName(),'/') : 
		      GetFileName(),
		      GetFileSize());
	      sprintf(dMiscbuffer,"Rows = %d;Columns = %d\n",GetNumOfRows(),
		      GetNumOfColumns());
	      strcat(dMiscbuffer2,dMiscbuffer);
	      SetMessage(dMiscbuffer2,"",(int)dType,M_CONT_LOGTXTTS);
	      dOpen = kTrue;
	      return FILE_PROCESS_OK;
	    }
	  else
	    {
	      FlushMessages();
	      SetMessage(FILE_OPEN_ERROR_MSG,"OpenFile",(int)dType,M_CONT_ERROR_MSG);
	      dOpen = kFalse;
	      dFp = NULL;
	      return FILE_OPEN_ERROR;
	    }
	  break;
	case FM_UPDATE:
	  dFp = fopen(GetFileName(),"a+");
	  if(dFp)
	    {
	      if(GetNumOfColumns() < 0){return FILE_READ_ERROR;};
	      if(GetNumOfRows() < 0){return FILE_READ_ERROR;};
	      memset(dMiscbuffer2,'\0',sizeof(dMiscbuffer2));
	      sprintf(dMiscbuffer2,"Open: %s\nFilesize: %ld\n",
		      strrchr(GetFileName(),'/') ? strrchr(GetFileName(),'/') : 
		      GetFileName(),
		      GetFileSize());
	      sprintf(dMiscbuffer,"Rows = %d;Columns = %d\n",GetNumOfRows(),
		      GetNumOfColumns());
	      strcat(dMiscbuffer2,dMiscbuffer);
	      SetMessage(dMiscbuffer2,"",(int)dType,M_CONT_LOGTXTTS);
	      dOpen = kTrue;
	      return FILE_PROCESS_OK;
	    }
	  else
	    {
	      FlushMessages();
	      SetMessage(FILE_OPEN_ERROR_MSG,"OpenFile",(int)dType,M_CONT_ERROR_MSG);
	      dOpen = kFalse;
	      dFp = NULL;
	      return FILE_OPEN_ERROR;
	    }
	  break;
	}
      }
    else if(dType == FT_TEXT)
      {
	switch (dMode){
	case FM_WRITE:
	  dFp = fopen(GetFileName(),"w");
	  if(dFp)
	    {
	      memset(dMiscbuffer2,'\0',sizeof(dMiscbuffer2));
	      sprintf(dMiscbuffer2,"Open: %s\nFilesize: %ld\n",
		      strrchr(GetFileName(),'/') ? strrchr(GetFileName(),'/') : 
		      GetFileName(),
		      GetFileSize());
	      SetMessage(dMiscbuffer2,"",(int)dType,M_CONT_LOGTXTTS);
	      dOpen = kTrue;		
	      return FILE_PROCESS_OK;
	    }
	  else
	    {
	      FlushMessages();
	      SetMessage(FILE_OPEN_ERROR_MSG,"OpenFile",(int)dType,M_CONT_ERROR_MSG);
	      dOpen = kFalse;
	      dFp = NULL;
	      return FILE_OPEN_ERROR;
	    }
	  break;
	case FM_READ:
	  dFp = fopen(GetFileName(),"r");
	  if(dFp)
	    {
	      memset(dMiscbuffer2,'\0',sizeof(dMiscbuffer2));
	      sprintf(dMiscbuffer2,"Open: %s\nFilesize: %ld\n",
		      strrchr(GetFileName(),'/') ? strrchr(GetFileName(),'/') : 
		      GetFileName(),
		      GetFileSize());
	      SetMessage(dMiscbuffer2,"",(int)dType,M_CONT_LOGTXTTS);
	      dOpen = kTrue;
	      return FILE_PROCESS_OK;
	    }
	  else
	    {
	      FlushMessages();
	      SetMessage(FILE_OPEN_ERROR_MSG,"OpenFile",(int)dType,M_CONT_ERROR_MSG);
	      dOpen = kFalse;
	      dFp = NULL;
	      return FILE_OPEN_ERROR;
	    }
	  break;
	case FM_UPDATE:
	  dFp = fopen(GetFileName(),"a+");
	  if(dFp)
	    {
	      memset(dMiscbuffer2,'\0',sizeof(dMiscbuffer2));
	      sprintf(dMiscbuffer2,"Open: %s\nFilesize: %ld\n",
		      strrchr(GetFileName(),'/') ? strrchr(GetFileName(),'/') : 
		      GetFileName(),
		      GetFileSize());
	      SetMessage(dMiscbuffer2,"",(int)dType,M_CONT_LOGTXTTS);
	      dOpen = kTrue;
	      return FILE_PROCESS_OK;
	    }
	  else
	    {
	      FlushMessages();
	      SetMessage(FILE_OPEN_ERROR_MSG,"OpenFile",(int)dType,M_CONT_ERROR_MSG);
	      dOpen = kFalse;
	      dFp = NULL;
	      return FILE_OPEN_ERROR;
	    }
	  break;
	}
      }
    else if(dType == FT_BINARY)
      { 
	switch (dMode){
	case FM_WRITE:
	  dFp = fopen(GetFileName(),"wb");
	  if(dFp)
	    {
	      memset(dMiscbuffer2,'\0',sizeof(dMiscbuffer2));
	      sprintf(dMiscbuffer2,"Open: %s\nFilesize: %ld\n",
		      strrchr(GetFileName(),'/') ? strrchr(GetFileName(),'/') : 
		      GetFileName(),
		      GetFileSize());
	      SetMessage(dMiscbuffer2,"",(int)dType,M_CONT_LOGTXTTS);
	      dOpen = kTrue;
	      return FILE_PROCESS_OK;
	    }
	  else
	    {
	      FlushMessages();
	      SetMessage(FILE_OPEN_ERROR_MSG,"OpenFile",(int)dType,M_CONT_ERROR_MSG);
	      dOpen = kFalse;
	      dFp = NULL;
	      return FILE_OPEN_ERROR;
	    }
	  break;
	case FM_READ:
	  dFp = fopen(GetFileName(),"rb");
	  if(dFp)
	    {
	      memset(dMiscbuffer2,'\0',sizeof(dMiscbuffer2));
	      sprintf(dMiscbuffer2,"Open: %s\nFilesize: %ld\n",
		      strrchr(GetFileName(),'/') ? strrchr(GetFileName(),'/') : 
		      GetFileName(),
		      GetFileSize());
	      SetMessage(dMiscbuffer2,"",(int)dType,M_CONT_LOGTXTTS);
	      dOpen = kTrue;
	      return FILE_PROCESS_OK;
	    }
	  else
	    {
	      FlushMessages();
	      SetMessage(FILE_OPEN_ERROR_MSG,"OpenFile",(int)dType,M_CONT_ERROR_MSG);
	      dOpen = kFalse;
	      dFp = NULL;
	      return FILE_OPEN_ERROR;
	    }
	  break;
	case FM_UPDATE:
	  dFp = fopen(GetFileName(),"r+b");
	  if(dFp)
	    {
	      memset(dMiscbuffer2,'\0',sizeof(dMiscbuffer2));
	      sprintf(dMiscbuffer2,"Open: %s\nFilesize: %ld\n",
		      strrchr(GetFileName(),'/') ? strrchr(GetFileName(),'/') : 
		      GetFileName(),
		      GetFileSize());
	      SetMessage(dMiscbuffer2,"",(int)dType,M_CONT_LOGTXTTS);
	      dOpen = kTrue;
	      return FILE_PROCESS_OK; 
	    }
	  else
	    {
	      FlushMessages();
	      SetMessage(FILE_OPEN_ERROR_MSG,"OpenFile",(int)dType,M_CONT_ERROR_MSG);
	      dOpen = kFalse;
	      dFp = NULL;
	      return FILE_OPEN_ERROR;
	    }
	  break;
	}
      }
  }
  else{
    FlushMessages(); 
    SetMessage(FILE_CONT_OCCUPIED,"OpenFile",(int)dType,M_CONT_ERROR_MSG);
    return FILE_OPEN_ERROR;
  }
  
  dOpen = kFalse;
  dFp = NULL;
  return FILE_OPEN_ERROR;
}


Int_t RDataContainer::OpenMapFile(const char* file){

  try {
    fMapFile = TMapFile::Create(file);
    fMapFile->Print();
  }
  catch( char * str ) {
    //cout << "Exception raised: " << str << '\n';
  }
  return 1;
};

int RDataContainer::GetNumOfRootObjects()
{
  if(fRfile != NULL) {
    if(fRfile->IsOpen()){      
      return fRfile->GetNkeys();
    }
    else{
      FlushMessages();
      SetMessage(ROOT_OPEN_ERROR,"GetNumOfRootObjects",(int)dType,M_CONT_ERROR_MSG);
      return -1;
    }
  }
  FlushMessages();
  SetMessage(PNTR_NULL_ERROR,"GetNumOfRootObjects",(int)dType,M_CONT_ERROR_MSG);
  return -1;
}

int RDataContainer::GetListOfRootObjects(TString str[])
{
  if(fRfile != NULL){
    if(fRfile->IsOpen()){
      TIter next(fRfile->GetListOfKeys());
      TKey *key;
      int i = 0;
      while((key=(TKey*)next())){
	str[i] = key->GetName();
	i++;
      }
      return FILE_PROCESS_OK;
    }
    else{
      FlushMessages();
      SetMessage(ROOT_OPEN_ERROR,"GetListOfRootObjects",(int)dType,M_CONT_ERROR_MSG);
      return FILE_READ_ERROR;
    }
  }

  FlushMessages();
  SetMessage(PNTR_NULL_ERROR,"GetListOfRootObjects",(int)dType,M_CONT_ERROR_MSG);
  return FILE_READ_ERROR;
}

char *RDataContainer::GetObjectType(const char *name)
{
  if(fRfile != NULL){
    if(fRfile->IsOpen()){
      
      return (char*)fRfile->Get(name)->ClassName();
    }
    else{
      FlushMessages();
      SetMessage(ROOT_OPEN_ERROR,"GetObjectType",(int)dType,M_CONT_ERROR_MSG);
      return NULL;
    }
  }
  FlushMessages();
  SetMessage(PNTR_NULL_ERROR,"GetObjectType",(int)dType,M_CONT_ERROR_MSG);
  return NULL;
}

Bool_t RDataContainer::IsObjectType(const char *objname, const char *type)
{
  if(!strcmp(type,GetObjectType(objname)))
    return kTrue;
  
  return kFalse;
}

Int_t  RDataContainer::GetHighestKey(char *name, Int_t mkey) 
{
  if(!fRfile)  return 0;
  if(!fRfile->IsOpen())  return 0;
  TKey *key = fRfile->GetKey(name,mkey);
  if(!key) return 0;
  return key->GetCycle(); 
}

Int_t RDataContainer::GetNumberOfKeys()
{
  if(!fRfile)  return 0;
  if(!fRfile->IsOpen())  return 0;  
  return fRfile->GetNkeys();
}

void  RDataContainer::DeleteObjectsWithKey(char *mkey) 
{
  if(!fRfile)  return;
  if(!fRfile->IsOpen())  return;    
  fRfile->Delete(mkey);
}; 


Bool_t RDataContainer::cd()
{
  if(fRfile && fRfile->IsOpen()){
    return fRfile->cd();
  }
  return kFalse;
}

Bool_t RDataContainer::cd(char *dir)
{
  if(fRfile && fRfile->IsOpen()){
    return fRfile->cd(dir);
  }
  return kFalse;
}

Bool_t RDataContainer::mkdir(char *dir)
{
  if(fRfile && fRfile->IsOpen()){
    return fRfile->mkdir(dir);
  }
  return kFalse;
}

TObject *RDataContainer::ReadData(const char *objname)
{
  //This function reads data from file "dFilename" and stores it
  //in "buffer" as follows:
  //
  //If the file format is of "root object" (*.root), then...
  //
  //The function performs mode (dMode) checks and returns without
  //doing anything except pass an error message if the file was 
  //opened as write only.

  TObject *obj;

  if(dMode == FM_UPDATE || dMode == FM_READ){  
    if(dType == FT_ROOT){
      if(fRfile != NULL){
	if(fRfile->IsOpen()){
	  obj = fRfile->Get(objname);
	  if(obj != NULL){
	    return obj;
	  }
	  else{
	    FlushMessages();
	    sprintf(dMiscbuffer,"%s for %s",ROOT_OBJCRT_ERROR,objname);
	    SetMessage(dMiscbuffer,"ReadData",(int)dType,M_CONT_ERROR_MSG);
	    // SetMessage(ROOT_OBJCRT_ERROR,"ReadData",(int)dType,M_CONT_ERROR_MSG);
	    return NULL;	
	  }
	}
	else{
	  FlushMessages();
	  SetMessage(ROOT_OPEN_ERROR,"ReadData",(int)dType,M_CONT_ERROR_MSG);
	  return NULL;
	}
      }
      else{
	FlushMessages();
	SetMessage(PNTR_NULL_ERROR,"ReadData",(int)dType,M_CONT_ERROR_MSG);
	return NULL;
      }
    }
    else {
      FlushMessages();
      SetMessage(READ_TYPE_ERROR,"ReadData",(int)dType,M_CONT_ERROR_MSG);
      return NULL;
    }
  }
  else{
    FlushMessages();
    SetMessage(WRITE_MODE_ERROR,"ReadData",(int)dType,M_CONT_ERROR_MSG);
    return NULL;
  }
  
}

int RDataContainer::ReadData(const char* objname,Double_t *x, 
			     Double_t *w, Double_t *y, Double_t *z)
{
  Int_t binx, biny, binz;
  Int_t firstx=0,lastx=0,firsty=0,lasty=0,firstz=0,lastz=0;

  if(ReadData(objname)){
    if(x && w && !y && !z){
      TH1D hst = *dynamic_cast<TH1D*>(ReadData(objname));
      lastx  = hst.GetXaxis()->GetNbins();      
      for (binx=firstx;binx<lastx;binx++){
	x[binx] = hst.GetXaxis()->GetBinCenter(binx);
	w[binx] = hst.GetBinContent(binx);
      }
    }
    else if(x && w && y && !z){
      if(IsObjectType(objname,"TH2D")){
	TH2D hst = *dynamic_cast<TH2D*>(ReadData(objname));
	lastx  = hst.GetXaxis()->GetNbins();      
	lasty  = hst.GetYaxis()->GetNbins();      
	for (binx=firstx;binx<lastx;binx++){
	  x[binx] = hst.GetXaxis()->GetBinCenter(binx);
	  for (biny=firsty;biny<lasty;biny++){
	    y[biny] = hst.GetYaxis()->GetBinCenter(biny);
	    w[biny+binx*lasty] = hst.GetBinContent(binx,biny);
	  }
	}
      }
      else if (IsObjectType(objname,"TH3D")){
	TH3D hst = *dynamic_cast<TH3D*>(ReadData(objname));
	lastx  = hst.GetXaxis()->GetNbins();      
	lasty  = hst.GetYaxis()->GetNbins();      
	for (binx=firstx;binx<lastx;binx++){
	  x[binx] = hst.GetXaxis()->GetBinCenter(binx);
	  for (biny=firsty;biny<lasty;biny++){
	    y[biny] = hst.GetYaxis()->GetBinCenter(biny);
	    w[biny+binx*lasty] = hst.GetBinContent(binx,biny);
	  }
	}
      }
      else if (IsObjectType(objname,"TH2F")){
	TH2F hst = *dynamic_cast<TH2F*>(ReadData(objname));
	lastx  = hst.GetXaxis()->GetNbins();      
	lasty  = hst.GetYaxis()->GetNbins();      
	for (binx=firstx;binx<lastx;binx++){
	  x[binx] = hst.GetXaxis()->GetBinCenter(binx);
	  for (biny=firsty;biny<lasty;biny++){
	    y[biny] = hst.GetYaxis()->GetBinCenter(biny);
	    w[biny+binx*lasty] = hst.GetBinContent(binx,biny);
	  }
	}
      }
      else if (IsObjectType(objname,"TH3F")){
	TH3F hst = *dynamic_cast<TH3F*>(ReadData(objname));
	lastx  = hst.GetXaxis()->GetNbins();      
	lasty  = hst.GetYaxis()->GetNbins();      
	for (binx=firstx;binx<lastx;binx++){
	  x[binx] = hst.GetXaxis()->GetBinCenter(binx);
	  for (biny=firsty;biny<lasty;biny++){
	    y[biny] = hst.GetYaxis()->GetBinCenter(biny);
	    w[biny+binx*lasty] = hst.GetBinContent(binx,biny);
	  }
	}
      }
      else if (IsObjectType(objname,"TH2S")){
	TH2S hst = *dynamic_cast<TH2S*>(ReadData(objname));
	lastx  = hst.GetXaxis()->GetNbins();      
	lasty  = hst.GetYaxis()->GetNbins();      
	for (binx=firstx;binx<lastx;binx++){
	  x[binx] = hst.GetXaxis()->GetBinCenter(binx);
	  for (biny=firsty;biny<lasty;biny++){
	    y[biny] = hst.GetYaxis()->GetBinCenter(biny);
	    w[biny+binx*lasty] = hst.GetBinContent(binx,biny);
	  }
	}
      }
      else if (IsObjectType(objname,"TH3S")){
	TH3S hst = *dynamic_cast<TH3S*>(ReadData(objname));
	lastx  = hst.GetXaxis()->GetNbins();      
	lasty  = hst.GetYaxis()->GetNbins();      
	for (binx=firstx;binx<lastx;binx++){
	  x[binx] = hst.GetXaxis()->GetBinCenter(binx);
	  for (biny=firsty;biny<lasty;biny++){
	    y[biny] = hst.GetYaxis()->GetBinCenter(biny);
	    w[biny+binx*lasty] = hst.GetBinContent(binx,biny);
	  }
	}
      }
      else{
	FlushMessages();
	SetMessage(ROOT_DIM_ERROR,"ReadData(char*,Double_t*,Double_t*,Double_t*)",
		 (int)dType,M_CONT_ERROR_MSG);
	return FILE_READ_ERROR;	
      }
    }
    else if(x && y && z && w){
      if (IsObjectType(objname,"TH3D")){
	TH3D hst = *dynamic_cast<TH3D*>(ReadData(objname));
	lastx  = hst.GetXaxis()->GetNbins();      
	lasty  = hst.GetYaxis()->GetNbins();      
	lastz  = hst.GetZaxis()->GetNbins();      
	for (binx=firstx;binx<lastx;binx++){
	  x[binx] = hst.GetXaxis()->GetBinCenter(binx);
	  for (biny=firsty;biny<lasty;biny++){
	    y[biny] = hst.GetYaxis()->GetBinCenter(biny);
	    for (binz=firstz;binz<lastz;binz++){
	      z[binz] = hst.GetZaxis()->GetBinCenter(binz);
	      w[binz+biny*lastz+binx*lasty*lastz] = hst.GetBinContent(binx,biny,binz);
	    }
	  }
	}
      }
      else if (IsObjectType(objname,"TH3F")){
	TH3F hst = *dynamic_cast<TH3F*>(ReadData(objname));
	lastx  = hst.GetXaxis()->GetNbins();      
	lasty  = hst.GetYaxis()->GetNbins();      
	lastz  = hst.GetZaxis()->GetNbins();      
	for (binx=firstx;binx<lastx;binx++){
	  x[binx] = hst.GetXaxis()->GetBinCenter(binx);
	  for (biny=firsty;biny<lasty;biny++){
	    y[biny] = hst.GetYaxis()->GetBinCenter(biny);
	    for (binz=firstz;binz<lastz;binz++){
	      z[binz] = hst.GetZaxis()->GetBinCenter(binz);
	      w[binz+biny*lastz+binx*lasty*lastz] = hst.GetBinContent(binx,biny,binz);
	    }
	  }
	}
      }
      else if (IsObjectType(objname,"TH3S")){
	TH3S hst = *dynamic_cast<TH3S*>(ReadData(objname));
	lastx  = hst.GetXaxis()->GetNbins();      
	lasty  = hst.GetYaxis()->GetNbins();      
	lastz  = hst.GetZaxis()->GetNbins();      
	for (binx=firstx;binx<lastx;binx++){
	  x[binx] = hst.GetXaxis()->GetBinCenter(binx);
	  for (biny=firsty;biny<lasty;biny++){
	    y[biny] = hst.GetYaxis()->GetBinCenter(biny);
	    for (binz=firstz;binz<lastz;binz++){
	      z[binz] = hst.GetZaxis()->GetBinCenter(binz);
	      w[binz+biny*lastz+binx*lasty*lastz] = hst.GetBinContent(binx,biny,binz);
	    }
	  }
	}
      }
      else{
	FlushMessages();
	SetMessage(ROOT_DIM_ERROR,"ReadData(char*,Double_t*,Double_t*,Double_t*)",
		 (int)dType,M_CONT_ERROR_MSG);
	return FILE_READ_ERROR;	
      }
    }
    else if(!x && y && !z){

    }
    else if(!x && y && z){

    }
    else if(!x && !y && z){

    }
    else if(x && !y && z){

    }
    else if(!x && !y && !z){
      FlushMessages();
      SetMessage(PARM_NULL_ERROR,"ReadData(char*,Double_t*,Double_t*,Double_t*)",
		 (int)dType,M_CONT_ERROR_MSG);
      return FILE_READ_ERROR;
    }
  }
  else{
    strcpy(dMiscbuffer,"From ReadData(char*): ");
    strcat(dMiscbuffer,GetMessage());
    strcat(dMiscbuffer,ROOT_OBJFIND_ERROR);
    FlushMessages();
    SetMessage(dMiscbuffer,"ReadData(char*,Double_t*,Double_t*,Double_t*)",
	       (int)dType,M_CONT_ERROR_MSG);
    return FILE_READ_ERROR;
  }

  return FILE_PROCESS_OK;
}

int RDataContainer::ReadData(Double_t *data, int rows, int cols)
{
  int nbytes = 0;

  if(GetNumOfColumns()*GetNumOfRows() > rows*cols) return FILE_READ_ERROR;

  if(dOpen) {
    if(dMode == FM_UPDATE || dMode == FM_READ){
      if(dType == FT_ROWCOLUMN){
	if(dFp){ 
	  if(!fseek(dFp,0,SEEK_SET)){

	    for(int r = 0; r < rows; r++){
	      for(int c = 0; c < cols; c++){
		nbytes += fscanf(dFp,"%le",&data[(int)(c*rows + r)]);
// 		if(c < cols-1) nbytes += fscanf(dFp," ");
	      }  
	    }
	    return FILE_PROCESS_OK;
	  }
	  else{
	    FlushMessages();
	    SetMessage(PNTR_POS_ERROR,"ReadData",(int)dType,M_CONT_ERROR_MSG);
	    return(FILE_READ_ERROR);
	  }
	}
	else{
	  FlushMessages();
	  SetMessage(PNTR_NULL_ERROR,"ReadData",(int)dType,M_CONT_ERROR_MSG);
	  return(FILE_READ_ERROR);
	}
      }
      else{
	FlushMessages();
	SetMessage(READ_TYPE_ERROR,"ReadData",(int)dType,M_CONT_ERROR_MSG);
	return FILE_READ_ERROR;
      }
    }
    else
      {
	FlushMessages();
	SetMessage(READ_MODE_ERROR,"ReadData",(int)dType,M_CONT_ERROR_MSG);
	return FILE_READ_ERROR;
      }
    return FILE_PROCESS_OK;
  }
  FlushMessages();
  SetMessage(FILE_NOT_OPEN,"ReadData",(int)dType,M_CONT_ERROR_MSG);
  return FILE_READ_ERROR;
}


TObject *RDataContainer::ReadTree(const char *treename)
{
  TObject *obj;
  TTree *temptree;
  if(dMode == FM_UPDATE || dMode == FM_READ){  
    if(dType == FT_ROOT){
      if(fRfile != NULL){
	if(fRfile->IsOpen()){
	  temptree = (TTree*) fRfile->Get(treename);
	  obj = (TObject*) temptree;
	  //	  obj = fRfile->Get(objname);
	  if(obj != NULL){
	    return obj;
	  }
	  else{
	    FlushMessages();
	    SetMessage(ROOT_OBJCRT_ERROR,"ReadData",(int)dType,M_CONT_ERROR_MSG);
	    return NULL;	
	  }
	}
	else{
	  FlushMessages();
	  SetMessage(ROOT_OPEN_ERROR,"ReadData",(int)dType,M_CONT_ERROR_MSG);
	  return NULL;
	}
      }
      else{
	FlushMessages();
	SetMessage(PNTR_NULL_ERROR,"ReadData",(int)dType,M_CONT_ERROR_MSG);
	return NULL;
      }
    }
    else {
      FlushMessages();
      SetMessage(READ_TYPE_ERROR,"ReadData",(int)dType,M_CONT_ERROR_MSG);
      return NULL;
    }
  }
  else{
    FlushMessages();
    SetMessage(WRITE_MODE_ERROR,"ReadData",(int)dType,M_CONT_ERROR_MSG);
    return NULL;
  }
  
}

int RDataContainer::ReadData(Double_t *x, int row)
{
  //Read a whole row of a typical row-column data files
  
  int flag;
  int numcol;
//   Double_t trash;
  if(dOpen) {
    if(dMode == FM_UPDATE || dMode == FM_READ){
      if(dType == FT_ROWCOLUMN){
	if(dFp){ 
	  numcol = GetNumOfColumns();
	  if(x){
	    if(row > 0 && row <= GetNumOfRows()){
	      if(SetRowColumn(row,1) == FILE_PROCESS_OK){
		for(int j = 0; j < numcol; j++){
		  if(j == numcol-1) 
		    flag = fscanf(dFp,"%lf",&x[j]);
		  else
		    flag = fscanf(dFp,"%lf ",&x[j]);
		}
	      }
	      else{
		//SetRowColumn has already set the error message
		return(FILE_READ_ERROR);
	      }
	    }
	    else{
	      FlushMessages();
	      SetMessage(FILE_RCOFLOW_ERROR,"ReadData",(int)dType,M_CONT_ERROR_MSG);
	      return(FILE_READ_ERROR);
	    }
	  }
	}
	else{
	  FlushMessages();
	  SetMessage(PNTR_NULL_ERROR,"ReadData",(int)dType,M_CONT_ERROR_MSG);
	  return(FILE_READ_ERROR);
	}
      }
      else{
	FlushMessages();
	SetMessage(READ_TYPE_ERROR,"ReadData",(int)dType,M_CONT_ERROR_MSG);
	return FILE_READ_ERROR;
      }
    }
    else
      {
	FlushMessages();
	SetMessage(WRITE_MODE_ERROR,"ReadData",(int)dType,M_CONT_ERROR_MSG);
	return FILE_READ_ERROR;
      }
    return FILE_PROCESS_OK;
  }
  FlushMessages();
  SetMessage(FILE_NOT_OPEN,"ReadData",(int)dType,M_CONT_ERROR_MSG);
  return FILE_READ_ERROR;
}

int RDataContainer::ReadData(Double_t *x, int ri, int c,int num)
{
  int flag;
  int numcol;
  Double_t trash;
  if(dOpen) {
    if(dMode == FM_UPDATE || dMode == FM_READ){
      if(dType == FT_ROWCOLUMN){
	if(dFp){ 
	  numcol = GetNumOfColumns();
	  if(x){
	    if(ri > 0 && c > 0 && c <= numcol &&
	       ri <= (GetNumOfRows()-num+1) &&
	       num <= (GetNumOfRows()-ri+1)){
	      if(SetRowColumn(ri,1) == FILE_PROCESS_OK){
		
		for(int i = 0; i < num; i++){
		  for(int j = 0; j < numcol; j++){
		    if(j == c-1)
		      flag = fscanf(dFp,"%lf ",&x[i]);
		    else
		      flag = fscanf(dFp,"%lf ",&trash);
		  }
		}
	      }
	      else{
		//SetRowColumn has already set the error message
		return(FILE_READ_ERROR);
	      }
	    }
	    else{
	      FlushMessages();
	      SetMessage(FILE_RCOFLOW_ERROR,"ReadData",(int)dType,M_CONT_ERROR_MSG);
	      return(FILE_READ_ERROR);
	    }
	  }
	}
	else{
	  FlushMessages();
	  SetMessage(PNTR_NULL_ERROR,"ReadData",(int)dType,M_CONT_ERROR_MSG);
	  return(FILE_READ_ERROR);
	}
      }
      else{
	FlushMessages();
	SetMessage(READ_TYPE_ERROR,"ReadData",(int)dType,M_CONT_ERROR_MSG);
	return FILE_READ_ERROR;
      }
    }
    else
      {
	FlushMessages();
	SetMessage(WRITE_MODE_ERROR,"ReadData",(int)dType,M_CONT_ERROR_MSG);
	return FILE_READ_ERROR;
      }
    return FILE_PROCESS_OK;
  }
  FlushMessages();
  SetMessage(FILE_NOT_OPEN,"ReadData",(int)dType,M_CONT_ERROR_MSG);
  return FILE_READ_ERROR;
}

int RDataContainer::ReadData(Double_t *x, Double_t *y, 
			     int ri, int c1,int c2, int num)
{
  //This function reads data from file "dFilename" and stores it
  //in "buffer" as follows:
  //
  //The function expects to read from a text file consisting of
  //rows and columns of text representing numbers. Here the
  //numbers are interpreted as type double.
  //
  //The function expects the paramters "x" and "y" to be 
  //arrays. A number of elements equal to 2*"num" are read from the 
  //file into the arrays, starting at row "ri" and column "c1" in
  //the following way: 
  //
  //Assume the file containes the following:
  //
  //                  1  7  3  2
  //                 12 16  8 21
  //                  9  0  0  6
  //                 11 26  1  4
  //                 13  5 19 25
  //                 16  8 27  0
  //
  //If (ri,c1,c2)=(2,2,3) and "num" = 4, then array 
  //x = [16,0,26,5] and array y = [8,0,1,19]. If c2 = 4, then
  //y = [21,6,4,25] and array x will stay the same.
  //
  //If "num" exceeds the number of elements allocated in either
  //"x" or "y" the program will crash.
  //
  //The function performs mode (dMode) checks and returns without
  //doing anything except pass an error message if the file was 
  //opened as write only.

  int flag;
  int numcol;
  Double_t trash;
  int skip = abs(c2-c1)-1;
  int ci = c1;

  if(dOpen) {
    if(dMode == FM_UPDATE || dMode == FM_READ){
      if(dType == FT_ROWCOLUMN){
	if(dFp){ 
	  numcol = GetNumOfColumns();
	  if(x && y){
	    if(ri > 0 && ci > 0 && 
	       ci <= (numcol-1-skip) &&
	       ri <= (GetNumOfRows()-num+1) &&
	       num <= (GetNumOfRows()-ri+1) &&
	       skip < (numcol-ci)){
	      if(SetRowColumn(ri,1) == FILE_PROCESS_OK){
		
		for(int i = 0; i < num; i++){
		  for(int j = 0; j < numcol; j++){
		    if(j == ci-1)
		      flag = fscanf(dFp,"%lf ",&x[i]);
		    else if(j == ci+skip) 
		      flag = fscanf(dFp,"%lf ",&y[i]);
		    else
		      flag = fscanf(dFp,"%lf ",&trash);
		  }
		}
	      }
	      else{
		//SetRowColumn has already set the error message
		return(FILE_READ_ERROR);
	      }
	    }
	    else{
	      FlushMessages();
	      SetMessage(FILE_RCOFLOW_ERROR,"ReadData",(int)dType,M_CONT_ERROR_MSG);
	      return(FILE_READ_ERROR);
	    }
	  }
	  else if(x && !y){
	    if(ri > 0 && 
	       ri <= (GetNumOfRows()-num+1) &&
	       num <= (GetNumOfRows()-ri+1)){
	      if(SetRowColumn(ri,1) == FILE_PROCESS_OK){
		
		for(int i = 0; i < num; i++){
		  flag = fscanf(dFp,"%lf ",&x[i]);		  
		}
	      }
	      else{
		//SetRowColumn has already set the error message
		return(FILE_READ_ERROR);
	      }
	    }
	    else{
	      FlushMessages();
	      SetMessage(FILE_RCOFLOW_ERROR,"ReadData",(int)dType,M_CONT_ERROR_MSG);
	      return(FILE_READ_ERROR);
	    }
	  }	  
	}
	else{
	  FlushMessages();
	  SetMessage(PNTR_NULL_ERROR,"ReadData",(int)dType,M_CONT_ERROR_MSG);
	  return(FILE_READ_ERROR);
	}
      }
      else{
	FlushMessages();
	SetMessage(READ_TYPE_ERROR,"ReadData",(int)dType,M_CONT_ERROR_MSG);
	return FILE_READ_ERROR;
      }
    }
    else
      {
	FlushMessages();
	SetMessage(WRITE_MODE_ERROR,"ReadData",(int)dType,M_CONT_ERROR_MSG);
	return FILE_READ_ERROR;
      }
    return FILE_PROCESS_OK;
  }
  FlushMessages();
  SetMessage(FILE_NOT_OPEN,"ReadData",(int)dType,M_CONT_ERROR_MSG);
  return FILE_READ_ERROR;

}


// int RDataContainer::GetSectionPointer(char before, char after, int *start, int occurence)
// {
//   char l;
//   int flag1 = 0;
//   int flag2 = 0;
//   int occ = 0;
//   int pos = 0;

//   if(dFp)
//     {
//       pos = ftell(dFp);
//       fseek(dFp,0,SEEK_SET);
//       while (1) {
// 	flag = fscanf(dFp,"%c",&l);
// 	if (flag < 0) break;
	
// 	if(l == before) {occ++;}
// 	if(occ == occurence) {
// 	  start = ftell(dFp);
// 	  flag1 = 1;  break;
// 	}
//       }     
      

//       fseek(dFp,pos,SEEK_SET);
//       return dRows;
//     }
//   else
//     return -1;


//   fread((char*)buffer,sizeof(char),size-1,dFp)

    
// }

int RDataContainer::ReadRow(const char *buffer, int row)
{
  if(!dFp || !buffer) return FILE_READ_ERROR;
  if(row < 1 || row > GetNumOfRows()) return FILE_READ_ERROR;
  SetRowColumn(row,1);
  if(!fread((char*)buffer,GetRowLength(row),1,dFp)) {
    FlushMessages();
    SetMessage(READ_PNTR_ERROR,"ReadData",(int)dType,M_CONT_ERROR_MSG);
    return(FILE_READ_ERROR);
  }
  return FILE_PROCESS_OK;
}

int RDataContainer::ReadData(const char *buffer, int size)
{
  if(!fread((char*)buffer,size,1,dFp)) {
    FlushMessages();
    SetMessage(READ_PNTR_ERROR,"ReadData",(int)dType,M_CONT_ERROR_MSG);
    return(FILE_READ_ERROR);
  }
  return FILE_PROCESS_OK;
}

int RDataContainer::SeekPosition(int start)
{
  if(dFp){ 
    if(start >= 0){
      if(!fseek(dFp,start,SEEK_SET)){
	return FILE_PROCESS_OK;
      }
      else{
	fseek(dFp,0,SEEK_SET);
	FlushMessages();
	SetMessage(PNTR_POS_ERROR,"SeekPosition()",(int)dType,M_CONT_ERROR_MSG);
	return(FILE_READ_ERROR);
      }
    }
    else{
      fseek(dFp,0,SEEK_SET);
      FlushMessages();
      SetMessage(PNTR_RUN_ERROR,"ReadData",(int)dType,M_CONT_ERROR_MSG);
      return(FILE_READ_ERROR);
    }
  }
  else{
    FlushMessages();
    SetMessage(PNTR_NULL_ERROR,"ReadData",(int)dType,M_CONT_ERROR_MSG);
    return(FILE_READ_ERROR);
  }
}

int RDataContainer::ReadData(const char *buffer, int size, int start)
{
  //This function reads data from file "dFilename" and stores it
  //in "buffer" as follows:
  //
  //If the file format is binary, then one item of "size" "buffer",
  //starting at byte "start", is read and the function returns.
  //If a more complex data structure it to be read, it must be
  //passed with a typecast to a char pointer.
  //
  //If the file format is plain text, then a number of characters
  //equal to the length(=size) of "buffer" minus one are read, starting
  //at byte "start". The "buffer" needs to be memset, by the
  //parent functions, to a terminating '\0' character in the last 
  //character position.
  //
  //The function performs mode (dMode) checks and returns without
  //doing anything except pass an error message if the file was 
  //opened as write only.

  if(dOpen) {
    if(dMode == FM_UPDATE || dMode == FM_READ){
      if(dType == FT_BINARY || dType == FT_TEXT){
	if(dFp){ 
	  if(start >= 0){
	    if(!fseek(dFp,start,SEEK_SET)){
	      if(dType == FT_BINARY) {
		if(!fread((char*)buffer,size,1,dFp)) {
		  FlushMessages();
		  SetMessage(READ_PNTR_ERROR,"ReadData",(int)dType,M_CONT_ERROR_MSG);
		  return(FILE_READ_ERROR);
		}
		return FILE_PROCESS_OK;
	      }
	      else if(dType == FT_TEXT){
		if(!fread((char*)buffer,sizeof(char),size,dFp)){
		  FlushMessages();
		  SetMessage(READ_PNTR_ERROR,"ReadData",(int)dType,M_CONT_ERROR_MSG);
		  return(FILE_READ_ERROR);
		}
		return(FILE_PROCESS_OK);
	      }
	    }
	    else{
	      FlushMessages();
	      SetMessage(PNTR_POS_ERROR,"ReadData",(int)dType,M_CONT_ERROR_MSG);
	      return(FILE_READ_ERROR);
	    }
	  }
	  else{
	    FlushMessages();
	    SetMessage(PNTR_RUN_ERROR,"ReadData",(int)dType,M_CONT_ERROR_MSG);
	    return(FILE_READ_ERROR);
	  }
	}
	else{
	  FlushMessages();
	  SetMessage(PNTR_NULL_ERROR,"ReadData",(int)dType,M_CONT_ERROR_MSG);
	  return(FILE_READ_ERROR);
	}
      }
    }
    else
      {
	FlushMessages();
	SetMessage(WRITE_MODE_ERROR,"ReadData",(int)dType,M_CONT_ERROR_MSG);
	return FILE_READ_ERROR;
      }
    return FILE_PROCESS_OK;
  }
  FlushMessages();
  SetMessage(FILE_NOT_OPEN,"ReadData",(int)dType,M_CONT_ERROR_MSG);
  return FILE_READ_ERROR;
}

int RDataContainer::WriteData(Double_t *x, Double_t *y, Double_t *ye, int rows, const char *label)
{
  int nbytes = 0;

  if(dOpen) {
    if(dMode == FM_UPDATE || dMode == FM_WRITE){
      if(dType == FT_ROWCOLUMN){
	if(dFp){ 
	  if(!fseek(dFp,GetFileSize(),SEEK_SET)){

	    if(label){
	      nbytes += fprintf(dFp,"\n%s\n\n",label);
	    }

	    for(int i = 0; i < rows; i++){
	      nbytes += fprintf(dFp,"%+2.8e %+2.8e %+2.8e\n",x[i],y[i],ye[i]);
	    }
	    memset(dMiscbuffer2,'\0',sizeof(dMiscbuffer2));
	    memset(dMiscbuffer,'\0',sizeof(dMiscbuffer));
	    sprintf(dMiscbuffer2,"Wrote : %d bytes to file %s\n",nbytes,
		    strrchr(GetFileName(),'/') ? strrchr(GetFileName(),'/') : 
		    GetFileName());
	    sprintf(dMiscbuffer,"File size is now: %ld\n",GetFileSize());
	    strcat(dMiscbuffer2,dMiscbuffer);
	    SetMessage(dMiscbuffer2,"",(int)dType,M_CONT_LOGTXTTS);
	    return FILE_PROCESS_OK;
	  }
	  else{
	    FlushMessages();
	    SetMessage(PNTR_POS_ERROR,"WriteData",(int)dType,M_CONT_ERROR_MSG);
	    return(FILE_WRITE_ERROR);
	  }
	}
	else{
	  FlushMessages();
	  SetMessage(PNTR_NULL_ERROR,"WriteData",(int)dType,M_CONT_ERROR_MSG);
	  return(FILE_WRITE_ERROR);
	}
      }
      else if(dType == FT_MATHCAD){
	if(dFp){ 
	  if(!fseek(dFp,GetFileSize(),SEEK_SET)){

	    if(label){
	      nbytes += fprintf(dFp,"\n%s\n\n",label);
	    }

	    nbytes += fprintf(dFp,"{");
	    for(int i = 0; i < rows; i++){
	      nbytes += fprintf(dFp,"{%lf,%lf,%lf}",x[i],y[i],ye[i]);
	      if(i < rows-1) nbytes += fprintf(dFp,",\n");
	    }
	    nbytes += fprintf(dFp,"}\n");
	    memset(dMiscbuffer2,'\0',sizeof(dMiscbuffer2));
	    memset(dMiscbuffer,'\0',sizeof(dMiscbuffer));
	    sprintf(dMiscbuffer2,"Wrote : %d bytes to file %s\n",nbytes,
		    strrchr(GetFileName(),'/') ? strrchr(GetFileName(),'/') : 
		    GetFileName());
	    sprintf(dMiscbuffer,"File size is now: %ld\n",GetFileSize());
	    strcat(dMiscbuffer2,dMiscbuffer);
	    SetMessage(dMiscbuffer2,"",(int)dType,M_CONT_LOGTXTTS);
	    return FILE_PROCESS_OK;
	  }
	  else{
	    FlushMessages();
	    SetMessage(PNTR_POS_ERROR,"WriteData",(int)dType,M_CONT_ERROR_MSG);
	    return(FILE_WRITE_ERROR);
	  }
	}
	else{
	  FlushMessages();
	  SetMessage(PNTR_NULL_ERROR,"WriteData",(int)dType,M_CONT_ERROR_MSG);
	  return(FILE_WRITE_ERROR);
	}
      }
      else{
	FlushMessages();
	SetMessage(WRITE_TYPE_ERROR,"WriteData",(int)dType,M_CONT_ERROR_MSG);
	return FILE_WRITE_ERROR;
      }
    }
    else
      {
	FlushMessages();
	SetMessage(READ_MODE_ERROR,"WriteData",(int)dType,M_CONT_ERROR_MSG);
	return FILE_WRITE_ERROR;
      }
    return FILE_PROCESS_OK;
  }
  FlushMessages();
  SetMessage(FILE_NOT_OPEN,"WriteData",(int)dType,M_CONT_ERROR_MSG);
  return FILE_WRITE_ERROR;
}

int RDataContainer::WriteData(Double_t *x, Double_t *y, Double_t *xe, Double_t *ye, int rows, const char *label)
{
  int nbytes = 0;

  if(dOpen) {
    if(dMode == FM_UPDATE || dMode == FM_WRITE){
      if(dType == FT_ROWCOLUMN){
	if(dFp){ 
	  if(!fseek(dFp,GetFileSize(),SEEK_SET)){

	    if(label){
	      nbytes += fprintf(dFp,"\n%s\n\n",label);
	    }

	    for(int i = 0; i < rows; i++){
	      nbytes += fprintf(dFp,"%+2.8e %+2.8e %+2.8e %+2.8e\n",x[i],y[i],xe[i],ye[i]);
	    }
	    memset(dMiscbuffer2,'\0',sizeof(dMiscbuffer2));
	    memset(dMiscbuffer,'\0',sizeof(dMiscbuffer));
	    sprintf(dMiscbuffer2,"Wrote : %d bytes to file %s\n",nbytes,
		    strrchr(GetFileName(),'/') ? strrchr(GetFileName(),'/') : 
		    GetFileName());
	    sprintf(dMiscbuffer,"File size is now: %ld\n",GetFileSize());
	    strcat(dMiscbuffer2,dMiscbuffer);
	    SetMessage(dMiscbuffer2,"",(int)dType,M_CONT_LOGTXTTS);
	    return FILE_PROCESS_OK;
	  }
	  else{
	    FlushMessages();
	    SetMessage(PNTR_POS_ERROR,"WriteData",(int)dType,M_CONT_ERROR_MSG);
	    return(FILE_WRITE_ERROR);
	  }
	}
	else{
	  FlushMessages();
	  SetMessage(PNTR_NULL_ERROR,"WriteData",(int)dType,M_CONT_ERROR_MSG);
	  return(FILE_WRITE_ERROR);
	}
      }
      else if(dType == FT_MATHCAD){
	if(dFp){ 
	  if(!fseek(dFp,GetFileSize(),SEEK_SET)){

	    if(label){
	      nbytes += fprintf(dFp,"\n%s\n\n",label);
	    }

	    nbytes += fprintf(dFp,"{");
	    for(int i = 0; i < rows; i++){
	      nbytes += fprintf(dFp,"{%lf,%lf,%lf,%lf}",x[i],y[i],xe[i],ye[i]);
	      if(i < rows-1) nbytes += fprintf(dFp,",\n");
	    }
	    nbytes += fprintf(dFp,"}\n");
	    memset(dMiscbuffer2,'\0',sizeof(dMiscbuffer2));
	    memset(dMiscbuffer,'\0',sizeof(dMiscbuffer));
	    sprintf(dMiscbuffer2,"Wrote : %d bytes to file %s\n",nbytes,
		    strrchr(GetFileName(),'/') ? strrchr(GetFileName(),'/') : 
		    GetFileName());
	    sprintf(dMiscbuffer,"File size is now: %ld\n",GetFileSize());
	    strcat(dMiscbuffer2,dMiscbuffer);
	    SetMessage(dMiscbuffer2,"",(int)dType,M_CONT_LOGTXTTS);
	    return FILE_PROCESS_OK;
	  }
	  else{
	    FlushMessages();
	    SetMessage(PNTR_POS_ERROR,"WriteData",(int)dType,M_CONT_ERROR_MSG);
	    return(FILE_WRITE_ERROR);
	  }
	}
	else{
	  FlushMessages();
	  SetMessage(PNTR_NULL_ERROR,"WriteData",(int)dType,M_CONT_ERROR_MSG);
	  return(FILE_WRITE_ERROR);
	}
      }
      else{
	FlushMessages();
	SetMessage(WRITE_TYPE_ERROR,"WriteData",(int)dType,M_CONT_ERROR_MSG);
	return FILE_WRITE_ERROR;
      }
    }
    else
      {
	FlushMessages();
	SetMessage(READ_MODE_ERROR,"WriteData",(int)dType,M_CONT_ERROR_MSG);
	return FILE_WRITE_ERROR;
      }
    return FILE_PROCESS_OK;
  }
  FlushMessages();
  SetMessage(FILE_NOT_OPEN,"WriteData",(int)dType,M_CONT_ERROR_MSG);
  return FILE_WRITE_ERROR;
}

int RDataContainer::WriteData(Double_t *x, Double_t *y, int rows, const char * label)
{
  int nbytes = 0;

  if(dOpen) {
    if(dMode == FM_UPDATE || dMode == FM_WRITE){
      if(dType == FT_ROWCOLUMN){
	if(dFp){ 
	  if(!fseek(dFp,GetFileSize(),SEEK_SET)){

	    if(label){
	      nbytes += fprintf(dFp,"\n%s\n\n",label);
	    }


	    for(int i = 0; i < rows; i++){
	      nbytes += fprintf(dFp,"%+2.8e %+2.8e\n",x[i],y[i]);
	    }
	    memset(dMiscbuffer2,'\0',sizeof(dMiscbuffer2));
	    memset(dMiscbuffer,'\0',sizeof(dMiscbuffer));
	    sprintf(dMiscbuffer2,"Wrote : %d bytes to file %s\n",nbytes,
		    strrchr(GetFileName(),'/') ? strrchr(GetFileName(),'/') : 
		    GetFileName());
	    sprintf(dMiscbuffer,"File size is now: %ld\n",GetFileSize());
	    strcat(dMiscbuffer2,dMiscbuffer);
	    SetMessage(dMiscbuffer2,"",(int)dType,M_CONT_LOGTXTTS);
	    return FILE_PROCESS_OK;
	  }
	  else{
	    FlushMessages();
	    SetMessage(PNTR_POS_ERROR,"WriteData",(int)dType,M_CONT_ERROR_MSG);
	    return(FILE_WRITE_ERROR);
	  }
	}
	else{
	  FlushMessages();
	  SetMessage(PNTR_NULL_ERROR,"WriteData",(int)dType,M_CONT_ERROR_MSG);
	  return(FILE_WRITE_ERROR);
	}
      }
      else if(dType == FT_MATHCAD){
	if(dFp){ 
	  if(!fseek(dFp,GetFileSize(),SEEK_SET)){

	    if(label){
	      nbytes += fprintf(dFp,"\n%s\n\n",label);
	    }

	    nbytes += fprintf(dFp,"{");
	    for(int i = 0; i < rows; i++){
	      nbytes += fprintf(dFp,"{%lf,%lf}",x[i],y[i]);
	      if(i < rows-1) nbytes += fprintf(dFp,",\n");
	    }
	    nbytes += fprintf(dFp,"}\n");
	    memset(dMiscbuffer2,'\0',sizeof(dMiscbuffer2));
	    memset(dMiscbuffer,'\0',sizeof(dMiscbuffer));
	    sprintf(dMiscbuffer2,"Wrote : %d bytes to file %s\n",nbytes,
		    strrchr(GetFileName(),'/') ? strrchr(GetFileName(),'/') : 
		    GetFileName());
	    sprintf(dMiscbuffer,"File size is now: %ld\n",GetFileSize());
	    strcat(dMiscbuffer2,dMiscbuffer);
	    SetMessage(dMiscbuffer2,"",(int)dType,M_CONT_LOGTXTTS);
	    return FILE_PROCESS_OK;
	  }
	  else{
	    FlushMessages();
	    SetMessage(PNTR_POS_ERROR,"WriteData",(int)dType,M_CONT_ERROR_MSG);
	    return(FILE_WRITE_ERROR);
	  }
	}
	else{
	  FlushMessages();
	  SetMessage(PNTR_NULL_ERROR,"WriteData",(int)dType,M_CONT_ERROR_MSG);
	  return(FILE_WRITE_ERROR);
	}
      }
      else{
	FlushMessages();
	SetMessage(WRITE_TYPE_ERROR,"WriteData",(int)dType,M_CONT_ERROR_MSG);
	return FILE_WRITE_ERROR;
      }
    }
    else
      {
	FlushMessages();
	SetMessage(READ_MODE_ERROR,"WriteData",(int)dType,M_CONT_ERROR_MSG);
	return FILE_WRITE_ERROR;
      }
    return FILE_PROCESS_OK;
  }
  FlushMessages();
  SetMessage(FILE_NOT_OPEN,"WriteData",(int)dType,M_CONT_ERROR_MSG);
  return FILE_WRITE_ERROR;
}

int RDataContainer::WriteData(Double_t *data, int rows, int cols, const char * label)
{
  int nbytes = 0;

  if(dOpen) {
    if(dMode == FM_UPDATE || dMode == FM_WRITE){
      if(dType == FT_ROWCOLUMN){
	if(dFp){ 
	  if(!fseek(dFp,GetFileSize(),SEEK_SET)){

	    if(label){
	      nbytes += fprintf(dFp,"\n%s\n\n",label);
	    }

	    for(int r = 0; r < rows; r++){
	      for(int c = 0; c < cols; c++){
		nbytes += fprintf(dFp,"%+2.8e",data[(int)(c*rows + r)]);
		if(c < cols-1) nbytes += fprintf(dFp," ");
	      }  
	      nbytes += fprintf(dFp,"\n");
	    }
	    memset(dMiscbuffer2,'\0',sizeof(dMiscbuffer2));
	    memset(dMiscbuffer,'\0',sizeof(dMiscbuffer));
	    sprintf(dMiscbuffer2,"Wrote : %d bytes to file %s\n",nbytes,
		    strrchr(GetFileName(),'/') ? strrchr(GetFileName(),'/') : 
		    GetFileName());
	    sprintf(dMiscbuffer,"File size is now: %ld\n",GetFileSize());
	    strcat(dMiscbuffer2,dMiscbuffer);
	    SetMessage(dMiscbuffer2,"",(int)dType,M_CONT_LOGTXTTS);
	    return FILE_PROCESS_OK;
	  }
	  else{
	    FlushMessages();
	    SetMessage(PNTR_POS_ERROR,"WriteData",(int)dType,M_CONT_ERROR_MSG);
	    return(FILE_WRITE_ERROR);
	  }
	}
	else{
	  FlushMessages();
	  SetMessage(PNTR_NULL_ERROR,"WriteData",(int)dType,M_CONT_ERROR_MSG);
	  return(FILE_WRITE_ERROR);
	}
      }
      else{
	FlushMessages();
	SetMessage(WRITE_TYPE_ERROR,"WriteData",(int)dType,M_CONT_ERROR_MSG);
	return FILE_WRITE_ERROR;
      }
    }
    else
      {
	FlushMessages();
	SetMessage(READ_MODE_ERROR,"WriteData",(int)dType,M_CONT_ERROR_MSG);
	return FILE_WRITE_ERROR;
      }
    return FILE_PROCESS_OK;
  }
  FlushMessages();
  SetMessage(FILE_NOT_OPEN,"WriteData",(int)dType,M_CONT_ERROR_MSG);
  return FILE_WRITE_ERROR;
}

int RDataContainer::WriteData(const char* buffer ,int size)
{
  if(dOpen) {
    if(dMode == FM_WRITE || dMode == FM_UPDATE){
      if(dType == FT_BINARY || dType == FT_TEXT){
	if(dFp){ 
	  if(/*start >= 0*/1){
	    if(!fseek(dFp,GetFileSize(),SEEK_SET)){
	      if(dType == FT_BINARY) {
		if(dMode == FM_WRITE){
		  if(fwrite(buffer,size,1,dFp)!=1) {
		    FlushMessages();
		    SetMessage(WRITE_PNTR_ERROR,"WriteData",(int)dType,
			       M_CONT_ERROR_MSG);
		    return(FILE_WRITE_ERROR);
		  }
		  memset(dMiscbuffer2,'\0',sizeof(dMiscbuffer2));
		  memset(dMiscbuffer,'\0',sizeof(dMiscbuffer));
		  sprintf(dMiscbuffer2,"Wrote : %d bytes to file %s\n",size,
			  strrchr(GetFileName(),'/') ? strrchr(GetFileName(),'/') : 
			  GetFileName());
		  sprintf(dMiscbuffer,"File size is now: %ld\n",GetFileSize());
		  strcat(dMiscbuffer2,dMiscbuffer);
		  SetMessage(dMiscbuffer2,"",(int)dType,M_CONT_LOGTXTTS);
		  return FILE_PROCESS_OK;
		}
	      }
	      else if(dType == FT_TEXT){
		if(fwrite(buffer,sizeof(char),size,dFp) != 
		   (unsigned int)(size)){
		  FlushMessages();
		  SetMessage(WRITE_PNTR_ERROR,"WriteData",(int)dType,
			     M_CONT_ERROR_MSG);
		  return(FILE_WRITE_ERROR);
		}
		memset(dMiscbuffer2,'\0',sizeof(dMiscbuffer2));
		memset(dMiscbuffer,'\0',sizeof(dMiscbuffer));
		sprintf(dMiscbuffer2,"Wrote : %d bytes to file %s\n",size,
			strrchr(GetFileName(),'/') ? strrchr(GetFileName(),'/') : 
			GetFileName());
		sprintf(dMiscbuffer,"File size is now: %ld\n",GetFileSize());
		strcat(dMiscbuffer2,dMiscbuffer);
		SetMessage(dMiscbuffer2,"",(int)dType,M_CONT_LOGTXTTS);
		return(FILE_PROCESS_OK);
	      }
	    }
	    else{
	      FlushMessages();
	      SetMessage(PNTR_POS_ERROR,"WriteData",(int)dType,M_CONT_ERROR_MSG);
	      return(FILE_WRITE_ERROR);
	    }
	  }
	  else{
	    FlushMessages();
	    SetMessage(PNTR_RUN_ERROR,"WriteData",(int)dType,M_CONT_ERROR_MSG);
	    return(FILE_WRITE_ERROR);
	  }
	}
	else{
	  FlushMessages();
	  SetMessage(PNTR_NULL_ERROR,"WriteData",(int)dType,M_CONT_ERROR_MSG);
	  return(FILE_WRITE_ERROR);
	}
      }
    }
    else
      {
	FlushMessages();
	SetMessage(READ_MODE_ERROR,"WriteData",(int)dType,M_CONT_ERROR_MSG);
	return FILE_WRITE_ERROR;
      }
  }
  FlushMessages();
  SetMessage(FILE_NOT_OPEN,"WriteData",(int)dType,M_CONT_ERROR_MSG);
  return FILE_WRITE_ERROR;

}

int RDataContainer::WriteData(TObject *obj)
{
  int nbytes = 0;

  if(dMode == FM_UPDATE || dMode == FM_WRITE){  
    if(dType == FT_ROOT && obj != NULL){

      if(fRfile != NULL){
	if(fRfile->IsOpen()){	  
	  nbytes = obj->Write("",TObject::kOverwrite);

	  if(nbytes){
	    memset(dMiscbuffer2,'\0',sizeof(dMiscbuffer2));
	    memset(dMiscbuffer,'\0',sizeof(dMiscbuffer));
	    sprintf(dMiscbuffer2,"Wrote : %d bytes to file %s\n",nbytes,
		    strrchr(GetFileName(),'/') ? strrchr(GetFileName(),'/') : 
		    GetFileName());
	    sprintf(dMiscbuffer,"File size is now: %ld\n",GetFileSize());
	    strcat(dMiscbuffer2,dMiscbuffer);
	    SetMessage(dMiscbuffer2,"",(int)dType,M_CONT_LOGTXTTS);
	    return FILE_PROCESS_OK;
	  }
	  else{
	    FlushMessages();
	    SetMessage(ROOT_WRITE_ERROR,"WriteData",(int)dType,M_CONT_ERROR_MSG);
	    return FILE_WRITE_ERROR;
	  }
	}
	else{
	  FlushMessages();
	  SetMessage(ROOT_OPEN_ERROR,"WriteData",(int)dType,M_CONT_ERROR_MSG);
	  return FILE_WRITE_ERROR;
	}
      }
      else{
	FlushMessages();
	SetMessage(PNTR_NULL_ERROR,"WriteData",(int)dType,M_CONT_ERROR_MSG);
	return FILE_WRITE_ERROR;
      }
    }
    else {
      FlushMessages();
      SetMessage(WRITE_TYPE_ERROR,"WriteData",(int)dType,M_CONT_ERROR_MSG);
      return FILE_WRITE_ERROR;
    }
  }

  FlushMessages();
  SetMessage(READ_MODE_ERROR,"WriteData",(int)dType,M_CONT_ERROR_MSG);
  return FILE_WRITE_ERROR;
}


void RDataContainer::Close(Bool_t kWrite)
{
  sprintf(dMiscbuffer2,"Closing: %s\n",
	  strrchr(GetFileName(),'/') ? strrchr(GetFileName(),'/') : 
	  GetFileName());

  SetMessage(dMiscbuffer2,"",(int)dType,M_CONT_LOGTXTTS);

  CloseFile(kWrite);
  delete this;
}
