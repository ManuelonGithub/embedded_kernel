/**
 * @file    systick.h
 * @brief   Contains all the definitions, structures, and function prototypes
 *          regarding the operation of the SysTick driver
 * @author  Manuel Burnay
 * @date    2019.09.26 (Created)
 * @date    2019.10.04 (Last Modified)
 */

#ifndef SYSTICK_H
	#define SYSTICK_H

	#include <stdint.h>

	// SysTick Registers
	#define ST_CTRL_R   	(*((volatile uint32_t*)0xE000E010))   /// SysTick Control and Status Register (STCTRL)
	#define ST_RELOAD_R 	(*((volatile uint32_t*)0xE000E014))   /// SysTick Reload Value Register (STRELOAD)
	#define ST_CURRENT_R 	(*((volatile uint32_t*)0xE000E018))   /// SysTick current value Register (STCURRENT)

	// SysTick defines 
	#define ST_CTRL_COUNT      0x00010000  // Count Flag for STCTRL
	#define ST_CTRL_CLK_SRC    0x00000004  // Clock Source for STCTRL
	#define ST_CTRL_INTEN      0x00000002  // Interrupt Enable for STCTRL
	#define ST_CTRL_ENABLE     0x00000001  // Enable for STCTRL

	// Maximum period
	#define MAX_WAIT    0x1000000   /* 2^24 */

    #define F_CPU_CLK   16000000

	#define SEC_TICK	F_CPU_CLK
	#define MSEC_TICK   (F_CPU_CLK/1000)

	void SysTick_Init(uint32_t Period);
	void SysTick_SetPeriod(uint32_t Period);

	void SysTick_Reset(void);

    /** @brief   Sets the interrupt enable bit in the SysTick control register */
    #define SysTick_IntEnable()     (ST_CTRL_R |= ST_CTRL_INTEN)

	/** @brief   Clears the interrupt enable bit in the SysTick control register */
    #define SysTick_IntDisable()    (ST_CTRL_R &= ~(ST_CTRL_INTEN))

	/**
	 * @brief   Starts the Systick.
	 * @details This function does not interfere with the ST CURRENT register,
	 *          so the systick will count from the value already in it.
	 */
    #define SysTick_Start() (ST_CTRL_R |= ST_CTRL_ENABLE)

	/** @brief   Stops the SysTick. */
    #define SysTick_Stop()  (ST_CTRL_R &= ~(ST_CTRL_ENABLE))
	
#endif // SYSTICK_H

