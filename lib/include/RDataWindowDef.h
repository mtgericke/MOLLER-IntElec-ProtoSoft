/////////////////////////////////////////////////////////////////////////////////////
//Root Based Data Visualization and Analysis Utility Library
//
//Version 000
//
//Definitions file, containing all common constants
/////////////////////////////////////////////////////////////////////////////////////


#ifndef RDATAWINDOWDEF_H
#define RDATAWINDOWDEF_H

#define RDW_DEBUG

//Definitions for major constants used throughout:

#ifndef kTrue
#define kTrue                 1
#endif
#ifndef kFalse
#define kFalse                0
#endif

#define DATA_PLOT_ERROR       1005
#define PLOT_PROCESS_OK       1006
#define ADD_PLOT_OK            800
#define ADD_PLOT_FAIL          801
#define PROCESS_FAILED         802
#define PROCESS_OK             803

#ifndef MSG_QUEUE_MAX
#define MSG_QUEUE_MAX         10           //Maximum entries per message
#endif                                     //queue
#ifndef MSG_SIZE_MAX
#define MSG_SIZE_MAX          1000         //Maximum size of each message
#endif
#ifndef NAME_STR_MAX
#define NAME_STR_MAX          200          //Maximum file name length
#endif


#define PLOTWIND_CLOSED_MSG   "SIGNAL: Object closed"
#define PARAM_NULL_ERROR      "ERROR: Data passed for plotting must be non null\n"
#define PLOT_TYPE_ERROR       "ERROR: Wrong plot type for this drawing routine\n"
#define PLOT_ADD_ERROR        "ERROR: Can't add more plots of this type to canvas\n"
//#define READ_MODE_ERROR     "ERROR: File was opened read only\n"
//#define READ_PNTR_ERROR     "ERROR: Could not read from file (fread failed)\n"
//#define WRITE_PNTR_ERROR    "ERROR: Could not write to file (fwrite failed)\n"
//#define PNTR_POS_ERROR      "ERROR: Can't find position in file(fseek failed)\n"
#define ROOTOBJ_NULL_ERROR    "ERROR: Object pointer must be non NULL\n"
#define ROOTOBJ_CRFAIL_ERROR  "ERROR: Could not create object on the heap\n"



enum ERPlotTypes {
  PT_HISTO_1D       = 900,     //Make a 1D histogram from the data
  PT_HISTO_2D,                 //Make a 2D histogram from the data
  PT_HISTO_3D,                 //Make a 3D histogram from the data

  PT_GRAPH,                    //Make a 1D graph from the data
  PT_GRAPH_ER,                 //Make a 1D graph with errorbars from the data
  PT_GRAPH_ASYMER,             //Make a 1D graph with asymetric errorbars 
                               //from the data
  PT_MULTIGRAPH,               //Make a 1D multigraph from the data
  PT_SCATTER_2D,
  PT_PROFILE,
};

enum RDWCommandId {

  M_PLOT_SAVEPS       = 1100,
  M_PLOT_SAVERT,
  M_PLOT_SAVESETUP,
  M_PLOT_FITSETUP,
  M_PLOT_ATTR,
  M_PLOT_ADD,
  M_PLOT_CLOSE,
  M_PLOT_PRINT,
  M_PLOT_OPEN,
  M_PLOT_ADDLEGEND,
  M_PLOT_WRITEMISC,
  M_PLOT_READMISC,
  M_PLOT_INTEG,
  M_PLOT_DIVIDE,
  M_PLOT_SCALE,
  M_PLOT_NORM,
  M_PLOT_VIEWROOTOBJ,
  M_PLOT_EDITOR,

  M_DTWIND_CLOSED,
  M_DTWIND_LOGTXTTS,
  M_DTWIND_ERROR_MSG
};

//enum ERRDWMessageTypes {
#define kC_DTWIND_OBJECT    1300 
#define kCM_DTWIND_MSG      1301
    //};

#endif
