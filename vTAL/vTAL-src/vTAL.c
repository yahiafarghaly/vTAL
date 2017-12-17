#include <vTAL.h>
#include "HTAL.h"

#define EMPTY_LIST      (-1)
#define EMPTY_CONFIG    ((void*)0)
#define NOT_FOUND       (-1)

typedef struct
{
    VTAL_tTimerId           timerID;
    VTAL_tenuTimerMode      timerMode;
    VTAL_tCALLBACK          expiredTimeEvent;
    VTAL_tCallBackArg       eventContextInfo;
    VTAL_tTimeMilliSec      relativeTimeoutmSec;
    VTAL_tTimeMilliSec      absoluteTimeoutmSec;
}VTAL_tstrTimerInfo;

static VTAL_tTimeMilliSec gAbsoulateTimeoutmSec = 0;
static long gTimersListSize = EMPTY_LIST;
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
static void shiftTimersListOneStepForward(const int lastIdxToShift);
static void shiftTimersListOneStepBackward(const int toIdx);
static int findTimer(VTAL_tTimerId timerID);

#ifdef __cplusplus
} /* extern "C" */
#endif


void VTAL_addTimer(VTAL_tstrConfig* VTAL_tpstrConfig)
{
    if (VTAL_tpstrConfig == EMPTY_CONFIG)
        return;

    VTAL_LOCK();
    if(gTimersListSize == NUMBER_OF_TIMERS)
    {
        VTAL_UNLOCK();
        return;
    }
    
    /* Search first if this Timer is already inserted.  */
    if(findTimer(VTAL_tpstrConfig->timerID) != NOT_FOUND)
    {
        VTAL_UNLOCK();
        return;
    }

    if (gTimersListSize == EMPTY_LIST)
    {
        gTimersList[0].absoluteTimeoutmSec =
            VTAL_tpstrConfig->expiredTime.milliseconds +
            (VTAL_tpstrConfig->expiredTime.seconds) * 1000;
        /* May be a use case leads to a timeout equals zero */
        if (gTimersList[0].absoluteTimeoutmSec <= 0)
        {
            if (VTAL_tpstrConfig->timerMode == VTAL_PERIODIC_TIMER)
            /*! Smallest time (1 ms) so, we don't get stuck of calling this
             * timer handler only ! and leave the others with no chance. */
                gTimersList[0].absoluteTimeoutmSec = 1;
            else
            {
                if(VTAL_tpstrConfig->expiredTimeEvent != (void*)0)
                    VTAL_tpstrConfig->expiredTimeEvent(VTAL_tpstrConfig->eventContextInfo);
                VTAL_UNLOCK();
                return;
            }
        }
        gTimersList[0].timerID = VTAL_tpstrConfig->timerID;
        gTimersList[0].expiredTimeEvent = VTAL_tpstrConfig->expiredTimeEvent;
        gTimersList[0].eventContextInfo = VTAL_tpstrConfig->eventContextInfo;
        gTimersList[0].timerMode = VTAL_tpstrConfig->timerMode;
        gTimersList[0].relativeTimeoutmSec = gTimersList[0].absoluteTimeoutmSec;

        gAbsoulateTimeoutmSec = gTimersList[0].absoluteTimeoutmSec;
        gTimersListSize = 1;
        /* start low level timer */
        HTAL_startPhysicalTimer(gAbsoulateTimeoutmSec,
                                gTimersList[0].expiredTimeEvent,
                                gTimersList[0].eventContextInfo);
        VTAL_UNLOCK();
    }
    else
    {
        VTAL_tTimeMilliSec newTimerRelativeTimeout;
        VTAL_tTimeMilliSec accumlatedRelativeTimes;
        VTAL_tTimeMilliSec firstTimerElapsedTime;
        unsigned long Idx;

        VTAL_tTimeMilliSec newTimerAbsTimeout =
            VTAL_tpstrConfig->expiredTime.milliseconds +
            (VTAL_tpstrConfig->expiredTime.seconds) * 1000;
        /*Don't rely that HTAL_startPhysicalTimer() handles the zero case*/
        if (newTimerAbsTimeout <= 0)
        {
            if (VTAL_tpstrConfig->timerMode == VTAL_PERIODIC_TIMER)
                newTimerAbsTimeout = 1;
            else
            {
                if(VTAL_tpstrConfig->expiredTimeEvent != (void*)0)
                    VTAL_tpstrConfig->expiredTimeEvent(VTAL_tpstrConfig->eventContextInfo);
                VTAL_UNLOCK();
                return;               
            }
        }

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
            gTimersList[gTimersListSize].eventContextInfo = VTAL_tpstrConfig->eventContextInfo;
            gTimersList[gTimersListSize].timerMode = VTAL_tpstrConfig->timerMode;
            gTimersList[gTimersListSize].relativeTimeoutmSec = newTimerRelativeTimeout;
            gTimersList[gTimersListSize].absoluteTimeoutmSec = newTimerAbsTimeout;
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
        firstTimerElapsedTime = 0;
        for (Idx = 0; Idx < gTimersListSize; ++Idx)
        {
            if ((newTimerAbsTimeout - accumlatedRelativeTimes) < gTimersList[Idx].relativeTimeoutmSec)
            {
                /*If first node is being changed, Change its 
                    user callback & timeout quickly @ lower level */
                if(Idx == 0)
                {
                    firstTimerElapsedTime = gTimersList[0].relativeTimeoutmSec - HTAL_remainingTime();
                    HTAL_stopPhysicalTimer();
                    HTAL_startPhysicalTimer(newTimerAbsTimeout,
                                            VTAL_tpstrConfig->expiredTimeEvent,
                                            VTAL_tpstrConfig->eventContextInfo);
                }
                shiftTimersListOneStepForward(Idx + 1);
                gTimersList[Idx].timerID = VTAL_tpstrConfig->timerID;
                gTimersList[Idx].expiredTimeEvent = VTAL_tpstrConfig->expiredTimeEvent;
                gTimersList[Idx].eventContextInfo = VTAL_tpstrConfig->eventContextInfo;
                gTimersList[Idx].timerMode = VTAL_tpstrConfig->timerMode;
                gTimersList[Idx].absoluteTimeoutmSec = newTimerAbsTimeout;
                gTimersList[Idx].relativeTimeoutmSec = newTimerAbsTimeout - accumlatedRelativeTimes;
                gTimersList[Idx + 1].relativeTimeoutmSec -= gTimersList[Idx].relativeTimeoutmSec;
                /*! 
                 * In case Idx is equal to 0, firstTimerElapsedTime has a value
                 * even zero (if time between two successive addTimer() call is
                 * zero(i.e very unrealistic case) ). Any way, the moved timer
                 * relative timeout @[1] must change since the physical timer 
                 * was timing for it. So the elapsed time has been calculated
                 * as above then we substracte this amount from @[1] timer and
                 * update the gAbsoulateTimeoutmSec to maintain a precise timing
                 * as possible. 
                 * */
                gTimersList[Idx + 1].relativeTimeoutmSec -= firstTimerElapsedTime;
                gAbsoulateTimeoutmSec -= firstTimerElapsedTime;

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
    int timerIdx;
    VTAL_LOCK();
    timerIdx = findTimer(timerID);
    if (timerIdx == NOT_FOUND)
    {
        VTAL_UNLOCK();
        return;
    }
    /*!
     * Add what will be removed to the next timer to maintain the same absolute
     * time and don't change other timers relative time. 
     * Change only the abs time in the case that this timerID 
     * is the last timer in the list.
     *  */
    if ((timerIdx + 1) == gTimersListSize)
    {
        gAbsoulateTimeoutmSec -= gTimersList[timerIdx].relativeTimeoutmSec;
        if (gTimersListSize == 1)
            HTAL_stopPhysicalTimer();
    }
    else
    {

        /* Change the timer callback before shifting when it is the first timer.*/
        if (timerIdx == 0)
        {
            VTAL_tTimeMilliSec firstTimerElapsedTime;
            firstTimerElapsedTime = gTimersList[0].relativeTimeoutmSec - HTAL_remainingTime();
            HTAL_stopPhysicalTimer();
            /*!
             * Ironically, I have to reverse the previous opertion since i need
             * ElapsedTime variable for decreasing the global absolute timeout. 
             */
            gTimersList[1].relativeTimeoutmSec += (gTimersList[0].relativeTimeoutmSec - firstTimerElapsedTime);
            gAbsoulateTimeoutmSec -= firstTimerElapsedTime;
            HTAL_startPhysicalTimer(gTimersList[1].relativeTimeoutmSec,
                                    gTimersList[1].expiredTimeEvent, gTimersList[1].eventContextInfo);
        }
        else
            gTimersList[timerIdx + 1].relativeTimeoutmSec += gTimersList[timerIdx].relativeTimeoutmSec;

        shiftTimersListOneStepBackward(timerIdx);
    }
    --gTimersListSize;
    if (gTimersListSize == 0)
        gTimersListSize = EMPTY_LIST;
    VTAL_UNLOCK();
}

void updateTimersList(void)
{
    /* ISO C90 forbids mixed declarations and code */ 
    VTAL_tstrTimerInfo firstTimer;

    VTAL_LOCK();
    firstTimer = gTimersList[0];
    
    gAbsoulateTimeoutmSec -= gTimersList[0].relativeTimeoutmSec;
    shiftTimersListOneStepBackward(0);
    --gTimersListSize;

    if (gTimersListSize == 0)
    {
        gTimersListSize = EMPTY_LIST;
    }
    else
    {
       /*!
        * When having multiple of Logical timers with the same timeout. By
        * design the next relative timeout will be zero, So we just execute 
        * user callback immediately.
        */
        if (gTimersList[0].relativeTimeoutmSec == 0)
        {
            if(gTimersList[0].expiredTimeEvent != (void*)0)
                gTimersList[0].expiredTimeEvent(gTimersList[0].eventContextInfo);
            VTAL_UNLOCK();
            updateTimersList();
        }
        else
        {
            HTAL_startPhysicalTimer(
                gTimersList[0].relativeTimeoutmSec,
                gTimersList[0].expiredTimeEvent,
                gTimersList[0].eventContextInfo);
        }
    }
    VTAL_UNLOCK();

    if(firstTimer.timerMode == VTAL_PERIODIC_TIMER)
    {
        VTAL_tstrConfig tmp;
        tmp.timerID = firstTimer.timerID;
        tmp.expiredTimeEvent = firstTimer.expiredTimeEvent;
        tmp.eventContextInfo = firstTimer.eventContextInfo;
        tmp.timerMode = firstTimer.timerMode;
        tmp.expiredTime.milliseconds = firstTimer.absoluteTimeoutmSec;
        tmp.expiredTime.seconds = 0;
        VTAL_addTimer(&tmp);
    }

#ifdef __DEBUG__
        VTAL_showTimerList();
#endif
}

void HTAL_notifyTimeoutToVTAL()
{
    updateTimersList();
}

void shiftTimersListOneStepForward(const int lastIdxToShift)
{
    int idx;
    if (lastIdxToShift <= 0)
        return;
    for (idx = gTimersListSize; idx >= lastIdxToShift; --idx)
        gTimersList[idx] = gTimersList[idx - 1];
}

void shiftTimersListOneStepBackward(const int toIdx)
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