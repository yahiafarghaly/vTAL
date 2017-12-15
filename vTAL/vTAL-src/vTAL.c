#include <vTAL.h>
#include "vTAL_low_level_timer_arch.h"

#define EMPTY_LIST      (-1)
#define EMPTY_CONFIG    ((void*)0)
#define NOT_FOUND       (-1)

typedef enum
{
    NOT_AVAILABLE = 0x0C0C,
    ACTIVE = 0x00DB,
    READY = 0x0FCF
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
static long gCurrentRunningTimerIdx = -1;
static VTAL_tstrTimerInfo gTimersList[NUMBER_OF_TIMERS];
/*!
 * To resolve the race condition between adding/removing timer functions
 * And the timer update list function. Remember: updateTimersList() is
 * called Async from the physcial timer not in user space. so we should take
 * care since gTimersList[] is changing among these functions.  */ 
static unsigned long gMutexVariable = 1;


#ifdef __cplusplus
extern "C" {
#endif

#define VTAL_LOCK()                 \
    do                              \
    {                               \
        while (gMutexVariable == 0) \
            ;                       \
        gMutexVariable = 0;         \
    } while (0);

#define VTAL_UNLOCK() gMutexVariable = 1;

static void updateTimersList(void);
static void shiftTimersListOneStepForward(int lastIdxToShift);
static void shiftTimersListOneStepBackward(int toIdx);
static int findTimer(VTAL_tTimerId timerID);

#ifdef __cplusplus
} /* extern "C" */
#endif


void VTAL_addTimer(VTAL_tstrConfig* VTAL_tpstrConfig)
{
    if (VTAL_tpstrConfig == EMPTY_CONFIG)
        return;

    VTAL_LOCK();
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
        gCurrentRunningTimerIdx = 0;
        /*Assign Timerslist Update function */
        HTAL_updateVirtualTimersList(updateTimersList);
        /* start low level timer */
        HTAL_startPhysicalTimer(gAbsoulateTimeoutmSec,
                                gTimersList[0].expiredTimeEvent);
        VTAL_UNLOCK();
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
            gTimersList[gTimersListSize].timerStatus = READY;
            ++gTimersListSize;
            gAbsoulateTimeoutmSec = newTimerAbsTimeout;
            VTAL_UNLOCK();
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
                /*If first node is being changed, Change its 
                    user callback & timeout quickly @ lower level */
                if(Idx == 0)
                {
                    HTAL_changeUserTimerCallBack(VTAL_tpstrConfig->expiredTimeEvent);
                    /* This is not correct by any means to stop the actual timing &
                        should be placed with the correct behaviour/Interface.*/
                    HTAL_stopPhysicalTimer();
                    HTAL_startPhysicalTimer(newTimerAbsTimeout,
                                            VTAL_tpstrConfig->expiredTimeEvent);
                }
                shiftTimersListOneStepForward(Idx + 1);
                gTimersList[Idx].timerID = VTAL_tpstrConfig->timerID;
                gTimersList[Idx].expiredTimeEvent = VTAL_tpstrConfig->expiredTimeEvent;
                gTimersList[Idx].timerMode = VTAL_tpstrConfig->timerMode;
                gTimersList[Idx].timerStatus = ((Idx == 0) ? ACTIVE : READY);

                gTimersList[Idx].relativeTimeoutmSec = newTimerAbsTimeout - accumlatedRelativeTimes;
                gTimersList[Idx + 1].relativeTimeoutmSec =
                    gTimersList[Idx + 1].relativeTimeoutmSec - gTimersList[Idx].relativeTimeoutmSec;
                ++gTimersListSize;
                VTAL_UNLOCK();
                return;
            }
            accumlatedRelativeTimes += gTimersList[Idx].relativeTimeoutmSec;
        }
    }
}

void VTAL_removeTimer(VTAL_tTimerId timerID)
{
    VTAL_LOCK();
    int timerIdx = findTimer(timerID);
    if (timerIdx == NOT_FOUND)
        return;
    /*!
     * Add what will be removed to the next timer to maintain the same absolute
     * time and don't change other timers relative time. 
     * Change only the abs time in the case that this timerID 
     * is the last timer in the list.
     *  */
    if((timerIdx + 1) == gTimersListSize)
    {
        gAbsoulateTimeoutmSec -= gTimersList[timerIdx].relativeTimeoutmSec;
    }
    else
    {
        gTimersList[timerIdx + 1].relativeTimeoutmSec += gTimersList[timerIdx].relativeTimeoutmSec;
        shiftTimersListOneStepBackward(timerIdx);
    }  
    --gTimersListSize;
    VTAL_UNLOCK();
}

void updateTimersList(void)
{
    VTAL_LOCK();
    ++gCurrentRunningTimerIdx;
    if (gCurrentRunningTimerIdx >= gTimersListSize)
    {
        gCurrentRunningTimerIdx = -1;
        gTimersListSize == EMPTY_LIST;
    }
    else
    {
       /*!
        * When having multiple of Logical timers with the same timeout. By
        * design the next relative timeout will be zero, So we just execute 
        * user callback immediatly.
        */
        if (gTimersList[gCurrentRunningTimerIdx].relativeTimeoutmSec == 0)
        {
            gTimersList[gCurrentRunningTimerIdx].expiredTimeEvent((void *)0);
            VTAL_UNLOCK();
            updateTimersList();
        }
        else
        {
            HTAL_startPhysicalTimer(
                gTimersList[gCurrentRunningTimerIdx].relativeTimeoutmSec,
                gTimersList[gCurrentRunningTimerIdx].expiredTimeEvent);
        }
    }
    VTAL_UNLOCK();
}

void shiftTimersListOneStepForward(int lastIdxToShift)
{
    int idx;
    if (lastIdxToShift <= 0)
        return;
    for (idx = gTimersListSize; idx >= lastIdxToShift; --idx)
        gTimersList[idx] = gTimersList[idx - 1];
}

void shiftTimersListOneStepBackward(int toIdx)
{
    int idx;
    if (toIdx < 0)
        return;
    for(idx = toIdx; idx < (gTimersListSize - 1); idx++)
        gTimersList[idx] = gTimersList[idx + 1];
    /*!
     * Note: The last element at [idx] position will
     * be duplicated of what at position [idx - 1] since we loop
     * until gTimerListSize - 1 to avoid transfer garbage/ or access
     * an unauthorized memory place to position [idx].
     * 
     * Don't decrease the array size, let it for the developer when (outside)
     * to decrease it.
     */
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