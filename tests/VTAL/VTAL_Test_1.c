#include <stdio.h>

#include <vTAL.h>

int testSingleShot = 0x0;

void t7Event(void *arg);
void t8Event(void *arg);
void t9Event(void *arg);

int main()
{
    VTAL_tstrConfig t7, t8, t9;

    t7.timerID = TIMER_ID_7;
    t7.expiredTimeEvent = t7Event;
    t7.timerMode = VTAL_ONE_SHOT_TIMER;
    t7.expiredTime.seconds = 0;
    t7.expiredTime.milliseconds = 400;

    t8.timerID = TIMER_ID_8;
    t8.expiredTimeEvent = t8Event;
    t8.timerMode = VTAL_ONE_SHOT_TIMER;
    t8.expiredTime.seconds = 0;
    t8.expiredTime.milliseconds = 1700;

    t9.timerID = TIMER_ID_9;
    t9.expiredTimeEvent = t9Event;
    t9.timerMode = VTAL_ONE_SHOT_TIMER;
    t9.expiredTime.seconds = 0;
    t9.expiredTime.milliseconds = 300;

    printf("VTAL Test#1: Start\n");

    VTAL_addTimer(&t7);
#ifdef __DEBUG__
    VTAL_showTimerList();
#endif
    VTAL_addTimer(&t8);
#ifdef __DEBUG__
    VTAL_showTimerList();
#endif
    VTAL_addTimer(&t9);
#ifdef __DEBUG__
    VTAL_showTimerList();
#endif

    while (testSingleShot != 0x0380)
        ;
    printf("VTAL Test#1: Done\n");

    return 0;
}

void t7Event(void *arg)
{
    testSingleShot |= (1 << 7);
    printf("T7 event\n");
}

void t8Event(void *arg)
{
    testSingleShot |= (1 << 8);
    printf("T8 event\n");
}

void t9Event(void *arg)
{
    testSingleShot |= (1 << 9);
    printf("T9 event\n");
}