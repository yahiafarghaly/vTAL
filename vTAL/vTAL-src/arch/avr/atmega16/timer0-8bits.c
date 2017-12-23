/****************************************************************************/
/*    FILE: timer0-8bits.c                                                  */
/*                                                                          */
/*    This file is the implementation of the hardware timer 0 in atmega16   */
/*    or atmega32 MCUs. This timer is 8-bit width.                          */
/*                                                                          */
/*    This file is compiled with avr-gcc version 4.7.2. More explicitly,	*/
/*    with the gcc toolchain shipped with atmel studio.                     */
/*                                                                          */
/*                                                                          */
/*    BY:   Yahia Farghaly                                                  */
/*          23-Dec-2017	                                                    */
/*    Email: yahiafarghaly@gmail.com                                        */
/*                                                                          */
/****************************************************************************/

#include "../../../HTAL.h"

#if defined (__AVR_ATmega16__) || defined(__AVR_ATmega32__)

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
#include <avr/interrupt.h>

/*	Private Variables	*/
static void (*gUserTimerCallBack)(void *);
static void* gUserTimerCallbackArg;

static long gNumberOf_1ms_passed = 0;
static long gtimePeriodMilliSec = 0;


void HTAL_PhysicalTimerInit(void)
{
	/*!
	 *	FOCO = 1; To ensure non-PWM mode (Timer mode).
	 *	WGM01 = 1; To set to compare mode. OCR0 contains the compared value against TCNT0
	 *	Other bits are zero for normal OC0 port operation (i.e OC0 disconnected).
	 *	By setting CS02, CS01, CS00 to 101, we select a Pre-scalar of 1KHz.
	 *
	 *  Since This code works under 1MHz, the timer clock source becomes 1KHz.
	 *  And the 1 tick takes 1 ms which is the lowest time can be provided by VTAL.
	 *
	 *	Recap: why we set a Pre-scalar value in the first place ?
	 *  Since this timer is 8-bit, so if we leave it without any pre-scalar (i.e 1 MHz),
	 *  the timer will overflow in 0xFF * (1us) = 255 us, so the timer will interrupt around 3921 times
	 *  in a second which will not leave a chance for the application code to execute some thing has a meaning.
	 *
	 * So, in this code, our base timing is 1ms for each tick.
	 **/
	TCCR0 = (1<<FOC0)|(1 << WGM01)|(1<<CS02)|(1<<CS00);
}

void HTAL_startPhysicalTimer(long timePeriodMilliSec,
                             void (*userTimerCallBack)(void *),
                             void *userTimerCallbackArg)
{
	gtimePeriodMilliSec		= timePeriodMilliSec;
	gUserTimerCallBack		= userTimerCallBack;
	gUserTimerCallbackArg	= userTimerCallbackArg;

	/* Setting the initial value for the timer to zero.	*/
	TCNT0 = 0x00;
	/* Enable global interrupt (bit:7 in SREG).*/
	sei();
	/*
	 * We will set the compare value to 1 tick, since it will overflow after 1 ms
	 * which gives us the capability to count up a time greater > 1 ms by the variable (gNumberOf_1ms_passed)
	 */
	OCR0 = 0x01;
	/* Enable the compare match interrupt bit mask.
     * When we set the OCIE0 bit and I bit (global interrupt), the interrupt for this timer is enabled.
     */
	TIMSK |= (1 << OCIE0);
}

void HTAL_stopPhysicalTimer(void)
{
	/* Disable the interrupt for this timer.	*/
	TIMSK &= ~(1 << OCIE0);
}



long HTAL_remainingTime(void)
{
    return (gtimePeriodMilliSec - gNumberOf_1ms_passed);
}

void HTAL_changeUserTimerCallBack(void (*userTimerCallBack)(void *),
                                  void *userTimerCallbackArg)
{
    gUserTimerCallBack = userTimerCallBack;
    gUserTimerCallbackArg = userTimerCallbackArg;
}

/*!
 * In AVR architecture,
 * When an interrupt occurs, the Global Interrupt Enable I-bit is cleared and all interrupts are disabled.
 * The user software can write logic one to the I-bit to enable nested interrupts. The I-bit is automatically set when a
 * Return from Interrupt instruction - RETI - is executed. Also the interrupt Flag of this handler is cleared automatically
 * upon interrupt return.
 * 
 * Here we disable the nested interrupts.
*/
ISR(TIMER0_COMP_vect)
{

	++gNumberOf_1ms_passed;
	if (gtimePeriodMilliSec == gNumberOf_1ms_passed)
	{
		/* Stop counting.*/
		HTAL_stopPhysicalTimer();
		gNumberOf_1ms_passed = 0;
		/* Call the user timer callback */
		if (gUserTimerCallBack != (void *)0)
			gUserTimerCallBack(gUserTimerCallbackArg);

		/*Notify VTAL that timer is timedout*/
		HTAL_notifyTimeoutToVTAL();
	}
}

#endif /*	__AVR_ATmega16__ or __AVR_ATmega32__	*/
