/**
 * @file    Kernel_Handlers.c
 * @brief   Contains all functions pertaining towards
 *          the kernel's trap handlers and initializer functions.
 * @details This module should not be exposed to user programs.
 * @author  Manuel Burnay
 * @date    2019.10.23 (Created)
 * @date    2019.10.23 (Last Modified)
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "Kernel_Handlers.h"
#include "Kernel_Scheduler.h"
#include "Kernel_Processes.h"
#include "cpu.h"

pcb_t* running;

/**
 * @brief   Generic Idle process used by the kernel.
 */
void idle()
{
    while(1) {}
}

/**
 * @brief   Initializes kernel data structures, drivers, and critical processes.
 * @details
 */
void kernel_init()
{
    // init pendSV priority
    // init SVC priority ?

	// init drivers
	// 		systick init
	// 		uart init

    scheduler_init();
    reg_proc(idle, 0, IDLE_LEVEL);
	// register the server processes
}

/**
 * @brief   Starts the kernel run-mode.
 * @details "Run-mode" means that now user processes are running.
 */
inline void kernel_start() {
	running = Schedule();
	// set kernel call arguments
	SVC();
}

/**
 * @brief   Pending Supervisor Call trap handler.
 */
void PendSV_handler()
{
    printf("hey");
}

/**
 * @brief   Supervisor Call trap handler.
 */
void SVC_handler() 
{
	/* Supervisor call (trap) entry point
	 * Using MSP - trapping process either MSP or PSP (specified in LR)
	 * Source is specified in LR: F1 (MSP) or FD (PSP)
	 * Save r4-r11 on trapping process stack (MSP or PSP)
	 * Restore r4-r11 from trapping process stack to CPU
	 * SVCHandler is called with r0 equal to MSP or PSP to access any arguments
	 */

	/* Save LR for return via MSP or PSP */
	__asm(" 	PUSH 	{LR}");	// Not sure if this is required. LR's state is saved 

	/* Trapping source: MSP or PSP? */
	__asm(" 	TST 	LR,#4");	/* Bit #3 (0100b) indicates MSP (0) or PSP (1) */
	__asm(" 	BNE 	RtnViaPSP");

	/* Trapping source is MSP - save r4-r11 on stack (default, so just push) */
	__asm(" 	PUSH 	{r4-r11}");
	__asm(" 	MRS	r0,msp");
	__asm(" 	BL	SVCHandler");	/* r0 is MSP */
	__asm(" 	POP	{r4-r11}");
	__asm(" 	POP 	{PC}");

	/* Trapping source is PSP - save r4-r11 on psp stack (MSP is active stack) */
	__asm("RtnViaPSP:");
	__asm(" 	mrs 	r0,psp");
	__asm("		stmdb 	r0!,{r4-r11}");	/* Store multiple, decrement before */
	__asm("		msr	psp,r0");
	__asm(" 	BL	SVCHandler");	/* r0 Is PSP */
	  
	/* Restore r4..r11 from trapping process stack  */
	__asm(" 	mrs 	r0,psp");
	__asm("		ldmia 	r0!,{r4-r11}");	/* Load multiple, increment after */
	__asm("		msr	psp,r0");
	__asm(" 	POP 	{PC}");
}
