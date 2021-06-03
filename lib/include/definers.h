/////////////////////////////////////////////////////////////////////////////////////
//Root Based Data Visualization and Analysis Utility Library
//
//Version 000
//
//Definitions file, containing all common constants
/////////////////////////////////////////////////////////////////////////////////////


#ifndef DEFINERS_H
#define DEFINERS_H

//Definitions for major constants used throughout:

#define kTrue                             1
#define kFalse                            0

//#ifndef Pi
//#define Pi                                3.14159265359
//#endif

#define MSG_QUEUE_MAX                     10           //Maximum entries per message
                                                       //queue
#define MSG_SIZE_MAX                      1000         //Maximum size of each message
#define NAME_STR_MAX                      200          //Maximum file name length

#define FILE_READ_ERROR                   1001
#define FILE_OPEN_ERROR                   1002
#define PROCESS_OK                        1003

#define WRITE_MODE_ERROR                  "File was opened write only\n"
#define READ_MODE_ERROR                   "File was opened read only\n"

enum ERFileTypes {
  FT_TEXT       = 300,                                 //File is ASCII text
  FT_BINARY,                                           //FIle is binary
  FT_ROWCOLUMN,                                        //File is ASCII text arranged
                                                       //in rows and columns
  FT_ROOT,                                             //File is a ROOT object file
                                                       //(*.root)
};

enum ERFileModes {
  FM_READ       = 500,                                 //File is opened for reading
  FM_WRITE,                                            //FIle is opened for writing
  FM_UPDATE,                                           //File is opened for updating
};

enum ERMessageTypes {
  kC_CONT_OBJECT    = 700, 
  kCM_CONT_MSG,
  M_CONT_CLOSED,
  M_CONT_LOGTXTTS,
};

#endif
