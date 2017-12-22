/****************************************************************************/
/*    FILE: WTimerA0.c                                                      */
/*                                                                          */
/*    This file is the implementation of the one of the hardware timers in  */
/*    Stellaris LM4F120XL board.                                            */
/*    This timer is the wide timer 0 (32-bit), subtimer A.                  */
/*                                                                          */
/*    To use this driver, you need to export the following interrupt        */
/*    handlers in the compiler startup code:                                */
/*    WideTimer0A_Handler                                                   */
/*                                                                          */
/*    BY:   Yahia Farghaly                                                  */
/*          22-Dec-2017	                                                    */
/*    Email: yahiafarghaly@gmail.com                                        */
/*                                                                          */
/****************************************************************************/

#include "../../../HTAL.h"
#include "lm4f120h5qr.h"

/*!
 * Changing this value will reflect in HTAL_startPhysicalTimer() automatically
 * and affect HTAL_remainingTime() in not automatic way since it depends on number 16.
 */
#define CLK_SRC_MHZ 16

/*!
 * Every compiler has its way to write the assembly mixed with the C code.
 * The following syntex is for the ARM Compiler shipped with uVision Keil.
 * ARM Compiler v4.1 [build 894],uVision 4.54
 * 
 * */

#ifdef rvmdk /*This is defined in this keil project pre-processor macros */
__asm void
EnableInterrupts(void)
{
    CPSIE 	I;  /* Set I=0, the bit in PRIMASK register  */
    BX 		LR  /* Return from this routine*/
}
#else
    #error "EnableInterrupts() is not defined for this compiler syntex rules."
#endif



void (*gUserTimerCallBack)(void *);
void* gUserTimerCallbackArg;


void HTAL_PhysicalTimerInit(void)
{
    long tmp;
    /*!
     *  First thing to do is setting the clock source of your timer, since 
     *  it depends on the system clock, the Precision internal oscillator (aka PIOSC)
     *  which normally it is operating at 16 MHz is chosen for simplicity. 
     * */
    SYSCTL_RCC_R |= (SYSCTL_RCC_BYPASS | SYSCTL_RCC_OSCSRC_INT);
    /*Waste some system clocks till it stabilizes*/
    tmp = SYSCTL_RCC_R;

    /* Enable the 32/64-Bit Wide General-Purpose Timer 0 Run Mode.  */
    SYSCTL_RCGCWTIMER_R |= SYSCTL_RCGCWTIMER_R0;
    /* As per datasheet, Waste some system clocks before accessing the timer registers. */
    tmp = SYSCTL_RCGCWTIMER_R;
    /*!
     *  For a 32/64-bit wide timer, this value selects 
     *  the 32-bit timer configuration which means only Timer A or
     *  Timer B can be used.
     * */
    WTIMER0_CFG_R |= TIMER_CFG_16_BIT;
    /*!
     * Configure one single shot timer with down counting direction
     * and An interrupt is generated when the match value in the
     * GPTMTAMATCHR register is reached.
     * */
    WTIMER0_TAMR_R |= (TIMER_TAMR_TAMR_1_SHOT | TIMER_TAMR_TAMIE);

    /*  GPTM Timer A Match Interrupt Mask enable    */
    WTIMER0_IMR_R |= TIMER_IMR_TAMIM;

    /*!
     * Enable timer interrupt handler in NVIC table.
     * As per datasheet, the IRQ of WTimer0A is (94) and Vector interrupt number is 110
     * To get the corresponding enable register, we do integer division of (IRQ/32)
     * the reminder (IRQ%32) is the bit field position of WTimer0A enable.
     * 
     * Similary, the piority register for WTimer0A is calculated by (IRQ/4) where,
     * the location of bit fields of piority number is at [ 8*(IRQ%4)+7 , 8*(IRQ%4)+6, 8*(IRQ%4)+5 ] 
     * The Piority is set to zero to get a highly corresponding interrupt piority.
     * */
    NVIC_PRI23_R &= ~( (1<<23) | (1<<22) | (1<<21) );
    NVIC_EN2_R |= (1<<30);

    /*Enable the global interrupt for the ARM processor if it is not enabled. */
    EnableInterrupts();
}

void HTAL_startPhysicalTimer(long timePeriodMilliSec,
                             void (*userTimerCallBack)(void *),
                             void *userTimerCallbackArg)
{
    /*!
     * When the timer is counting down, this register is used to load
     * the starting count value into the timer.
     * 
     * Since we accept time in MilliSeconds, then ignore any float-point operation here
     * by removing 1000 from CLK SOURCE Value.
     * Original equation: Timer Register tick counter = F(MHz)*T(Second) */
    WTIMER0_TAILR_R = CLK_SRC_MHZ * 1000 * timePeriodMilliSec;

    /*!
     * This register is loaded with a match value which an interrupt can be generated 
     * when the timer value is equal to the value in this register in one-shot mode.
     * Since the timer is counting down, the matched value is 0
     */
    WTIMER0_TAMATCHR_R = 0;

    /*Assign user timer Callback.*/
    gUserTimerCallBack = userTimerCallBack;
    gUserTimerCallbackArg = userTimerCallbackArg;
    
    /*Finally, We start timer by enable its flag. (Timer A)*/
    WTIMER0_CTL_R |= TIMER_CTL_TAEN;
}

void HTAL_stopPhysicalTimer(void)
{
    /*This stops the timer without losing the configuration in HTAL_PhysicalTimerInit()*/
    WTIMER0_CTL_R &= ~(TIMER_CTL_TAEN);
}

/*!
 * Fast algorthim to divide by a const integer (in our case, it is 1000)
 * Source: Hacker's Delight book, Chapter 10, FIGURE 10–13. Unsigned divide by 1000.
 * (http://www.hackersdelight.org/divcMore.pdf)
*/
unsigned long divu1000(unsigned long n) {
	unsigned long q, r, t;
	t = (n >> 7) + (n >> 8) + (n >> 12);
	q = (n >> 1) + t + (n >> 15) + (t >> 11) + (t >> 14);
	q = q >> 9;
	r = n - q * 1000;
	return q + ((r + 24) >> 10);
}

long HTAL_remainingTime(void)
{
    /*!
     * Since i run the timer on match mode, the WTIMER0_TAV_R will contain 
     * the remaining ticks till the timeout.
     * 
     * Here are the benefit of using Matcher Interrupt rather than the time-out interrupt event.
     * Since time-out interrupt will require setting the timer to be up-counting and substract from the user 
     * provided time to get the remaining ticks while using WTIMER0_TAV_R with match interrupt gives me 
     * directly the remaining ticks at WTIMER0_TAV_R.
     * 
     * To Convert the remaining ticks to remaining time, you only need to do 
     * Time(ms) = (Timer Tick Register Value)/16*1000 ,where 16 is the clock source for this timer. 
     * Instead of doing a software division which will take a big number of cycles, we optimize it
     * by simple 4 right shift and call the result on Hacker's Delight algorithm for unsigned integer division by constant.
     *  
     **/
    return divu1000(WTIMER0_TAV_R >> 4);
}

void HTAL_changeUserTimerCallBack(void (*userTimerCallBack)(void *),
                                  void *userTimerCallbackArg)
{
    gUserTimerCallBack = userTimerCallBack;
    gUserTimerCallbackArg = userTimerCallbackArg;
}

void WideTimer0A_Handler(void)
{
  /*!
   * Acknowledge that interrupt is recieved, so clear interrupt status in
   * both in the GPTMRIS register and the TAMMIS bit in the GPTMMIS register.
   * Also, By default the TAEN bit in GPTMCTL is cleared for one shot timer 
   * mode after clearing the status bit.*/
    WTIMER0_ICR_R |= TIMER_ICR_TAMCINT;
    /* Call the user timer callback */
    if (gUserTimerCallBack != (void*)0)
        gUserTimerCallBack(gUserTimerCallbackArg);

    /*Notify VTAL that timer is timedout*/
    HTAL_notifyTimeoutToVTAL();
}
