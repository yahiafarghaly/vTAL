#ifndef _VTAL_H_
#define _VTAL_H_

#include "vTALConfig.h"

typedef void*               VTAL_tCallBackArg;
typedef void (*VTAL_tCALLBACK)(VTAL_tCallBackArg);

typedef enum
{
    VTAL_ONE_SHOT_TIMER,
    VTAL_PERIODIC_TIMER
}VTAL_tenuTimerMode;

typedef struct
{
    VTAL_tTimeSec       seconds;
    VTAL_tTimeMilliSec  milliseconds;
}VTAL_tstrTimeSettings;

typedef struct
{
    VTAL_tTimerId           timerID;
    VTAL_tstrTimeSettings   expiredTime;
    VTAL_tenuTimerMode      timerMode;
    VTAL_tCALLBACK          expiredTimeEvent;
}VTAL_tstrConfig;

#ifdef __cplusplus
extern "C" {
#endif

/*  Once it is added, it starts*/
void VTAL_addTimer(VTAL_tstrConfig* VTAL_tpstrConfig);

void VTAL_removeTimer(VTAL_tTimerId timerID);

#define __VTAL_DEBUG__
#ifdef __VTAL_DEBUG__
    void VTAL_showTimerList();
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*   _VTAL_H_ */