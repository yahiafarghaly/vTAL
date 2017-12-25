#ifndef _VTAL_H_
#define _VTAL_H_

/****************************************************************************/
/*    FILE: vTAL.h                                                          */
/*                                                                          */
/*    Persentation of the standard interfaces for using a virtual timer.    */
/*                                                                          */
/*    BY:   Yahia Farghaly                                                  */
/*          25-Dec-2017	                                                    */
/*    Email: yahiafarghaly@gmail.com                                        */
/*                                                                          */
/****************************************************************************/

#include "vTALConfig.h"

/*! Type of user's timer callback and its argument type.    */
typedef void*               VTAL_tCallBackArg;
typedef void (*VTAL_tCALLBACK)(VTAL_tCallBackArg);

/*! The mode of the virtual timer.   */
typedef enum
{
    VTAL_ONE_SHOT_TIMER,
    VTAL_PERIODIC_TIMER
}VTAL_tenuTimerMode;

/*! Time structure of the virtual timer.    */
typedef struct
{
    VTAL_tTimeSec       seconds;
    VTAL_tTimeMilliSec  milliseconds;
}VTAL_tstrTimeSettings;

/*! Configuration structure of the virtual timer.   */
typedef struct
{
    VTAL_tTimerId           timerID;
    VTAL_tstrTimeSettings   expiredTime;
    VTAL_tenuTimerMode      timerMode;
    VTAL_tCALLBACK          expiredTimeEvent;
    VTAL_tCallBackArg       eventContextInfo;
}VTAL_tstrConfig;

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Function:  VTAL_init
 * ----------------------------
 *   It Does what is neccessary to prepare VTAL for proper functionality.
 *   It is called only once in the program life and before using any of VTAL_* APIs.
 * 
 *   returns: void
 */
void VTAL_init(void);

/*
 * Function: VTAL_addTimer
 * ----------------------------
 *   It adds a new timer to a list of timers to be executed in the proper order.
 *   --- Some corner Cases:
 *   1 - If a timer with a .timerID field is added for a second time and its callback has not 
 *       been called yets, the function doesn't add it and stick with the previous added timer.
 *       Only VTAL_removeTimer() has an effect of removing it then you can add it again with 
 *       the same/different configuration as desired.
 * 
 *   2 - If the added timer with an expiredTime field = 0 , and the timer mode is (periodic), 
 *       it is expiredTime field is modified to be 1 ms to give other timers a chance for its 
 *       callback to be executed. 
 *      
 *   VTAL_tpstrConfig  : Configuration of the added timer.
 * 
 *   Note: the execution of the timer callback should be fast and non-blocking since on embedded
 *         environment with a single core, it is not possible to create the timer callback on 
 *         another thread as it may be done on the Operation systems as Linux/windows.
 * 
 *   returns: void
 */
void VTAL_addTimer(VTAL_tstrConfig* VTAL_tpstrConfig);

/*
 * Function: VTAL_removeTimer
 * ----------------------------
 *   Remove a previously added timer with a .timerID field in case it is .expiredTime  field
 *   is not come yet.
 *
 *   timerID   : The id of the desired removed timer.
 * 
 *   returns: void
 */
void VTAL_removeTimer(VTAL_tTimerId timerID);

#ifdef __DEBUG__
/*
 * Function: VTAL_removeTimer
 * ----------------------------
 *   Show the a list of the added timers with their relative times to be fired.
 *   returns: void
 */
    void VTAL_showTimerList();
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*   _VTAL_H_ */