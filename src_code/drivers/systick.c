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


/**
 * @brief   Initializes the sysTick driver & sets up the descriptor for the driver.
 * @param   [in] rate: Frequency that SysTick should trigger (in Hz).
 * @details The descriptor's pointer will be saved internally into the driver module so the
 *          interrupt handler can have access to it.
 */
void SysTick_Init(uint32_t rate)
{
	ST_CTRL_R = ST_CTRL_CLK_SRC;

    SysTick_SetPeriod(F_CPU_CLK/rate);
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





