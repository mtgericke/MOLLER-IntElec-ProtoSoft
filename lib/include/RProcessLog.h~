/////////////////////////////////////////////////////////////////////////////////////
//Root Based Data Visualization and Analysis Utility Library
//
//Version 000
//
//RProcessLog header file, declaring basic methods to store and display a log
//of the current process.
/////////////////////////////////////////////////////////////////////////////////////


#ifndef RPROCESSLOG_H
#define RPROCESSLOG_H

#include <TSystem.h>
#include <TGMsgBox.h>
#include <TGFileDialog.h>
#include <TGFrame.h>
#include <TGMenu.h>
#include "RProcessLogDef.h"
#include <TGTextEntry.h>
#include <TGTextEdit.h>
#include <time.h>

class RProcessLog : public TGTransientFrame {

 private:

  Int_t                dMsgcnt;
  Bool_t               dOpenFlag;
  char                 dObjName[NAME_STR_MAX];
  char                 dMainName[NAME_STR_MAX]; 
  char                 dReceiverName[NAME_STR_MAX]; 
  char                 dMiscbuffer[MSG_SIZE_MAX]; //For random use
  char                 dMiscbuffer2[MSG_SIZE_MAX];
  char                 dLogtitle[NAME_STR_MAX];
  char                 dLogname[NAME_STR_MAX];
  char                 dMessage[MSG_QUEUE_MAX][MSG_SIZE_MAX]; //Message in queue
  time_t              *cutime;
  tm                  *ltime;


  TGPopupMenu         *fMenuLogFile;
  TGMenuBar           *fMenuBar;
  TGLayoutHints       *fEditLayout; 
  TGLayoutHints       *fMenuBarItemLayout;
  TGLayoutHints       *fMenuBarLayout;

  TGWindow            *fMrec;
  TGText              *fText;
  TGTextEdit          *fEdit;

  Bool_t               SetMessage(const char *msg, 
				  const char *func,
				  Int_t TS, 
				  Int_t MESSAGETYPE); //Add message to queue
  void                 FlushMessages();//Get rid of all messages in queue
  
 public:
  
  RProcessLog(const TGWindow *p, const TGWindow *main,
	      char *objName, char *mainname,
	      char *logname, char *logtitle,
	      UInt_t w, UInt_t h,
	      UInt_t options = kVerticalFrame);
  virtual ~RProcessLog();

  TGTextEdit          *GetTextEdit() {return fEdit;};
  char                *GetMessage();
  char                *GetLogTitle(){return dLogtitle;};
  Long_t               GetLogSize();
  ULong_t              GetLineCnt();
  Int_t                GetLog(char *buffer, Long_t size);
  void                 PrintLogLatex();
  char                *AddLatexFigure(char *buffer, char *filename);
  Int_t                Append(char*, Bool_t);

  virtual void         CloseWindow();
  virtual Bool_t       ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2);
  void                 IsClosing(char *);
  void                 ConnectWith(const TGWindow *, char *); 
  void                 CloseConnection(); 
  virtual void         ToggleOpenFlag();
  Bool_t               IsFileOpen(){return dOpenFlag;};
  ClassDef(RProcessLog,0);
};

#endif
