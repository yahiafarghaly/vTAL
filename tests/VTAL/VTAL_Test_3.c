#include <stdio.h>

#include <vTAL.h>

int testFlag = 0x0;

void t9Event(void *arg);
void t8Event(void *arg);

int main()
{
    VTAL_tstrConfig t8,t9;

    t9.timerID = TIMER_ID_9;
    t9.expiredTimeEvent = t9Event;
    t9.timerMode = VTAL_PERIODIC_TIMER;
    t9.expiredTime.seconds = 0;
    t9.expiredTime.milliseconds = 0;

    t8.timerID = TIMER_ID_8;
    t8.expiredTimeEvent = t8Event;
    t8.timerMode = VTAL_PERIODIC_TIMER;
    t8.expiredTime.seconds = 0;
    t8.expiredTime.milliseconds = 0;

    printf("VTAL Test#3: Start\n");
    VTAL_addTimer(&t9);
    VTAL_addTimer(&t8);
    while (testFlag !=  18)
        ;
    printf("VTAL Test#3: Done\n");

    return 0;
}

void t9Event(void *arg)
{
    testFlag += 0x01;
    printf("T9 event\n");
}

void t8Event(void *arg)
{
    testFlag += 0x01;
    printf("T8 event\n");
}