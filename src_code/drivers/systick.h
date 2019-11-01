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

	#include "cpu.h"

	// SysTick Registers
	#define ST_CTRL_R   	(*((volatile unsigned long *)0xE000E010))   /// SysTick Control and Status Register (STCTRL)
	#define ST_RELOAD_R 	(*((volatile unsigned long *)0xE000E014))   /// SysTick Reload Value Register (STRELOAD)
	#define ST_CURRENT_R 	(*((volatile unsigned long *)0xE000E018))   /// SysTick current value Register (STCURRENT)

	// SysTick defines 
	#define ST_CTRL_COUNT      0x00010000  // Count Flag for STCTRL
	#define ST_CTRL_CLK_SRC    0x00000004  // Clock Source for STCTRL
	#define ST_CTRL_INTEN      0x00000002  // Interrupt Enable for STCTRL
	#define ST_CTRL_ENABLE     0x00000001  // Enable for STCTRL

	// Maximum period
	#define MAX_WAIT           	0x1000000   /* 2^24 */

	#define SEC_TICK			F_CPU_CLK
	#define MSEC_TICK			(F_CPU_CLK/1000)

    /**
     * @brief   SysTick counter structure
     * @details SysTick uses this structure to count ticks,
     *          if comparision is enabled, it'll check if the count has reached the cmp value,
     *          and if it has it'll reset the count and call the callback funtion.
     */
    typedef struct systick_counter_ {
        uint32_t    value;
        uint32_t    cmp;
        bool        cmp_en;
        void        (*counter_cb)(void);
    } systick_counter_t;

    /**
     * @brief   SysTick countdown structure.
     * @details SysTick uses this structure to count down the value every tick,
     *          once the value reaches zero, it'll stop the count down and
     *          call the callback function.
     */
    typedef struct systick_countdown_ {
        uint32_t    value;
        bool        en;
        void        (*countdown_cb)(void);
    } systick_countdown_t;

    /**
     * @brief   SysTick driver descriptor
     */
	typedef struct systick_descriptor_ {
	    systick_counter_t   counter;
	    systick_countdown_t countdown;
	    uint32_t            tick_rate;
	}systick_descriptor_t;

	void SysTick_Init(systick_descriptor_t* descriptor);
	void SysTick_SetPeriod(uint32_t Period);

	void SysTick_Reset(void);

	void SysTick_IntEnable(void);
	void SysTick_IntDisable(void);

	void SysTick_Start(void);
	void SysTick_Stop(void);

	void SysTick_Handler(void);

	uint32_t SysTick_GetTime(void);
	int32_t SysTick_TimeElapsed(uint32_t time);
	
#endif // SYSTICK_H

