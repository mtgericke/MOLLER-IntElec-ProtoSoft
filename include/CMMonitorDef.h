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

#define SAMPLES_PER_SECOND    14705883
#define MAX_SAMPLES (SAMPLES_PER_SECOND*5)
#define MAX_ALLOC   ((MAX_SAMPLES+MAX_SAMPLES)*8)
#define ADC_CONVERSION 0.00003125         // 2^-17*4.096  V/bin
#define TS_TO_NS  4                       // nanoseconds per timestamp tick
#define TS_CONVERSION  17                 // Number of cycles to convert (68ns  / 4ns) at 250Mhz ADC clock

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
  M_CONNECT,

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
  M_FILE_SAVE_SETTINGS,
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
  M_RUN_START,
  M_RUN_STOP,
  M_RUN_PAUSE,
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
  M_IPA_QUERY,
  M_SDIV_SELECT,
  M_RUN_SEQ_SELECT,

  CMB_CH0SEL,
  LB_CH0_ID1,
  LB_CH0_ID2,
  LB_CH0_ID3,
  LB_CH0_ID4,
  LB_CH0_ID5,
  LB_CH0_ID6,
  LB_CH0_ID7,
  LB_CH0_ID8,
  LB_CH0_ID9,
  LB_CH0_ID10,
  LB_CH0_ID11,
  LB_CH0_ID12,
  LB_CH0_ID13,
  LB_CH0_ID14,
  LB_CH0_ID15,
  LB_CH0_ID16,

  CMB_CH1SEL,
  LB_CH1_ID1,
  LB_CH1_ID2,
  LB_CH1_ID3,
  LB_CH1_ID4,
  LB_CH1_ID5,
  LB_CH1_ID6,
  LB_CH1_ID7,
  LB_CH1_ID8,
  LB_CH1_ID9,
  LB_CH1_ID10,
  LB_CH1_ID11,
  LB_CH1_ID12,
  LB_CH1_ID13,
  LB_CH1_ID14,
  LB_CH1_ID15,
  LB_CH1_ID16,

  BTN_TGRAPH,
  BTN_HSTHR,
  BTN_FFT,
  BTN_RUN_START,
  BTN_CLEAR_PL,
  M_RUN_TIME_SELECT,
  M_SET_IP

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
