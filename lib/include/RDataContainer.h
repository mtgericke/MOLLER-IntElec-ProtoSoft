/////////////////////////////////////////////////////////////////////////////////////
//Root Based Data Visualization and Analysis Utility Library
//
//Version 000
//
//DataContainer header file, containing methods to open and read data
//Stores a pointer to a currently open file
//All file formats are specified here.
/////////////////////////////////////////////////////////////////////////////////////


#ifndef RDATACONTAINER_H
#define RDATACONTAINER_H


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <TSystem.h>
#include <TString.h>
#include <TQObject.h>
#include <TGWidget.h>
#include <TFile.h>
#include <TMapFile.h>
#include <TTree.h>
#include <TKey.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TH3D.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TH3F.h>
#include <TH1S.h>
#include <TH2S.h>
#include <TH3S.h>
#include <TBrowser.h>
#include <TRootBrowser.h>
#include "RDataContainerDef.h"

class RDataContainer : public TQObject {

 private:

  Bool_t              dBrowserFlag;
  char                dObjName[NAME_STR_MAX];  
  char                dMainName[NAME_STR_MAX]; 
  char                dReceiverName[NAME_STR_MAX];
  Bool_t              dOpen;  //Is the file open or closed (true or false)
  ERFileTypes         dType;  //Type of data in the file (see RDataContainerDef.h)
  ERFileModes         dMode;  //Mode of operation on file (see RDataContainerDef.h)
  int                 dColumns;       //For Text files arranged in columns and
  int                 dRows;          //rows
  int                 dMsgcnt;        //Number of messages in queue
  FILE               *dFp;            //Regular file pointer (any file)
  char                dDataname[NAME_STR_MAX]; //Name by which you refer to the data
  char                dFilename[NAME_STR_MAX]; //The actual filename
  char                dMessage[MSG_QUEUE_MAX][MSG_SIZE_MAX]; //Message in queue
  char                dMiscbuffer[NAME_STR_MAX]; //For random use
  char                dMiscbuffer2[NAME_STR_MAX];
  long int            dDatasize;      //Size (in bytes) of the actual data in file
  long int            dFilesize;      //Overall file size (in bytes), incl. header
  long int            dCurrange;      //The current data range read from file

  TFile              *fRfile;         //Root object file 
  TBrowser           *fRootBrowser;
  TRootBrowser       *fImp;
  TGWindow           *fMrec;
  TObject            *dSelectedObj;

 protected:

  void                CloseFile(Bool_t kWrite = kTrue);    //CloseFile() is private to 
                                      //make sure that only destroying 
                                      //the RDataContainer itself will control the 
                                      //existence of a file pointer

  Bool_t              SetMessage(const char *msg, 
				 const char *func,
				 int TS, 
				 int MESSAGETYPE);
                                      //Add message to queue
  void                FlushMessages();//Get rid of all messages in queue
  int                 SetRowColumn(int,int);

 public:

  TMapFile           *fMapFile;      //Root Memory mapped object file 
  RDataContainer(const TGWindow *p, const TGWindow *main, 
		 const char *objName, const char *ownername, const char *dataname, 
		 ERFileModes mode, ERFileTypes type, 
		 UInt_t w=0, UInt_t h=0,
		 UInt_t options = kVerticalFrame);
/*   RDataContainer(const RDataContainer &copy); */
  void operator=(const RDataContainer &copy);
  ~RDataContainer();

  virtual int   OpenFile(const char *);  
  Int_t         OpenMapFile(const char* file = NULL);
  TObject      *ReadData(const char *);
  TObject      *ReadTree(const char *);
  int           ReadRow(const char *, int);
  int           ReadData(Double_t*,int);
  int           ReadData(Double_t*,int,int);
  int           ReadData(Double_t*,int,int,int);
  int           ReadData(Double_t*,Double_t*,int,int,int,int);
  int           ReadData(const char*,Double_t *x, Double_t *w, 
			 Double_t *y=NULL, Double_t *z=NULL);
  int           ReadData(const char*,int,int);
  int           ReadData(const char*,int);
  int           WriteData(TObject*);
  int           WriteData(Double_t *,int,int, const char *label = NULL);
  int           WriteData(Double_t *,Double_t *, int,  const  char *label = NULL);
  int           WriteData(Double_t *,Double_t *, Double_t *,int, const char *label = NULL);
  int           WriteData(Double_t *,Double_t *, Double_t *,Double_t *,int, const char *label = NULL);
  int           WriteData(const char*,int);
  char         *GetDataName()      {return dDataname;};
  char         *GetFileName()      {return dFilename;};
  long int      GetDataSize()      {return dDatasize;};
  void          SetDataSize(long int sz){dDatasize=sz;};
  long int      GetCurrentPos();
  long int      GetFileSize();
  int           GetRowLength(int row);
  int           GetNumOfColumns();
  int           GetNumOfRows();
  ERFileTypes   GetFileType()      {return dType;};
  ERFileModes   GetFileMode()      {return dMode;};
  int           GetNumOfRootObjects();
  int           GetListOfRootObjects(TString str[]);
  char         *GetObjectType(const char *);
  Bool_t        IsFileOpen() {return dOpen;};
  Bool_t        IsObjectType(const char*, const char*);
  void          SetBrowserFlag(Bool_t flag = kTrue){dBrowserFlag = flag;};
  int           SeekPosition(int);
  char         *GetMessage();      //Get and remove message from queue
  void          Close(Bool_t kWrite = kTrue); //Close this object and associated file(s)
  void          BrowserClosed();   //Slot to receive "Browser closed" signal
  virtual void  IsClosing(char *); //Emit signal that this object is closing
  void          OnBrowserObject(TObject*);
  void          ConnectWith(const TGWindow *, const char *); 
  void          CloseConnection(); 
  void          OpenBrowser();
  void          CloseBrowser();
  void          MakeCurrent();
  void          SendMessageSignal(char*);
  Int_t         GetHighestKey(char *name, Int_t mkey);
  Int_t         GetNumberOfKeys();
  void          DeleteObjectsWithKey(char *mkey); 
  Bool_t        cd();
  Bool_t        cd(char *dir);  
  Bool_t        mkdir(char *dir);
  TObject* GetObjFromFile(const Char_t* name);//return an object from the root file
  TObject* GetObjFromMapFile(const Char_t* name);//return an object from the map file
  TObject* GetFromMapFile(const Char_t* name, TObject* retObj = 0) {return fMapFile->Get(name, retObj);};

};

#endif
