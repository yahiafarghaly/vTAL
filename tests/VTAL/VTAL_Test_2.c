#include <stdio.h>

#include <vTAL.h>

int testFlag = 0x0;

void t9Event(void *arg);

int main()
{
    VTAL_tstrConfig t9;

    t9.timerID = TIMER_ID_9;
    t9.expiredTimeEvent = t9Event;
    t9.timerMode = VTAL_PERIODIC_TIMER;
    t9.expiredTime.seconds = 0;
    t9.expiredTime.milliseconds = 0;

    printf("VTAL Test#2: Start\n");
    VTAL_addTimer(&t9);
    while (testFlag != 0x09)
        ;
    printf("VTAL Test#2: Done\n");

    return 0;
}

void t9Event(void *arg)
{
    testFlag += 0x01;
    printf("T9 event\n");
}