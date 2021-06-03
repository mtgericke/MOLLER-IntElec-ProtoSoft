///////////////////////////////////////////////////////////////////////////////
//
// name: CMMonitor.cxx
// date: 5-29-2021
// auth: Michael Gericke 
//
///////////////////////////////////////////////////////////////////////////////

#ifndef CMMONITORDEF_H
#define CMMONITORDEF_H

#include "RDataWindowDef.h"
#include "RDataContainerDef.h"

/* #define NPDG_DEBUG */


#ifndef Pi
#define Pi                       3.14159265359
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


#define ANL_MAX_OBJ_CNT                  10000 //Maximum number of obj used in 
                                               //Analysis
#define ADC_CONV_FACTOR                 3276.8 //ADC sample unit conversion factor
                                               //(counts per Volt)
#define AMPL_NOISE                     1.9e-14 //amps per sqrt(Hz) from amplifier
#define MAIN_GAIN                         50e6 //First amp stage gain (Ohms)
#define SAMPL_FAC                          0.6 //from bessel filtering (see lab book)
#define SAMPL_BW                          4200 //Bandwidth (Hz) set by the Sum-Diff Amps 
#define DAQ_SDFAC                            1 //Summing and Differencing
                                               //noise enhancement factor
#define COSMIC_DEP_MEV                      85 //Energy deposited in a detector
                                               //by a single cosmic traversing
                                               //the entire detector length (MeV)
#define e_CHARGE                1.60217733e-19 //electron charge in Coulombs
#define v_LIGHT                 2.99792458e8   //speed of light in m/s
#define n_MASS                  939.56533e6    //neutron mass in eV/c^2             

#ifndef kTrue
#define kTrue                             1
#endif
#ifndef kFalse
#define kFalse                            0
#endif

#define N_AVOG                    6.0221367e23

#define ERR_RET_VAL                 400000000 

#define QUEUE_GRPUFLOW_ERROR   "ERROR: Trying to read beyond end of file.\n"
#define FILE_RANGE_ERROR       "ERROR: Invalid macro pulse range selected.\n"
#define ANL_ALLOC_ERROR        "ERROR: Unable to allocate memory for analysis\n"
#define ANL_BINNING_ERROR      "ERROR: Unable to auto establish binning of histos\n"

//Error Messages
#define LIST_ITEM_CREATE_ERROR                1011
#define LIST_ITEM_DELETE_ERROR                1012
#ifndef PROCESS_OK
#define PROCESS_OK                            1013
#endif
#ifndef PROCESS_FAILED
#define PROCESS_FAILED                        1014
#endif
#define PROCESS_CANCELED                      1015
#define MEMORY_ALLOC_ERROR                    1016
#ifndef LOG_PROCESS_ERROR
#define LOG_PROCESS_ERROR                     1017
#endif
#ifndef LOG_PROCESS_OK
#define LOG_PROCESS_OK                        1018
#endif

#define kC_PR_DIALOG                   1500
#define kCM_PR_MSG                      150
#define kC_PLO_DIALOG                  1060
#define kCM_PLO_MSG                     106
#define kC_MATH_DIALOG                 1400
#define kCM_MATH_MSG                    140
#define kC_ANL_ENT_DIALOG              1100
#define kCM_ANL_ENT_MSG                 110

#define NUM_CHANNELS                      2


enum CommandIdentifiers {

  M_HP_LOGTXTTS,
  M_HP_LOGTXTNS,
  M_HP_CLOSED,
  M_DC_LOGTXTTS,
  M_DC_LOGTXTNS,
  M_DC_CLOSED,
  M_HS_LOGTXTTS,
  M_HS_LOGTXTNS,
  M_HS_CLOSED,
  M_PL_LOGTXTTS,
  M_PL_LOGTXTNS,
  M_PLO_CLOSED,
  M_MATH_LOGTXTTS,
  M_MATH_LOGTXTNS,
  M_MATH_ERROR_MSG,

  M_HPE_LOGTXTTS,
  M_HPE_LOGTXTNS,
  M_HPE_CLOSED,
  M_HSE_LOGTXTTS,
  M_HSE_LOGTXTNS,
  M_HSE_CLOSED,
  M_PLE_LOGTXTTS,
  M_PLE_LOGTXTNS,
  M_PLE_CLOSED,
  M_CALC_SFL_EFF,
  M_SET_SFL_EFF_PARMS,
  M_SET_CONNECT,

  M_ANL_PR_SEQ,
  M_ANL_PR_RUN,
  M_ANL_CLOSE,
  M_ANL_CLOSED,
  M_ANLE_CLOSE,
  M_ANLE_CLOSED,
  M_ANL_PLOT_SELFUNC,
  M_TOF_DEFAULTS,
  M_PR_SEQ,
  M_PR_SEQ2,
  M_PR_RUN,
  M_PR_CLOSED,
  M_PR_CLOSE,
  M_PR_STOP,
  M_PR_PAUSE,
  M_PR_RESUME,

  M_CSDE_CLOSE,
  M_CSDE_CLOSED,
  
  M_WRITE_RC_DATA,

  M_FILE_NEW,
  M_ROOT_FILE_NEW,
  M_FILE_OPEN,
  M_ROOT_FILE_OPEN,
  M_FILE_SAVE,
  M_FILE_SAVEAS,
  M_FILE_PRINT,
  M_FILE_PRINTSETUP,
  M_FILE_EXIT,
  M_FILE_CLOSE,
  M_ROOT_FILE_CLOSE,
  M_FILE_ANALYZE,
  M_FILE_NOISESTAT,
  M_FILE_ASYMSTAT,
  M_DRAW_ARRAY,
  M_OPEN_DATAWINDOW,

  M_VIEW_LOG,
  M_VIEW_DET_DATA,
  M_VIEW_DET_HIST,
  M_VIEW_DET_WAVE,
  M_VIEW_DET_WAVESC,
  M_VIEW_SUM_HIST,
  M_VIEW_SUM_WAVE,
  M_VIEW_DIFF_HIST,
  M_VIEW_DIFF_WAVE,
  M_VIEW_MON_DATA,
  M_VIEW_MON_HIST,
  M_VIEW_MON_WAVE,
  M_VIEW_SFL_DATA,
  M_VIEW_SFL_HIST,
  M_VIEW_SFL_WAVE,
  M_VIEW_RHIST_HIST,
  M_VIEW_RHIST_WAVE,
  M_VIEW_MISC_DATA,
  M_VIEW_TUN_HIST,
  M_VIEW_BROWSER,
  M_VIEW_ANL_POL,
  M_SET_CONFIG,
  M_DIS_CONSCH,
  M_RESET_GEOM_VIEW,
  M_CALC_DETCROSS,
  M_VIEW_GEOMETRY,
  M_TBIN_SELECT,
  M_TARGET_SELECT,
  M_RUN_SELECT,
  M_ANL_ASYM,
  M_GEN_PEDS,
  M_GEN_BGRND,
  M_GEN_COMPR,
  M_GEN_UPR,
  M_GEN_UPR2,
  M_GEN_UPR3,

  M_PLOT_DET_DATA,
  M_PLOT_DIF_DATA,
  M_PLOT_SUM_DATA,
  M_HIST_DET_DATA,
  M_HIST_DIF_DATA,
  M_HIST_SUM_DATA,
  M_COMB_PH_DATA,
  M_ANL_OUTPUT,
  
  M_HISTO_SAVE,
  M_HISTO_SAVESETUP,
  
  M_HISTO_CLOSE,
  M_HISTO_ENTRY_SET,
  M_HISTO_ENTRY_CANCEL,
  
  M_HISTOPLOT_SAVE,
  M_HISTOPLOT_SAVESETUP,
  
  M_HISTOPLOT_CLOSE,
  M_HISTOPLOT_ENTRY_SET,
  M_HISTOPLOT_ENTRY_CANCEL,

  M_ANALYZE_CLOSE,
  M_ANALYZE_ENTRY_SET,
  M_ANALYZE_ENTRY_CANCEL,
  M_SETTING_ENTRY_SET,
  M_SETTING_ENTRY_CANCEL,
  M_ANALYZE_ALL_MPS,
  M_AUTO_BIN,
  M_SUB_PED,
  M_SUB_BGR,

  M_DATAVIEW_ENTRY_SET,
  M_DATAVIEW_ENTRY_CANCEL,

  M_FIT_CLOSE,
  M_FIT_ENTRY_SET,
  M_FIT_ENTRY_CANCEL,
  M_FIT_FULLRANGE,
  M_FIT_SELFUNC,
  M_FITE_CLOSE,
  M_FITE_CLOSED,
  
  M_HELP_CONTENTS,
  M_HELP_SEARCH,
  M_HELP_ABOUT,
  
  VId1,
  HId1,
  VId2,
  HId2,
  
  VSId1,
  HSId1,
  VSId2,
  HSId2,
  
  ColorSel
};


enum ENDetOpMode {
  DOM_DATA,
  DOM_SIM,
};

enum ERFileStatus {
  FS_NEW,
  FS_OLD,
};

enum ENMsgType {
  MT_START,
  MT_END,
  MT_HALT,
};


struct ClineArgs {

  int            bckgr;
  int            mute;
  int            realtime;
  int            checkmode;
  char           file[1000];

};




#endif
