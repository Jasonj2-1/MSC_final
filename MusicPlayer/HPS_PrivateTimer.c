/* 
 * HPS Private Timer Driver
 * ------------------------
 *
 * ~~ Add description, etc ~~
 */

#include "HPS_PrivateTimer.h"
#include "Util/bit_helpers.h"
#include "DE1SoC_Addresses/DE1SoC_Addresses.h"

//Register Offsets
#define HPS_TIMER_LOAD      (0x00/sizeof(unsigned int))
#define HPS_TIMER_VALUE     (0x04/sizeof(unsigned int))
#define HPS_TIMER_CONTROL   (0x08/sizeof(unsigned int))
#define HPS_TIMER_INTERRUPT (0x0C/sizeof(unsigned int))
#define HPS_TIMER_HPSIRQ    (0xA8/sizeof(unsigned int))

//Flags
#define HPS_TIMER_CONTROL_ENABLE    0
#define HPS_TIMER_CONTROL_AUTOLOAD  1
#define HPS_TIMER_CONTROL_IRQ_EN    2
#define HPS_TIMER_CONTROL_PRESCALE  8

#define HPS_TIMER_INTERRUPT_FLAG    0



/*
 * Internal Functions
 */


static inline void _HPS_Timer_clearInterruptFlag(HPSTimerCtx_t* ctx) {
    // base is volatile, so this will read the IRQ flag then write the same value 
    // back to clear it. Some timers clear on read, others clear on write, so this
    // ensures the IRQ is always cleared.
    ctx->base[HPS_TIMER_INTERRUPT] = ctx->base[HPS_TIMER_INTERRUPT]; // @suppress("Assignment to itself")
}

static inline void _HPS_Timer_disable(HPSTimerCtx_t* ctx) {
    ctx->base[HPS_TIMER_CONTROL] &= ~_BV(HPS_TIMER_CONTROL_ENABLE);
}

static inline unsigned int _HPS_Timer_getCtrlAndDisable(HPSTimerCtx_t* ctx) {
    unsigned int timerCtl = ctx->base[HPS_TIMER_CONTROL];
    if (timerCtl &  _BV(HPS_TIMER_CONTROL_ENABLE)) {
        _HPS_Timer_disable(ctx);
    }
    return timerCtl;
}

static inline unsigned int _HPS_Timer_getTime(HPSTimerCtx_t* ctx) {
    return ctx->base[HPS_TIMER_VALUE];
}
// equal to *(base + HPS_TIMER_VALUE) , a[3] means dereference == * (p+3)  if *p = a[4]
// -> Seek for the value in this address
// see the base as an array array[HPS_TIMER_VALUE] = array[4] is the value

static inline void _HPS_Timer_enable(HPSTimerCtx_t* ctx) {
    ctx->base[HPS_TIMER_CONTROL] |= _BV(HPS_TIMER_CONTROL_ENABLE);
}


// Cleanup function called when driver destroyed.
//  - Disables any hardware and interrupts.
//  - Free any allocated memory
static void _HPS_Timer_cleanup(HPSTimerCtx_t* ctx) {
    if (ctx->base) {
        // Stop the timer running when driver cleared.
        _HPS_Timer_disable(ctx);
        _HPS_Timer_clearInterruptFlag(ctx);
    }
}

// Initialise Private Timer Driver
//  - base is a pointer to the private timer module
//  - Returns Util/error Code
//  - Returns context pointer to *ctx
HpsErr_t HPS_Timer_initialise(void* base, HPSTimerCtx_t** pCtx) {
    //Ensure user pointers valid
    if (!base) return ERR_NULLPTR;
    if (!pointerIsAligned(base, sizeof(unsigned int))) return ERR_ALIGNMENT;
    //Allocate the driver context, validating return value.
    HpsErr_t status = DriverContextAllocateWithCleanup(pCtx, &_HPS_Timer_cleanup);
    if (ERR_IS_ERROR(status)) return status;
    //Save base address pointers
    HPSTimerCtx_t* ctx = *pCtx;
    ctx->base = (unsigned int*)base;
    //Initialise generic timer interface
    ctx->timer.ctx = ctx;
    //APIs in the generic timer interface, initialised to pointers in the
    //HPS Private Timer driver.
    ctx->timer.enable        = (TimerEnableFunc_t    )&HPS_Timer_enable;
    ctx->timer.disable       = (TimerDisableFunc_t   )&HPS_Timer_disable;
    ctx->timer.getLoad       = (TimerGetTimeFunc_t   )&HPS_Timer_getLoad;
    ctx->timer.getTime       = (TimerGetTimeFunc_t   )&HPS_Timer_getTime;
    ctx->timer.getRate       = (TimerGetRateFunc_t   )&HPS_Timer_getClockRate;
    ctx->timer.getMode       = (TimerGetModeFunc_t   )&HPS_Timer_getMode;
    ctx->timer.checkOverflow = (TimerOverflowedFunc_t)&HPS_Timer_getInterruptFlag;
    ctx->timer.configure     = (TimerConfigureFunc_t )&HPS_Timer_configure;
    //Ensure timer initialises to disabled
    _HPS_Timer_disable(ctx);
    _HPS_Timer_clearInterruptFlag(ctx);
    


    ctx->mode = TIMER_MODE_EVENT;

    //
    //
    //
    //   Finish This Bit
    //
    //
    
    // ... Any other initialisation "stuff" ...
    
    //Initialised
    DriverContextSetInit(ctx);
    return status;
}

// Check if driver initialised
//  - Returns true if driver previously initialised
bool HPS_Timer_isInitialised(HPSTimerCtx_t* ctx) {
    return DriverContextCheckInit(ctx);
}

// Configure the timer
//  - For one-shot mode, disables the timer. Call HPS_Timer_enable() to start timer.
//  - Otherwise timer will be enabled after configure only if it was enabled before
HpsErr_t HPS_Timer_configure(HPSTimerCtx_t* ctx, TimerMode mode, 
                             unsigned int prescalar, unsigned int loadValue
) {
    //Ensure context valid and initialised
    HpsErr_t status = DriverContextValidate(ctx);
    if (ERR_IS_ERROR(status)) return status;
    //Ensure prescalar in range
    if (prescalar > HPS_TIMER_PRESCALE_MAX) return ERR_TOOBIG;
    //Check if timer is enabled, and stop it if it is
    unsigned int timerCtl = _HPS_Timer_getCtrlAndDisable(ctx);
    //Clear any configurable bits
    timerCtl &= ~((HPS_TIMER_PRESCALE_MAX << HPS_TIMER_CONTROL_PRESCALE) | _BV(HPS_TIMER_CONTROL_AUTOLOAD));
    //Changes based on mode
    timerCtl |= ((mode != TIMER_MODE_ONESHOT) << HPS_TIMER_CONTROL_AUTOLOAD);
    if (mode == TIMER_MODE_EVENT) {
        // Clear IRQ bit in event mode
        timerCtl &= ~_BV(HPS_TIMER_CONTROL_IRQ_EN);    // clear Interrupt bit (I)
        loadValue = UINT32_MAX;
    } else if (mode == TIMER_MODE_ONESHOT) {
        // Ensure remains disabled if one-shot mode.
        timerCtl &= ~_BV(HPS_TIMER_CONTROL_ENABLE);    // clear Enable bit (E)
    }
    //Configure the timer ...
    //  e.g. set prescalar, load value, etc.
    
    timerCtl = timerCtl | (prescalar << HPS_TIMER_CONTROL_PRESCALE);

    ctx->loadValue    = loadValue;
    ctx->base[HPS_TIMER_LOAD]     = loadValue;
    ctx->mode = mode;
    
    ctx->base[HPS_TIMER_CONTROL] = timerCtl;
    
    return ERR_SUCCESS;
}

// Disable timer
HpsErr_t HPS_Timer_disable(HPSTimerCtx_t* ctx) {
    //Ensure context valid and initialised
    HpsErr_t status = DriverContextValidate(ctx);
    if (ERR_IS_ERROR(status)) return status;
    //Clear the timer enable flag
    _HPS_Timer_disable(ctx);
    return ERR_SUCCESS;
}

// Enable timer
//  - If fraction is non-zero, will set the load value
//    to the configured value divided by this fraction
//    for this run. Allows quickly scaling the timeout
//    between runs.
HpsErr_t HPS_Timer_enable(HPSTimerCtx_t* ctx, unsigned int fraction) {
    
    HpsErr_t status = DriverContextValidate(ctx);
    if (ERR_IS_ERROR(status)) return status;
    if (!fraction) {
    	ctx->loadValue = ctx->loadValue / fraction;
    }
    ctx->base[HPS_TIMER_CONTROL] = _BV(HPS_TIMER_CONTROL_ENABLE);
    return ERR_SUCCESS;
}

// Get load timer value
HpsErr_t HPS_Timer_getLoad(HPSTimerCtx_t* ctx, unsigned int* loadValue) {
    if (!loadValue) return ERR_NULLPTR;
    //Ensure context valid and initialised
    HpsErr_t status = DriverContextValidate(ctx);
    if (ERR_IS_ERROR(status)) return status;
    //Return current time
    *loadValue = ctx->loadValue;
    return ERR_SUCCESS;
}

// Get the rounded timer clock rate
//  - Returns via *clockRate the timer clock rate based on configured prescalar
//  - If prescalar is UINT32_MAX, will return based on previously configured.
//    prescalar settings, otherwise will calculate for specified value
HpsErr_t HPS_Timer_getClockRate(HPSTimerCtx_t* ctx, unsigned int prescalar, 
                                unsigned int* clockRate) {

	if (!clockRate) return ERR_NULLPTR;
    HpsErr_t status = DriverContextValidate(ctx);
    if (ERR_IS_ERROR(status)) return status;

    unsigned int actual_prescalar;

    if (prescalar == UINT32_MAX)
    {
    	actual_prescalar = (ctx->base[HPS_TIMER_CONTROL] >> HPS_TIMER_CONTROL_PRESCALE) & 0xff;
    } else {
    	actual_prescalar = prescalar;
    }

    *clockRate = *clockRate / actual_prescalar;

    return ERR_SUCCESS;
    
}

// Get timer mode
HpsErr_t HPS_Timer_getMode(HPSTimerCtx_t* ctx, TimerMode* mode) {
    
	if (!mode) return ERR_NULLPTR;
    HpsErr_t status = DriverContextValidate(ctx);
    if (ERR_IS_ERROR(status)) return status;
    *mode = ctx->mode;
    
    return ERR_SUCCESS;
}

// Get current timer value
HpsErr_t HPS_Timer_getTime(HPSTimerCtx_t* ctx, unsigned int* curTime) {
    
	if (!curTime) return ERR_NULLPTR;
    HpsErr_t status = DriverContextValidate(ctx);
    if (ERR_IS_ERROR(status)) return status;

    *curTime = ctx->base[HPS_TIMER_VALUE];
    
    return ERR_SUCCESS;
}

// Set whether timer generates an interrupt
HpsErr_t HPS_Timer_setInterruptEnable(HPSTimerCtx_t* ctx, bool enable) {
    //Ensure context valid and initialised
    HpsErr_t status = DriverContextValidate(ctx);
    if (ERR_IS_ERROR(status)) return status;
    //Set the timer interrupt flag
    if (enable) {
        if (ctx->mode == TIMER_MODE_EVENT) return ERR_INUSE;
        ctx->base[HPS_TIMER_CONTROL] |=  _BV(HPS_TIMER_CONTROL_IRQ_EN);
    } else {
        ctx->base[HPS_TIMER_CONTROL] &= ~_BV(HPS_TIMER_CONTROL_IRQ_EN);
    }
    return ERR_SUCCESS;
}

// Check and clear interrupt flag
//  - Returns true if timer was interrupt flag was set
//  - Returns false if interrupt flag was clear.
//  - If autoClear is true, will clear the interrupt flag.
HpsErr_t HPS_Timer_getInterruptFlag(HPSTimerCtx_t* ctx, bool autoClear) {
    //Ensure context valid and initialised
    HpsErr_t status = DriverContextValidate(ctx);
    if (ERR_IS_ERROR(status)) return status;
    //Check flag
    bool flagSet = ctx->base[HPS_TIMER_INTERRUPT] & _BV(HPS_TIMER_INTERRUPT_FLAG);
    if (flagSet && autoClear) {
        _HPS_Timer_clearInterruptFlag(ctx);
    }
    //Return the flag
    return flagSet ? ERR_TRUE : ERR_FALSE;
}
