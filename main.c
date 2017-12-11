#include <stdio.h>

#include <vTAL.h>

int main()
{
    VTAL_tstrConfig t1,t2,t3,t4,t5,t6,t7,t8,t9;

    t1.timerID = TIMER_ID_1;
    t1.expiredTime.seconds = 0;
    t1.expiredTime.milliseconds = 500;

    t2.timerID = TIMER_ID_2;
    t2.expiredTime.seconds = 0;
    t2.expiredTime.milliseconds = 400;

    t3.timerID = TIMER_ID_3;
    t3.expiredTime.seconds = 0;
    t3.expiredTime.milliseconds = 1000;

    t4.timerID = TIMER_ID_4;
    t4.expiredTime.seconds = 0;
    t4.expiredTime.milliseconds = 100;

    t5.timerID = TIMER_ID_5;
    t5.expiredTime.seconds = 0;
    t5.expiredTime.milliseconds = 150;

    t6.timerID = TIMER_ID_6;
    t6.expiredTime.seconds = 0;
    t6.expiredTime.milliseconds = 1000;

    t7.timerID = TIMER_ID_7;
    t7.expiredTime.seconds = 0;
    t7.expiredTime.milliseconds = 400;

    t8.timerID = TIMER_ID_8;
    t8.expiredTime.seconds = 0;
    t8.expiredTime.milliseconds = 1700;

    t9.timerID = TIMER_ID_9;
    t9.expiredTime.seconds = 0;
    t9.expiredTime.milliseconds = 300;

    printf("Hello vTAL\n");

    VTAL_addTimer(&t1);
#ifdef __VTAL_DEBUG__
        VTAL_showTimerList();
#endif
    VTAL_addTimer(&t2);
#ifdef __VTAL_DEBUG__
        VTAL_showTimerList();
#endif
    VTAL_addTimer(&t3);
#ifdef __VTAL_DEBUG__
        VTAL_showTimerList();
#endif
    VTAL_addTimer(&t4);
#ifdef __VTAL_DEBUG__
        VTAL_showTimerList();
#endif
    VTAL_addTimer(&t5);
#ifdef __VTAL_DEBUG__
        VTAL_showTimerList();
#endif
    VTAL_addTimer(&t6);
#ifdef __VTAL_DEBUG__
        VTAL_showTimerList();
#endif
    VTAL_addTimer(&t7);
#ifdef __VTAL_DEBUG__
        VTAL_showTimerList();
#endif
      VTAL_addTimer(&t8);
#ifdef __VTAL_DEBUG__
        VTAL_showTimerList();
#endif

      VTAL_addTimer(&t9);
#ifdef __VTAL_DEBUG__
        VTAL_showTimerList();
#endif

        /* final output:
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
        return 0;
}