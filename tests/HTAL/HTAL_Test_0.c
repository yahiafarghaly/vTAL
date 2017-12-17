#include <stdio.h>
#include "vTAL-src/HTAL.h"

int exitKey = 0;
void timerEvent(void* arg)
{
    printf("Timer event is called\n");
    exitKey = 1;
}

int main()
{   /*
     * In this test:
     *  After the event is triggered, a SEGFAULT occured and this is due to
     * the recusive calls happens accidentally in updateTimersList() when the
     * memory of the first element of the internal array is zero.
     * it is an easy fix, instead of doing (gTimersListSize == 0), making
     *   (gTimersListSize <= 0) will fix it. but i perfer leaving it comparing
     * it to zero since it is a bit faster than inequality. Also, it is never an
     * exist case when VTAL is operating the gTimersListSize will be less than
     * zero when facing the above condition.
     * */
    long rem = 0x01;
    int i;
    HTAL_startPhysicalTimer(5000,timerEvent,NULL);

    while(rem)
    {
        rem = HTAL_remainingTime();
        printf("Remaining time = %li\n",rem);
        for(i = 0; i < 1000*1000*1000;i++);
    }

    while(exitKey == 0);

    return 0;
}
