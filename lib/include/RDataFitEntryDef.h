/////////////////////////////////////////////////////////////////////////////////////
//Root Based Data Visualization and Analysis Utility Library
//
//Version 000
//
//Definitions file, containing all common constants
/////////////////////////////////////////////////////////////////////////////////////


#ifndef RDATAFITENTRYDEF_H
#define RDATAFITENTRYDEF_H

#define RDFE_DEBUG

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


#define FITENTRY_ERROR        1009
#define FITENTRY_OK           1010


#define FITENTRY_CLOSED_MSG   "SIGNAL: Object closed"
//#define FILE_OPEN_ERROR_MSG "ERROR: Could not open file\n"


enum ERRFEMessageTypes {
  kC_FITENTRY_OBJECT    = 1500, 
  kCM_FITENTRY_MSG,
  M_FITENTRY_CLOSED,
  M_FITENTRY_LOGTXTTS,
  M_FITENTRY_ERROR_MSG,

  M_FITENTRY_SET,
  M_FITENTRY_CANCEL,
  M_FITENTRY_FULLRANGE,
  M_FITENTRY_SELFUNC,
  M_FITENTRY_ROOTR
};

#endif
