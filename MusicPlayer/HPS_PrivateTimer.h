/* 
 * HPS Private Timer Driver
 * ------------------------
 *
 *
 * This part of code is from UOL API
 *
 *
 */


#ifndef HPS_PRIVATEHPS_TIMER_H_
#define HPS_PRIVATEHPS_TIMER_H_

#include "Util/macros.h"
#include "Util/driver_timer.h"

#include "Util/error.h"

// Max Prescalar Value
#define HPS_TIMER_PRESCALE_MAX (0xFFU)

// Driver context
typedef struct {
    //Header
    DrvCtx_t header;
    //Body
    volatile unsigned int * base;
    
    // add any variables you need here (e.g. last prescalar)
    unsigned int loadValue;
    
    //Generic timer interface
    TimerMode mode;
    TimerCtx_t timer;
} HPSTimerCtx_t;

// Initialise Private Timer Driver
//  - base is a pointer to the private timer module
//  - Returns Util/error Code
//  - Returns context pointer to *ctx
HpsErr_t HPS_Timer_initialise(void* base, HPSTimerCtx_t** pCtx);

// Check if driver initialised
//  - Returns true if driver previously initialised
bool HPS_Timer_isInitialised(HPSTimerCtx_t* ctx);

// Configure the timer
//  - For one-shot mode, disables the timer. Call HPS_Timer_enable() to start timer.
//  - Otherwise timer will be enabled after configure only if it was enabled before
HpsErr_t HPS_Timer_configure(HPSTimerCtx_t* ctx, TimerMode mode,
                             unsigned int prescalar, unsigned int loadValue);


// Get the rounded timer clock rate
//  - Returns via *clockRate the timer clock rate based on configured prescalar
//  - If prescalar is UINT32_MAX, will return based on previously configured.
//    prescalar settings, otherwise will calculate for specified value
HpsErr_t HPS_Timer_getClockRate(HPSTimerCtx_t* ctx, unsigned int prescalar, 
                                unsigned int* clockRate);

// Get the current timer mode
//  - Returns the mode the timer was configured for
HpsErr_t HPS_Timer_getMode(HPSTimerCtx_t* ctx, TimerMode* mode);

// Get the load value for the timer
HpsErr_t HPS_Timer_getLoad(HPSTimerCtx_t* ctx, unsigned int* loadValue);

// Enable timer
//  - If fraction is non-zero, will set the load value
//    to the configured value divided by this fraction
//    for this run. Allows quickly scaling the timeout
//    between runs.
HpsErr_t HPS_Timer_enable(HPSTimerCtx_t* ctx, unsigned int fraction);

// Disable timer
HpsErr_t HPS_Timer_disable(HPSTimerCtx_t* ctx);

// Set whether timer generates an interrupt
HpsErr_t HPS_Timer_setInterruptEnable(HPSTimerCtx_t* ctx, bool enable);

// Check and clear interrupt flag
//  - Returns true if timer was interrupt flag was set
//  - Returns false if interrupt flag was clear.
//  - If autoClear is true, will clear the interrupt flag.
HpsErr_t HPS_Timer_getInterruptFlag(HPSTimerCtx_t* ctx, bool autoClear);

// Get current timer value
HpsErr_t HPS_Timer_getTime(HPSTimerCtx_t* ctx, unsigned int* curTime);

#endif /* HPS_PRIVATEHPS_TIMER_H_ */
