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
        HTAL_startPhysicalTimer(gAbsoulateTimeoutmSec,
                                gTimersList[0].expiredTimeEvent,
                                (HTAL_tenuTimerMode)gTimersList[0].timerMode,
                                updateTimersList);
    }
    else
    {
        unsigned long Idx = 0;
        VTAL_tTimeMilliSec newTimerAbsTimeout =
            VTAL_tpstrConfig->expiredTime.milliseconds +
            (VTAL_tpstrConfig->expiredTime.seconds) * 1000;
        VTAL_tTimeMilliSec accumlatedRelativeTimes = 0;

        for (Idx = 0; Idx < gTimersListSize; ++Idx)
        {
            if (newTimerAbsTimeout < gTimersList[Idx].relativeTimeoutmSec)
            {
                /*!
                * O(1) case when first element is larger than new timer.
                * 1 - Shift TimersList by 1 position.
                * 2 - Insert newTimer at In index [0] with newTimerAbsTimeout.
                * 3 - take some time from gTimersList[1] due to [0].
                */
                shiftTimersListOneStepForward(Idx + 1);
                gTimersList[Idx].timerID = VTAL_tpstrConfig->timerID;
                gTimersList[Idx].expiredTimeEvent = VTAL_tpstrConfig->expiredTimeEvent;
                gTimersList[Idx].timerMode = VTAL_tpstrConfig->timerMode;
                gTimersList[Idx].timerStatus = ACTIVE;

                gTimersList[Idx].relativeTimeoutmSec = newTimerAbsTimeout;
                gTimersList[Idx + 1].relativeTimeoutmSec =
                    gTimersList[Idx + 1].relativeTimeoutmSec - gTimersList[Idx].relativeTimeoutmSec;
                ++gTimersListSize;
                return;
            }
            else if(accumlatedRelativeTimes == newTimerAbsTimeout)
            {
                shiftTimersListOneStepForward(Idx + 1);
                gTimersList[Idx].timerID = VTAL_tpstrConfig->timerID;
                gTimersList[Idx].expiredTimeEvent = VTAL_tpstrConfig->expiredTimeEvent;
                gTimersList[Idx].timerMode = VTAL_tpstrConfig->timerMode;
                gTimersList[Idx].timerStatus = ACTIVE;

                gTimersList[Idx].relativeTimeoutmSec = 0;
                ++gTimersListSize;
                return;
            }
            else if ((newTimerAbsTimeout - accumlatedRelativeTimes) > 0 &&
            ((newTimerAbsTimeout - accumlatedRelativeTimes) < gTimersList[Idx].relativeTimeoutmSec))
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
        /*! 
         * We reach this point when NewTimerAbsTimeout is >= absTimeout.
         * So, we put this newTimer at the end,substracting absTimeout from it.
         * 
         * In case of equal absTimeout == newTimerAbsTimeout, this will result
         * in a relative time for this timer to be zero. which will be fired
         * automatically after the previous timer. */
        VTAL_tTimeMilliSec newTimerRelativeTimeout;
        newTimerRelativeTimeout = newTimerAbsTimeout - gAbsoulateTimeoutmSec;

        gTimersList[gTimersListSize].timerID = VTAL_tpstrConfig->timerID;
        gTimersList[gTimersListSize].expiredTimeEvent = VTAL_tpstrConfig->expiredTimeEvent;
        gTimersList[gTimersListSize].timerMode = VTAL_tpstrConfig->timerMode;
        gTimersList[gTimersListSize].relativeTimeoutmSec = newTimerRelativeTimeout;
        gTimersList[gTimersListSize].timerStatus = ACTIVE;
        ++gTimersListSize;
        gAbsoulateTimeoutmSec = newTimerAbsTimeout;
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
    if(lastIdxToShift <= 0)
        return;
    int idx;
    for(idx = gTimersListSize; idx >= lastIdxToShift; --idx )
        gTimersList[idx] =  gTimersList[idx - 1]; 
}

static int findTimer(VTAL_tTimerId timerID)
{
    if (gTimersListSize == EMPTY_LIST)
        return NOT_FOUND;
    int i = 0;
    do
    {
        if (gTimersList[i].timerID == timerID)
            return i;
        ++i;
    } while (i < gTimersListSize);

    return NOT_FOUND;
}

#ifdef __VTAL_DEBUG__
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
    printf("T_abs = %d\n",gAbsoulateTimeoutmSec);
    for(i = 0; i < gTimersListSize;i++)
    {
        printf("T_rel[%d] = %d, ( id = %d )\n",i,gTimersList[i].relativeTimeoutmSec,
        gTimersList[i].timerID);
    }
}

#endif