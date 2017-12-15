#include <stdio.h>

#include <vTAL.h>

int testRes = 0x0;

void t1Event(void* arg);
void t2Event(void* arg);
void t3Event(void* arg);
void t4Event(void* arg);
void t5Event(void* arg);
void t6Event(void* arg);
void t7Event(void* arg);
void t8Event(void* arg);
void t9Event(void* arg);


int main()
{
    VTAL_tstrConfig t1,t2,t3,t4,t5,t6,t7,t8,t9;

    t1.timerID = TIMER_ID_1;
    t1.expiredTimeEvent = t1Event;
    t1.timerMode = VTAL_ONE_SHOT_TIMER;
    t1.expiredTime.seconds = 0;
    t1.expiredTime.milliseconds = 500;

    t2.timerID = TIMER_ID_2;
    t2.expiredTimeEvent = t2Event;
    t2.timerMode = VTAL_ONE_SHOT_TIMER;
    t2.expiredTime.seconds = 0;
    t2.expiredTime.milliseconds = 400;

    t3.timerID = TIMER_ID_3;
    t3.expiredTimeEvent = t3Event;
    t3.timerMode = VTAL_ONE_SHOT_TIMER;
    t3.expiredTime.seconds = 0;
    t3.expiredTime.milliseconds = 1000;

    t4.timerID = TIMER_ID_4;
    t4.expiredTimeEvent = t4Event;
    t4.timerMode = VTAL_ONE_SHOT_TIMER;
    t4.expiredTime.seconds = 0;
    t4.expiredTime.milliseconds = 100;

    t5.timerID = TIMER_ID_5;
    t5.expiredTimeEvent = t5Event;
    t5.timerMode = VTAL_ONE_SHOT_TIMER;
    t5.expiredTime.seconds = 0;
    t5.expiredTime.milliseconds = 150;

    t6.timerID = TIMER_ID_6;
    t6.expiredTimeEvent = t6Event;
    t6.timerMode = VTAL_ONE_SHOT_TIMER;
    t6.expiredTime.seconds = 0;
    t6.expiredTime.milliseconds = 1000;

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

    printf("Hello vTAL\n");

    VTAL_addTimer(&t1);
/*#ifdef __DEBUG__
        VTAL_showTimerList();
#endif */
    VTAL_addTimer(&t2);
/*#ifdef __DEBUG__
        VTAL_showTimerList();
#endif */
    VTAL_addTimer(&t3);
/*#ifdef __DEBUG__
        VTAL_showTimerList();
#endif */
    VTAL_addTimer(&t4);
/*#ifdef __DEBUG__
        VTAL_showTimerList();
#endif */
    VTAL_addTimer(&t5);
/*#ifdef __DEBUG__
        VTAL_showTimerList();
#endif */
    VTAL_addTimer(&t6);
/*#ifdef __DEBUG__
        VTAL_showTimerList();
#endif */
    VTAL_addTimer(&t7);
/*#ifdef __DEBUG__
        VTAL_showTimerList();
#endif */
    VTAL_addTimer(&t8);
/*#ifdef __DEBUG__
        VTAL_showTimerList();
#endif */

    VTAL_addTimer(&t9);
#ifdef __DEBUG__
        VTAL_showTimerList();
#endif
    
    /* final correct output before remove:
        T_abs = 1700
        T_rel[0] = 100, ( id = 4 )
        T_rel[1] = 50, ( id = 5 )
        T_rel[2] = 150, ( id = 9 )
        T_rel[3] = 100, ( id = 2 )
        T_rel[4] = 0, ( id = 7 )
        T_rel[5] = 100, ( id = 1 )
        T_rel[6] = 500, ( id = 3 )
        T_rel[7] = 0, ( id = 6 )
        T_rel[8] = 700, ( id = 8 )
    */
    VTAL_removeTimer(t3.timerID);
#ifdef __DEBUG__
        VTAL_showTimerList();
#endif
     /* final correct output after remove:
        T_abs = 1700
        T_rel[0] = 100, ( id = 4 )
        T_rel[1] = 50, ( id = 5 )
        T_rel[2] = 150, ( id = 9 )
        T_rel[3] = 100, ( id = 2 )
        T_rel[4] = 0, ( id = 7 )
        T_rel[5] = 100, ( id = 1 )
        T_rel[6] = 500, ( id = 6 )
        T_rel[7] = 700, ( id = 8 )
    */
#ifdef __DEBUG__
        VTAL_showTimerList();
#endif
    while(testRes != 0x03F6);
    printf("Done\n");
        return 0;
}

void t1Event(void *arg)
{
    testRes |= (1 << 1);
    printf("T1 event\n");
}

void t2Event(void *arg)
{
    testRes |= (1 << 2);
    printf("T2 event\n");
}

void t3Event(void *arg)
{
    testRes |= (1 << 3);
    printf("T3 event\n");
}

void t4Event(void *arg)
{
    testRes |= (1 << 4);
    printf("T4 event\n");
}

void t5Event(void *arg)
{
    testRes |= (1 << 5);
    printf("T5 event\n");
}

void t6Event(void *arg)
{
    testRes |= (1 << 6);
    printf("T6 event\n");
}

void t7Event(void *arg)
{
    testRes |= (1 << 7);
    printf("T7 event\n");
}

void t8Event(void *arg)
{
    testRes |= (1 << 8);
    printf("T8 event\n");
}

void t9Event(void *arg)
{
    testRes |= (1 << 9);
    printf("T9 event\n");
}
