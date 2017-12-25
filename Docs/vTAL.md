# vTAL (Virtual Timer Abstraction Layer)

### How it works ?
The design goal is to provide a way for executing multiple of timers simultaneously with a different or the same timeout with one physical timer is presented on the system.

To be able to achieve this design goal. we have to create a chain of the provided timers where each timer timeout is relative to the previous timer timeout. Simple as that... sure there are some corner cases which must be handled well. And this is provided with **vTAL** implementation.

Let's take an example, if we have 3 timers (T1,T2 and T3) where each timeout is **T1 = 500ms, T2 = 300ms and T3 = 50ms**. then the timer chain
would be something like this to achieve a correct timeout for each timer. 
```
T3->T2->T1
```
Assuming To is the reference time to be zero, then relative times will be **T3 = 50ms, T2= 250ms and T1 = 200ms** with a total absolute time from To equals to **500ms**. 

If a new timer T4 is being inserted with timeout 100ms, then the chain becomes.
```
T3(relative=50ms)->T4(rel=50ms)->T2(rel=200)->T1(rel=200) and absolute time remains 500ms
```
If another timer T5 is added with timeout = 1200ms,then the chain becomes.
```
T3(relative=50ms)->T4(rel=50ms)->T2(rel=200)->T1(rel=200)->T5(rel=700ms) and absolute time becomes 1200 ms
```
If another timer T6 with timeout = 10ms, then the chain becomes
```
T6(rel=10ms)->T3(relative=40ms)->T4(rel=50ms)->T2(rel=200)->T1(rel=200)->T5(rel=700ms) and absolute time remains 1200ms
```

This is the basic idea assuming that the **T3** is not started counting yet its timing. But if it is started then **T3** would have another relative time less 40ms because of the elapsed time of its timing when it was active. this elapsed time is handled in vTAL as well.

This is normally to be done with a linked list but since an embedded environment was the target of the first place then a similar behaviour of the linked list is made with an array of a fixed size which is configured by [vTALConfig.h](../vTAL/vTALConfig.h) file.

Association events with timers are possible also.

### vTAL structures and enums

#### VTAL_tenuTimerMode
```
typedef enum
{
    VTAL_ONE_SHOT_TIMER, /*The timer event is called only once then the timer is removed. */
    VTAL_PERIODIC_TIMER /*The event is called periodicly according to the timer timeout.*/
}VTAL_tenuTimerMode;
```

#### VTAL_tstrTimeSettings
```
typedef struct
{
    VTAL_tTimeSec       seconds;
    VTAL_tTimeMilliSec  milliseconds;
}VTAL_tstrTimeSettings; /*Available timer timeout units. */
```

#### VTAL_tstrConfig
```
typedef struct
{
    VTAL_tTimerId           timerID; /*The ID number mentioned in [vTALConfig.h](../vTAL/vTALConfig.h) */
    VTAL_tstrTimeSettings   expiredTime; /* Timer timeout */
    VTAL_tenuTimerMode      timerMode; /* Timer mode */
    VTAL_tCALLBACK          expiredTimeEvent; /*pointer to the user timer callback. */ 
    VTAL_tCallBackArg       eventContextInfo; /* pointer to the user timer callback arguments. */
}VTAL_tstrConfig; /*The configuration of a single timer */
```

The callback signature of the timer is 
```
void (*)(void*)
```

### vTAL Standard Interfaces

#### void VTAL_init(void)

- To prepare vTAL for inital things before adding timers such as initalize the hardware timer.
- Called only once.

#### void VTAL_addTimer(VTAL_tstrConfig* VTAL_tpstrConfig)

- Add a new timer to the chain of the timers.

#### void VTAL_removeTimer(VTAL_tTimerId timerID)

- Remove a timer from the timers chain if it is timeout not fired yet.

More details about APIs usage is commented well in the [vTAL.h](../vTAL/vTAL.h)

Have fun using it.