#ifndef _HTAL_H_
#define _HTAL_H_

/****************************************************************************/
/*    FILE: HTAL.h                                                          */
/*                                                                          */
/*    Persentation of the standard interfaces which should be               */
/*    implemented for any low level hardware timer.                         */
/*    BY:   Yahia Farghaly                                                  */
/*          17-Dec-2017	                                                    */
/*    Email: yahiafarghaly@gmail.com                                        */
/*                                                                          */
/****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Function: HTAL_startPhysicalTimer
 * ----------------------------
 *   Start the operation of the hardware timer for a single shot type
 *   and a timeout equals to timePeriodMilliSec in milliseconds.
 *
 *   timePeriodMilliSec  : timeout of the timer in milliseconds. 
 *   userTimerCallBack   : user timer callback when timeout is reached.
 *   userTimerCallbackArg: user timer callback arguments casted to void*
 * 
 *   returns: void
 *   Implementation: Required as described.
 */
void HTAL_startPhysicalTimer(long timePeriodMilliSec,
                             void (*userTimerCallBack)(void *),
                             void* userTimerCallbackArg);

/*
 * Function: HTAL_stopPhysicalTimer
 * ----------------------------
 *   Stop the operation of the hardware timer immediately.
 * 
 *   returns: void
 *   Implementation: Required as described.
 */
void HTAL_stopPhysicalTimer(void);

/*
 * Function: HTAL_remainingTime
 * ----------------------------
 *   Calculate the remaining time before the hardware timer timeouts.
 * 
 *   returns: remaining time in milliseconds.
 *   Implementation: Required as described. 
 *                   In case of in capability, just (return 0) but you will
 *                   lose percise timing from VTAL operations.
 */
long HTAL_remainingTime(void);

/*
 * Function: HTAL_changeUserTimerCallBack
 * ----------------------------
 *   Change the user timer callback and its arguments before the timer
 *   timeouts.
 *
 *   userTimerCallBack   : user timer callback when timeout is reached.
 *   userTimerCallbackArg: user timer callback arguments casted to void*
 * 
 *   returns: void
 *   Implementation: Optional.
 */
void HTAL_changeUserTimerCallBack(void (*userTimerCallBack)(void *),
                                  void *userTimerCallbackArg);

/*
 * Function: HTAL_notifyTimeoutToVTAL
 * ----------------------------
 *   Notify the VTAL that the hardware timer is timed out and its Interrupt
 *   is called.
 *   This function must be called at end of the hardware timer interrupt
 *   handler and after the userTimerCallBack() function.
 * 
 *   Implementation: Not required, it is already implemented in VTAL.
 */
void HTAL_notifyTimeoutToVTAL(void);


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /*  _HTAL_H_   */