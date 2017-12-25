# HTAL Implementation Guide

VTAL expects very specific functionalities from HTAL so it can operate and function properly.
Before going to implement HTAL for some MCU, you should first think about some constrains:

- VTAL is always provide HTAL with a time in unit of miliseconds. And the lowest value is 1 ms.
- The timer width in bits and how it will affect the way you do the correct timing efficiently.
- Last and most important, know what is the frequency that your timer will work with.

### For the timer width constrain:

The time register value for a give user time (T) in second is calculated by (TimeRegValue = TickTime*T)

if T = 1 ms (The smallest value) and TimeRegValue will overflow before T, then in the interrupt handler of
the timer you will need to add a simple counter for number of overflows to get the correct time T. or set
the timer tick to be 1 ms as shown in [atmega16](vTAL/vTAL-src/arch/avr/atmega16/timer0-8bits.c) example using an 8-bit timer to get the correct user timing with another variable inside the timer handler. 

In case of a larger timer width such as 32-bit, you may ignore doing as 8-bit case since the register value will 
possibly will overflow after the user giving time. 

for example in case timer register width is 32-bit. If the timer frequency is 16Mhz and T = 60 sec,then 
TimeRegValue = (1/16*10^6) * 50 = 960000000 which is less than 4294967295 (0xffffffff). But if your application
is excpecting to have a timer greater than 268 second, you may apply the same approach of 8-bit timer handler on 32-bit timer handler.

Another approach of increasing the time period will the TimeRegValue will overflow at is using timer pre-scalars. The higher pre-scalar
value divided by the system clock frequency, the more time you can expend without reaching to the timer overflow.
For the example of 32-bit timer, if the pre-scalar is 1024, then maximum time will the TimeRegValue will overflow is 274877 seconds. 

It all depends on your application needs.

For applications running on **windows/linux**, you don't need all the above details to care about since you can use the OS APIs directly and HTAL/VTAL is acting as a nice wrapper for it.

### Pseudo Code

The following is a pseudo code for implementing each HTAL APIs correctly for an embedded environment.

**Remember**: Set the clock source of the system to the desired need.

#### HTAL_PhysicalTimerInit()
1 - Enable the timer mode, disable any other modes.
2 - Enable one shot mode timer feature. (this may be seen as compare mode in some timers).
3 - Enable the timer interrupt in the interrupt vector table if neccessary.
4 - Set the inital value for the timer register counter.
5 - A prescalar value to the timer frequency if necessary.

#### HTAL_startPhysicalTimer()
1 -  You set the initial and final values for the timer. you also will convert the user time in millisecond to a tick count value in one of the timer registers.
2 - You save the user callback and callback arguments to a later call in the timer handler.
3 - You active the global interrupt of the system and the enable bit of the timer if exist.
4 - At that time the timer shall start counting whether up or down according to your design decision. Also be aware to **not disable** the global interrupt of the system till the user's timer callback is called.

#### HTAL_stopPhysicalTimer()
1 - You shall stop the timer counting by disabling its enable bit or timer interrupt enable mask bit.
2 - You **should not** disable gobal interrupt at this call. if you want, it is preferred to do it outside .

#### HTAL_remainingTime()
1 - You should retain the remaining time of an active timer in milliseconds before the timer time out. You may have to convert the remaining ticks into a remaining timeout.

#### HTAL_changeUserTimerCallBack()
1 - Just change the user's timer callback and its argument whether the timer is active, timeouted or dis-active.

#### HTAL_notifyTimeoutToVTAL()
You don't need to implement it, just call this method at the end of the timer handler(aka ISR).

### Interrupt Service Routine of the timer.
This ISR should be called automatically by processor the interrupt event is triggered. Whether this timer trigger is caused due to timer compare event or timer timeout event.
1 - Clear the timer flag to ackwnoledge that the timer event is reconginzed.
2 - Call the user's timer callback with user argument.
3 - Finally, don't forget to call ***HTAL_notifyTimeoutToVTAL()*** so VTAL can start the timer for the second timer in the timer list.

That's all.
