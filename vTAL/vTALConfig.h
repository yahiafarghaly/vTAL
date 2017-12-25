#ifndef _VTAL_CONFIG_H_
#define _VTAL_CONFIG_H_
/****************************************************************************/
/*    FILE: vTAL.h                                                          */
/*                                                                          */
/*    Stack size of how many virtual timers the application software can    */
/*    use.                                                                  */
/*                                                                          */
/*    BY:   Yahia Farghaly                                                  */
/*          25-Dec-2017	                                                    */
/*    Email: yahiafarghaly@gmail.com                                        */
/*                                                                          */
/****************************************************************************/

/*  define the suitable type for your application   */
typedef unsigned short      VTAL_tTimerId;
/*  Must be signed data types(i.e: char, short, long) for internal operations of vTAL and HTAL*/
typedef long       VTAL_tTimeSec;
typedef long       VTAL_tTimeMilliSec;

#define NUMBER_OF_TIMERS    (10u)
    /*!  
     *  TIMER_ID_<ID_Number> 
     * The <ID_Number> is not the index of the timer inside vTAL. it just for which timer 
     * is operating right now.
     */
    #define TIMER_ID_0      0
    #define TIMER_ID_1      1
    #define TIMER_ID_2      2
    #define TIMER_ID_3      3
    #define TIMER_ID_4      4
    #define TIMER_ID_5      5
    #define TIMER_ID_6      6
    #define TIMER_ID_7      7
    #define TIMER_ID_8      8
    #define TIMER_ID_9      9

#endif /*   _VTAL_CONFIG_H_ */