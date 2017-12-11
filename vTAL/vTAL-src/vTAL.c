#include <vTAL.h>
#include "vTAL_low_level_timer_arch.h"

#define EMPTY_LIST      (-1)
#define EMPTY_CONFIG    ((void*)0)
#define NOT_FOUND       (-1)

typedef enum
{
    NOT_ACTIVE = 0x0C0C,
    ACTIVE = 0x00DB
}VTAL_tenuTimerStatus;

typedef struct
{
    VTAL_tTimerId           timerID;
    VTAL_tenuTimerMode      timerMode;
    VTAL_tCALLBACK          expiredTimeEvent;
    VTAL_tTimeMilliSec      relativeTimeoutmSec;
    VTAL_tenuTimerStatus    timerStatus;
}VTAL_tstrTimerInfo;

static VTAL_tTimeMilliSec gAbsoulateTimeoutmSec = 0;
static long gTimersListSize = EMPTY_LIST;

static VTAL_tstrTimerInfo gTimersList[NUMBER_OF_TIMERS];


#ifdef __cplusplus
extern "C" {
#endif

static void updateTimersList(void);
static void shiftTimersListOneStepForward(int lastIdxToShift);
static int findTimer(VTAL_tTimerId timerID);

#ifdef __cplusplus
} /* extern "C" */
#endif


void VTAL_addTimer(VTAL_tstrConfig* VTAL_tpstrConfig)
{
    if (VTAL_tpstrConfig == EMPTY_CONFIG)
        return;

    /* Search first if this Timer is already inserted.  */
    if(findTimer(VTAL_tpstrConfig->timerID) != NOT_FOUND)
        return;

    if (gTimersListSize == EMPTY_LIST)
    {
        gTimersList[0].timerID = VTAL_tpstrConfig->timerID;
        gTimersList[0].expiredTimeEvent = VTAL_tpstrConfig->expiredTimeEvent;
        gTimersList[0].timerMode = VTAL_tpstrConfig->timerMode;
        gTimersList[0].relativeTimeoutmSec =
            VTAL_tpstrConfig->expiredTime.milliseconds +
            (VTAL_tpstrConfig->expiredTime.seconds) * 1000;
        gTimersList[0].timerStatus  = ACTIVE;
        gAbsoulateTimeoutmSec = gTimersList[0].relativeTimeoutmSec;
        gTimersListSize = 1;

        /* start low level timer */
       /* HTAL_startPhysicalTimer(gAbsoulateTimeoutmSec,
                                gTimersList[0].expiredTimeEvent,
                                (HTAL_tenuTimerMode)gTimersList[0].timerMode);*/
    }
    else
    {
        VTAL_tTimeMilliSec newTimerRelativeTimeout;
        unsigned long Idx;
        VTAL_tTimeMilliSec accumlatedRelativeTimes;

        VTAL_tTimeMilliSec newTimerAbsTimeout =
            VTAL_tpstrConfig->expiredTime.milliseconds +
            (VTAL_tpstrConfig->expiredTime.seconds) * 1000;
        
        /*! 
         * When NewTimerAbsTimeout is >= gAbsoulateTimeout, 
         * 1- We put this newTimer at the end of the list.
         * 2- Insert the relative time w.r.t to gAbsoulateTimeout.
         * 3- Finally, we update the gAbsoulateTimeout for the new abs timeout
         *      referrence.
         * 
         * In case of equal absTimeout == newTimerAbsTimeout, this will result
         * in a relative time for this timer to be zero. which will be fired
         * automatically after the previous timer.
         *  */
        newTimerRelativeTimeout = newTimerAbsTimeout - gAbsoulateTimeoutmSec;

        if (newTimerRelativeTimeout >= 0)
        {
            gTimersList[gTimersListSize].timerID = VTAL_tpstrConfig->timerID;
            gTimersList[gTimersListSize].expiredTimeEvent = VTAL_tpstrConfig->expiredTimeEvent;
            gTimersList[gTimersListSize].timerMode = VTAL_tpstrConfig->timerMode;
            gTimersList[gTimersListSize].relativeTimeoutmSec = newTimerRelativeTimeout;
            gTimersList[gTimersListSize].timerStatus = ACTIVE;
            ++gTimersListSize;
            gAbsoulateTimeoutmSec = newTimerAbsTimeout;
            return;
        }

        /*! 
         * Generalized Case when the relative time/s of timers are less than 
         * the relative time of a timer in [Idx] Position.
         * When finding this Idx position:
         * 1- Firstly, we shift the whole array one step forward till position
         *    [Idx + 1] becomes the first position in this shifted array.
         * 2- Then, the new timer is inserted with a relative timeout equals to 
         *    its absoulate time - sum of all previous relative times of timers.
         * 3- Then, the timer in the next of the inserted timer (i.e, @[Idx+1]),
         *    its relative timing is substracted a certain amount which was
         *    given for the new inserted timer @[Idx] position.
         * Best case: 1 iteration in the loop.
         *  the first timer relative time is greater than the inserted.
         * When newTimerAbsTimeout - accumlatedRelativeTimes == 0, this means
         * that the inserted timer will have a relative timeout of 0 which will
         * fire its callback immediately if available after the previous time 
         * finishes.
         *  */
        accumlatedRelativeTimes = 0;
        for (Idx = 0; Idx < gTimersListSize; ++Idx)
        {
            if ((newTimerAbsTimeout - accumlatedRelativeTimes) < gTimersList[Idx].relativeTimeoutmSec)
            {
                shiftTimersListOneStepForward(Idx + 1);
                gTimersList[Idx].timerID = VTAL_tpstrConfig->timerID;
                gTimersList[Idx].expiredTimeEvent = VTAL_tpstrConfig->expiredTimeEvent;
                gTimersList[Idx].timerMode = VTAL_tpstrConfig->timerMode;
                gTimersList[Idx].timerStatus = ACTIVE;

                gTimersList[Idx].relativeTimeoutmSec = newTimerAbsTimeout - accumlatedRelativeTimes;
                gTimersList[Idx + 1].relativeTimeoutmSec =
                    gTimersList[Idx + 1].relativeTimeoutmSec - gTimersList[Idx].relativeTimeoutmSec;
                ++gTimersListSize;
                return;
            }
            accumlatedRelativeTimes += gTimersList[Idx].relativeTimeoutmSec;
        }
    }
}

void VTAL_removeTimer(VTAL_tTimerId timerID)
{
    int timerIdx = findTimer(timerID);
    if (timerIdx == NOT_FOUND)
        return;
    gTimersList[timerIdx].timerStatus = NOT_ACTIVE;
    gTimersList[timerIdx].expiredTimeEvent = ((void*)0);
    /*  Not completed yet   */
    /*gTimersList[timerIdx].*/
}

void updateTimersList(void)
{

}

void shiftTimersListOneStepForward(int lastIdxToShift)
{
    int idx;
    if (lastIdxToShift <= 0)
        return;
    for (idx = gTimersListSize; idx >= lastIdxToShift; --idx)
        gTimersList[idx] = gTimersList[idx - 1];
}

static int findTimer(VTAL_tTimerId timerID)
{
    int i = 0;
    if (gTimersListSize == EMPTY_LIST)
        return NOT_FOUND;
    do
    {
        if (gTimersList[i].timerID == timerID)
            return i;
        ++i;
    } while (i < gTimersListSize);

    return NOT_FOUND;
}

#ifdef __DEBUG__
#include <stdio.h>

void VTAL_showTimerList()
{
    int i = 0;
    if(gTimersListSize == EMPTY_LIST)
    {
        printf("Empty list\n");
        return;
    }

    printf("-----------------\n");
    printf("T_abs = %ld\n",gAbsoulateTimeoutmSec);
    for(i = 0; i < gTimersListSize;i++)
    {
        printf("T_rel[%d] = %ld, ( id = %d )\n",i,gTimersList[i].relativeTimeoutmSec,
        gTimersList[i].timerID);
    }
}

#endif