#include "../../HTAL.h"

#ifdef __linux__    /* To Prevent Compiling it if in another arch   */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <signal.h>

void (*gUserTimerCallBack)(void *);
void* gUserTimerCallbackArg;

struct itimerval timerVal;
struct sigaction TimerSignalHandler, oldTimerSignalHandler;

static void HTAL_linux_TimerHandler(int);

void HTAL_PhysicalTimerInit(void)
{
    return;
}

void HTAL_startPhysicalTimer(long timePeriodMilliSec,
                             void (*userTimerCallBack)(void *),
                             void* userTimerCallbackArg)
{

    /*! 
     * The user should not supply a zero timeout and it doesn't make sense 
     * to handle it. But, a simple handling will not harm beside this is
     * optional implementation choice since zero timeout is handled from
     * VTAL anyway.
     */
    if(timePeriodMilliSec <= 0)
    {
        gUserTimerCallBack = userTimerCallBack;
        gUserTimerCallbackArg = userTimerCallbackArg;
        HTAL_linux_TimerHandler(0);
        return;
    }

    timerVal.it_interval.tv_sec = 0;
    timerVal.it_interval.tv_usec = 0;
    timerVal.it_value.tv_sec = timePeriodMilliSec / 1000;
    timerVal.it_value.tv_usec = (timePeriodMilliSec % 1000) * 1000;
    errno = 0;
    TimerSignalHandler.sa_handler = &HTAL_linux_TimerHandler;
    TimerSignalHandler.sa_flags = SA_NOMASK;
    gUserTimerCallBack = userTimerCallBack;
    gUserTimerCallbackArg = userTimerCallbackArg;

    if (setitimer(ITIMER_REAL, &timerVal, NULL) != 0)
    {
        printf("setitimer() error = %d\n", errno);
        exit(1);
    }

    if (sigaction(SIGALRM, &TimerSignalHandler, &oldTimerSignalHandler) != 0)
    {
        printf("sigaction() error = %d\n", errno);
        exit(1);
    }
}

void HTAL_stopPhysicalTimer(void)
{
  timerVal.it_interval.tv_sec = 0;
  timerVal.it_interval.tv_usec = 0;
  timerVal.it_value.tv_sec = 0;
  timerVal.it_value.tv_usec = 0;
  setitimer(ITIMER_REAL, &timerVal, NULL);
  sigaction(SIGALRM, &oldTimerSignalHandler, NULL);
}

void HTAL_linux_TimerHandler(int signal_num)
{
    if(gUserTimerCallBack != NULL)
        gUserTimerCallBack(gUserTimerCallbackArg);

    HTAL_notifyTimeoutToVTAL();
}

void HTAL_changeUserTimerCallBack(void (*userTimerCallBack)(void *),
                                  void *userTimerCallbackArg)
{
    gUserTimerCallBack = userTimerCallBack;
    gUserTimerCallbackArg = userTimerCallbackArg;
}

long HTAL_remainingTime(void)
{
  struct itimerval currTimerVal;
  getitimer(ITIMER_REAL, &currTimerVal);
  
  return  (currTimerVal.it_value.tv_sec*1000) + ((currTimerVal.it_value.tv_usec)/1000);
}

#endif  /*   __linux__   */