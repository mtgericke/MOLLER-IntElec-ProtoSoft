/////////////////////////////////////////////////////////////////////////////////////
//Root Based Data Visualization and Analysis Utility Library
//
//Version 000
//
//Definitions file, containing all common constants
/////////////////////////////////////////////////////////////////////////////////////


#ifndef RDATACONTAINERDEF_H
#define RDATACONTAINERDEF_H

//#define RDC_DEBUG

//Definitions for major constants used throughout:

#ifndef kTrue
#define kTrue                 1
#endif
#ifndef kFalse
#define kFalse                0
#endif

//#ifndef Pi
//#define Pi                    3.14159265359
//#endif

#ifndef MSG_QUEUE_MAX
#define MSG_QUEUE_MAX         10           //Maximum entries per message
#endif                                     //queue
#ifndef MSG_SIZE_MAX
#define MSG_SIZE_MAX          1000         //Maximum size of each message
#endif
#ifndef NAME_STR_MAX
#define NAME_STR_MAX          200          //Maximum file name length
#endif


#define FILE_READ_ERROR       1001
#define FILE_WRITE_ERROR      1002
#define FILE_OPEN_ERROR       1003
#define FILE_PROCESS_OK       1004

#define DATCONT_CLOSED_MSG  "SIGNAL: Object closed"
#define FILE_OPEN_ERROR_MSG "ERROR: Could not open file\n"
#define FILE_OPEN_OK_MSG    "ERROR: File is open\n"
#define WRITE_MODE_ERROR    "ERROR: File was opened write only\n"
#define READ_MODE_ERROR     "ERROR: File was opened read only\n"
#define READ_PNTR_ERROR     "ERROR: Could not read from file (fread failed)\n"
#define WRITE_PNTR_ERROR    "ERROR: Could not write to file (fwrite failed)\n"
#define PNTR_POS_ERROR      "ERROR: Can't find position in file(fseek failed)\n"
#define PNTR_NULL_ERROR     "ERROR: File pointer must be non NULL\n"
#define PNTR_RUN_ERROR      "ERROR: Pointer start position must be >= 0\n"
#define FILE_CONT_OCCUPIED  "ERROR: Trying to open file in occupied container\n"
#define FILE_UNDET_TYPE     "ERROR: Can't open file with unknown data type\n"
#define FILE_NOT_OPEN       "ERROR: Can't read data.Container has no open file\n"
#define FILE_RCPOS_ERROR    "ERROR: Row,Column positions start at (1,1)\n"
#define FILE_RCOFLOW_ERROR  "ERROR: Row,Column position overflow\n"
#define READ_TYPE_ERROR     "ERROR: Wrong read function called for this file type\n"
#define WRITE_TYPE_ERROR    "ERROR: Wrong write function called for this file type\n"
#define ROOT_OPEN_ERROR     "ERROR: Root file does not seem to be open\n"
#define ROOT_OBJCRT_ERROR   "ERROR: Could not create object in copy operation\n"
#define ROOT_WRITE_ERROR    "ERROR: Could not write object to this ROOT file\n"
#define UPDT_PNTR_ERROR     "ERROR: Can not insert data at negative pointer pos\n"
#define ROOT_OBJFIND_ERROR  "ERROR: Can not find root object with the given name\n"
#define PARM_NULL_ERROR     "ERROR: Parameters passed must be non NULL\n"
#define ROOT_DIM_ERROR      "ERROR: Wrong dimensionalty for this operation\n"

enum ERFileTypes {
  FT_TEXT       = 300,                     //File is ASCII text
  FT_BINARY,                               //FIle is binary
  FT_ROWCOLUMN,                            //File is ASCII text arranged
                                           //in rows and columns
  FT_ROOT,                                 //File is a ROOT object file
                                           //(*.root)
  FT_MATHCAD,
  FT_DBASE                                 //Arbitrary database file type
};

enum ERFileModes {
  FM_READ       = 500,                     //File is opened for reading
  FM_WRITE,                                //FIle is opened for writing
  FM_UPDATE                                //File is opened for updating
};

enum ERRDCMessageTypes {
  kC_CONT_OBJECT    = 700, 
  kCM_CONT_MSG,
  M_CONT_CLOSED,
  M_CONT_LOGTXTTS,
  M_CONT_ERROR_MSG
};

#endif
