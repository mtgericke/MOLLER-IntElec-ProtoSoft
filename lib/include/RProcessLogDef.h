/////////////////////////////////////////////////////////////////////////////////////
//Root Based Data Visualization and Analysis Utility Library
//
//Version 000
//
//Definitions file, containing all common constants
/////////////////////////////////////////////////////////////////////////////////////


#ifndef RPROCESSLOGDEF_H
#define RPROCESSLOGDEF_H

#define RPL_DEBUG

//Definitions for major constants used throughout:

#ifndef kTrue
#define kTrue                 1
#endif
#ifndef kFalse
#define kFalse                0
#endif

#ifndef MSG_QUEUE_MAX
#define MSG_QUEUE_MAX         10           //Maximum entries per message
#endif                                     //queue
#ifndef MSG_SIZE_MAX
#define MSG_SIZE_MAX          1000         //Maximum size of each message
#endif
#ifndef NAME_STR_MAX
#define NAME_STR_MAX          200          //Maximum file name length
#endif


#define LOG_PROCESS_ERROR     1011
#define LOG_PROCESS_OK        1012


#define LOGFILE_CLOSED_MSG  "SIGNAL: Object closed"
#define LOGFILE_OPEN_ERROR  "ERROR: Could not open file\n"
#define EDITOR_NULL_ERROR   "ERROR: Editor object doesn't exist (fEdit = NULL)\n"
#define TEXT_NULL_ERROR     "ERROR: Text container doesn't exist (fText = NULL)\n"
#define BUF_MEM_ERROR       "ERROR: Could not obtain enough memory for buffer\n"
#define BUF_OVFLOW_ERROR    "ERROR: Buffer is too small for data size\n"
#define FILE_CLOSED_ERROR   "ERROR: There is no text buffer open\n"


enum ERPLMessageTypes {
  kC_LOGFILE_OBJECT    = 1550, 
  kCM_LOGFILE_MSG,
  M_LOGFILE_CLOSED,
  M_LOGFILE_LOGTXTTS,
  M_LOGFILE_ERROR_MSG,

  M_LOG_FILE_OPEN,
  M_LOG_FILE_CLOSE, 
  M_LOG_FILE_SAVEAS,   
  M_LOG_FILE_SAVE,   
  M_LOG_FILE_PRINT,
  M_LOG_FILE_PRINTSETUP,
  M_LOG_FILE_EXIT,
};

#endif
