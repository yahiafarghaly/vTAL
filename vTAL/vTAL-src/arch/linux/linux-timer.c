#include "../../vTAL_low_level_timer_arch.h"

#ifdef __linux__    /* To Prevent Compiling it if in another arch   */



void HTAL_startPhysicalTimer(unsigned long timePeriodMilliSec,
                             void (*userTimerCallBack)(void *),
                             HTAL_tenuTimerMode userTimerMode,
                             void (*VTAL_updateCallBack)(void))
{

}

void HTAL_stopPhysicalTimer(void)
{

}



#endif  /*   __linux__   */