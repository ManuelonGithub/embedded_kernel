/**
 * @file    systick.c
 * @brief   Contains all functionality of the SysTick driver.
 * @author  Manuel Burnay
 * @date    2019.09.26 (Created)
 * @date    2019.10.04 (Last Modified)
 */

#include <stdint.h>
#include <stdbool.h>
#include "SysTick.h"


systick_descriptor_t* sys;

/**
 * @brief   Initializes the sysTick driver & sets up the descriptor for the driver.
 * @param   [in, out] descriptor: pointer to SysTick descriptor that the driver will configure
 *                                with to have access to it.
 * @details The descriptor's pointer will be saved internally into the driver module so the
 *          interrupt handler can have access to it.
 */
void SysTick_Init(systick_descriptor_t* descriptor)
{
	ST_CTRL_R = ST_CTRL_CLK_SRC;

    sys = descriptor;
    SysTick_SetPeriod(F_CPU_CLK/sys->tick_rate);
    SysTick_Reset();
}

/**
 * @brief   Sets the SysTick period.
 * @param   [in] Period: Number of clock cycles between interrupt triggers, i.e. period.
 */
void SysTick_SetPeriod(uint32_t Period)
{
    /*
     For an interrupt, must be between 2 and 16777216 (0x100.0000 or 2^24)
    */
    ST_RELOAD_R = Period - 1;  /* 1 to 0xff.ffff */
}

/**
 * @brief   Resets the SysTick current value register and time count.
 */
void SysTick_Reset()
{
    SysTick_IntDisable();

    ST_CURRENT_R = 0;

    SysTick_IntEnable();
}

/**
 * @brief	Sets the interrupt enable bit in the SysTick control register
 */
void SysTick_IntEnable(void)
{
    // Set the interrupt bit in STCTRL
    ST_CTRL_R |= ST_CTRL_INTEN;
}

/**
 * @brief	Clears the interrupt enable bit in the SysTick control register
 */
void SysTick_IntDisable(void)
{
    // Clear the interrupt bit in STCTRL
    ST_CTRL_R &= ~(ST_CTRL_INTEN);
}

/**
 * @brief 	Starts the Systick.
 * @details	This function does not interfere with the ST CURRENT register, 
 *			so the systick will count from the value already in it.
 */
void SysTick_Start(void)
{
	// Set the clock source to internal and enable the counter to interrupt
	ST_CTRL_R |= ST_CTRL_ENABLE;  
}

/**
 * @brief	Stops the SysTick.
 */
void SysTick_Stop(void)
{
	// Clear the enable bit to stop the counter
	ST_CTRL_R &= ~(ST_CTRL_ENABLE);  
}

/**
 * @brief	Interrupt Handler for the SysTick driver.
 * @details This systick interrupt handler performs two functions:
 *          - Increments the counter value
 *              - If comparison is enable it'll compare with the cmp value,
 *                  - If they are equal, it'll reset the counter and call the callback function.
 *          - Decrements the countdown value if enabled
 *              - If the value is 0, it'll disable the countdown and call the callback function.
 *
 */
void SysTick_IntHandler(void)
{
    sys->counter.value++;

    if (sys->counter.cmp_en && (sys->counter.value == sys->counter.cmp)) {
        sys->counter.counter_cb();
        sys->counter.value = 0;
    }

    if (sys->countdown.en) {
        sys->countdown.value--;

        if (!sys->countdown.value) {
            sys->countdown.countdown_cb();
            sys->countdown.en = false;
        }
    }
}
