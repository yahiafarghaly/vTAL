/****************************************************************************/
/*    FILE: VTAL_stellaris_Test.c                                           */
/*                                                                          */
/*    This software uses VTAL to execute 3 events at 3 different time.      */
/*    Each event is lighting a certain color of LED exist on stellaris      */
/*    board.                                                                */
/*    T1 is a periodic timer with t = 6 and it lights the RED Color.        */
/*    T2 is a one shot timer with t = 4 and it lights the GREEN Color.      */
/*    T3 is a one shot timer with t = 2 and it lights the BLUE Color.       */
/*                                                                          */
/*    T2,T3 are added to the VTAL list again once T1 event occurs.          */
/*    You should be able to see the RED Color for 2 seconds before T3 event */
/*    is fired again.                                                       */
/*    The example is designed where each event is fired periodicly.         */
/*                                                                          */
/*    BY:   Yahia Farghaly                                                  */
/*          22-Dec-2017	                                                    */
/*    Email: yahiafarghaly@gmail.com                                        */
/*                                                                          */
/****************************************************************************/

#include <vTAL.h>
#include "lm4f120h5qr.h"

/* Turn on some lights	*/
#define TURN_ON_RED() GPIO_PORTF_DATA_R |= 0x02;
#define TURN_ON_BLUE() GPIO_PORTF_DATA_R |= 0x04;
#define TURN_ON_GREEN() GPIO_PORTF_DATA_R |= 0x08;

/* Turn off	*/
#define TURN_OFF_RED() GPIO_PORTF_DATA_R &= ~(0x02);
#define TURN_OFF_BLUE() GPIO_PORTF_DATA_R &= ~(0x04);
#define TURN_OFF_GREEN() GPIO_PORTF_DATA_R &= ~(0x08);


void LEDsInit()
{
    /* Enable the GPIO port that is used for the on-board LED.*/
    SYSCTL_RCGC2_R = SYSCTL_RCGC2_GPIOF;
    /*! 
     * Enable the GPIO pins for the LEDs (PF3,PF2,PF1).  
     * Set the direction as output, and enable the GPIO
     * pin for digital function.
     */
    GPIO_PORTF_DIR_R = 0x0E;
    GPIO_PORTF_DEN_R = 0x0E;
}

void delay()
{
    volatile unsigned long ulLoop;
    for(ulLoop = 0; ulLoop < 1000*1000; ulLoop++);
}

/*	Timers events	*/
void t1Event(VTAL_tCallBackArg arg);
void t2Event(VTAL_tCallBackArg arg);
void t3Event(VTAL_tCallBackArg arg);

volatile unsigned long flag = 0;

int main()
{

    VTAL_tstrConfig t1,t2,t3;
	/*!
     *  First thing to do is setting the clock source of system. 
     *  The Precision internal oscillator (aka PIOSC) is chosen for simplicity.
     *  And it is operating at 16 MHz. 
     * */
    SYSCTL_RCC_R |= (SYSCTL_RCC_BYPASS | SYSCTL_RCC_OSCSRC_INT);
    /*Waste some system clocks till it stabilizes*/
    delay();
	
    LEDsInit();
    VTAL_init();
	
    t1.timerID = TIMER_ID_1;
    t1.expiredTimeEvent = t1Event;
    t1.eventContextInfo = (VTAL_tCallBackArg)0x01;
    t1.timerMode = VTAL_PERIODIC_TIMER;
    t1.expiredTime.seconds = 6;
    t1.expiredTime.milliseconds = 0;

    t2.timerID = TIMER_ID_2;
    t2.expiredTimeEvent = t2Event;
    t2.eventContextInfo = (VTAL_tCallBackArg)0x02;
    t2.timerMode = VTAL_ONE_SHOT_TIMER;
    t2.expiredTime.seconds = 4;
    t2.expiredTime.milliseconds = 0;

    t3.timerID = TIMER_ID_3;
    t3.expiredTimeEvent = t3Event;
    t3.eventContextInfo = (VTAL_tCallBackArg)0x04;
    t3.timerMode = VTAL_ONE_SHOT_TIMER;
    t3.expiredTime.seconds = 2;
    t3.expiredTime.milliseconds = 0;



    TURN_OFF_RED();
    TURN_OFF_GREEN();
    TURN_OFF_BLUE();

    VTAL_addTimer(&t1);
    VTAL_addTimer(&t2);
    VTAL_addTimer(&t3);


    for(;;)
    {
        if (flag == 0x07)
        {
            flag = 0;
            VTAL_addTimer(&t2);
            VTAL_addTimer(&t3);
        }
    }

	return 0;
}

void t1Event(VTAL_tCallBackArg arg)
{
    TURN_OFF_BLUE();
    TURN_OFF_GREEN();
    TURN_ON_RED();
    flag |= (unsigned long)arg;
}
void t2Event(VTAL_tCallBackArg arg)
{
    TURN_OFF_RED();
    TURN_OFF_BLUE();
    TURN_ON_GREEN();
    flag |= (unsigned long)arg;
}
void t3Event(VTAL_tCallBackArg arg)
{
    TURN_OFF_RED();
    TURN_OFF_GREEN();
    TURN_ON_BLUE();
    flag |= (unsigned long)arg;
}