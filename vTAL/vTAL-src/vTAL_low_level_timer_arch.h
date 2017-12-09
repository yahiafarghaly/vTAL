#ifndef _VTAL_LOW_LEVEL_TIMER_ARCH_H_
#define _VTAL_LOW_LEVEL_TIMER_ARCH_H_


/*  Standard Intefaces for any low level implementation */


#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    HTAL_ONE_SHOT_TIMER,
    HTAL_PERIODIC_TIMER
}HTAL_tenuTimerMode;

void HTAL_startPhysicalTimer(unsigned long timePeriodMilliSec,
                             void (*userTimerCallBack)(void *),
                             HTAL_tenuTimerMode userTimerMode,
                             void(*VTAL_updateCallBack)(void));

void HTAL_stopPhysicalTimer(void);


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /*   _VTAL_LOW_LEVEL_TIMER_ARCH_H_   */