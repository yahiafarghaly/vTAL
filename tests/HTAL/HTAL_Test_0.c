#include <stdio.h>
#include "vTAL-src/vTAL_low_level_timer_arch.h"

int exitKey = 0;
void timerEvent(void* arg)
{
    printf("Timer event is called\n");
    exitKey = 1;
}

int main()
{
    long rem = 0x01;
    int i;
    HTAL_updateVirtualTimersList(NULL);
    HTAL_startPhysicalTimer(5000,timerEvent,NULL);

    while(rem)
    {
        rem = HTAL_remainingTime();
        printf("Remaining time = %d\n",rem);
        for(i = 0; i < 1000*1000*1000;i++);
    }

    while(exitKey == 0);

    return 0;
}
