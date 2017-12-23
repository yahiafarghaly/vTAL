/*
 * vTAL_atmelStudio.c
 *
 * Created: 12/23/2017 12:48:34 PM
 *  Author: Yahia
 */ 

#ifndef F_CPU
	/*!
	 * Stick With the default shipped frequency
	 * 1MHz Clock frequency, Review the fuse bits while using the flashing tool.
	 * The fuse bits options are mentioned at Table.9 Page 29, atmega16 data sheet.
	 *
	 *	Must called before any include of Atmel files.
	 */
	#define F_CPU 1000000UL 
#endif

#include <avr/io.h>
#include <vTAL.h>


/* Turn on some lights	*/
#define TURN_ON_RED() 		PORTA|=(1<<PA0);
#define TURN_ON_BLUE() 		PORTA|=(1<<PA2);
#define TURN_ON_GREEN() 	PORTA|=(1<<PA1);

/* Turn off	*/
#define TURN_OFF_RED()		PORTA&=~(1<<PA0);
#define TURN_OFF_BLUE()		PORTA&=~(1<<PA2);
#define TURN_OFF_GREEN()	PORTA&=~(1<<PA1);


void LEDsInit()
{
	/*Enable 3 ports to be output.*/
	DDRA|=(1<<PA0);
	DDRA|=(1<<PA1);
	DDRA|=(1<<PA2);
}

/*	Timers events	*/
void t1Event(VTAL_tCallBackArg arg);
void t2Event(VTAL_tCallBackArg arg);
void t3Event(VTAL_tCallBackArg arg);

volatile unsigned long flag = 0;

int main(void)
{
    VTAL_tstrConfig t1,t2,t3;
    
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
