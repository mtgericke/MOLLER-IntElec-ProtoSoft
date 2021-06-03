/////////////////////////////////////////////////////////////////////////////////////
//Root Based Data Visualization and Analysis Utility Library
//
//Version 000
//
//Definitions file, containing all common constants
/////////////////////////////////////////////////////////////////////////////////////


#ifndef RDATAFITDEF_H
#define RDATAFITDEF_H

#define RDF_DEBUG

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

#define DATA_FIT_ERROR        1007
#define FIT_PROCESS_OK        1008

#define FITFUNC_CLOSED_MSG   "SIGNAL: Object closed"
//#define PARAM_NULL_ERROR      "ERROR: Data passed for plotting must be non null\n"
#define DIMENSION_ERROR       "ERROR: Dimension must be 1,2 or 3\n"
#define PARM_CNT_ERROR        "ERROR: Wrong number of paramters passed (1..10)\n"
#define RANGE_ERROR           "ERROR: Range must be larger than # of deg. of freed\n"
//#define READ_PNTR_ERROR     "ERROR: Could not read from file (fread failed)\n"
//#define WRITE_PNTR_ERROR    "ERROR: Could not write to file (fwrite failed)\n"
//#define PNTR_POS_ERROR      "ERROR: Can't find position in file(fseek failed)\n"
//#define ROOTOBJ_NULL_ERROR    "ERROR: Object pointer must be non NULL\n"
//#define ROOTOBJ_CRFAIL_ERROR  "ERROR: Could not create object on the heap\n"

enum ERFitLogOutput {
  FL_NORMAL          = 1400,
  FL_VERBOSE,
  FL_SUPERVERBOSE,
  FL_BARF
};

enum ERRDFMessageTypes {
  kC_DTFIT_OBJECT    = 1450, 
  kCM_DTFIT_MSG,
  M_DTFIT_CLOSED,
  M_DTFIT_LOGTXTTS,
  M_DTFIT_ERROR_MSG
};

#endif
