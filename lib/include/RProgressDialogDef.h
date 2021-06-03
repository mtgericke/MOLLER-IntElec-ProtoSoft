#ifndef RPROGRESSDIALOGDEF_H
#define RPROGRESSDIALOGDEF_H

#ifndef MSG_QUEUE_MAX
#define MSG_QUEUE_MAX         10           //Maximum entries per message
#endif                                     //queue
#ifndef MSG_SIZE_MAX
#define MSG_SIZE_MAX          1000         //Maximum size of each message
#endif
#ifndef NAME_STR_MAX
#define NAME_STR_MAX          200          //Maximum file name length
#endif

#ifndef kTrue
#define kTrue                 1
#endif
#ifndef kFalse
#define kFalse                0
#endif

#define kC_PR_DIALOG          1500
#define kCM_PR_MSG             150



enum ProgressComIdentifiers {

  M_PR_SEQ,
  M_PR_RUN,
  M_PR_CLOSED,
  M_PR_CLOSE,
  M_PR_STOP,
  M_PR_PAUSE,
  M_PR_RESUME

};

#endif
