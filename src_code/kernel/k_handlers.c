/**
 * @file    k_handlers.c
 * @brief   Contains all functions pertaining towards
 *          the kernel's trap handlers and initializer functions.
 * @details This module should not be exposed to user programs.
 * @author  Manuel Burnay
 * @date    2019.10.23 (Created)
 * @date    2019.10.23 (Last Modified)
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "k_handlers.h"
#include "k_scheduler.h"
#include "k_processes.h"
#include "k_calls.h"
#include "calls.h"
#include "cpu.h"

pcb_t* running;

void KernelCall_handler(cpu_context_t* argptr);

/**
 * @brief   Generic Idle process used by the kernel.
 */
void idle()
{
    while(1) {}
}

/**
 * @brief   Initializes kernel data structures, drivers, and critical processes.
 */
void kernel_init()
{
    // init pendSV priority
    // init SVC priority ?

	// init drivers
	// 		systick init
	// 		uart init

    scheduler_init();
    ProcessCreate(&idle, 0, IDLE_LEVEL);
	// register the server processes
}

/**
 * @brief   Starts the kernel run-mode.
 * @details "Run-mode" means that now user processes are running.
 */
inline void kernel_start() {
	running = Schedule();

	kernel_call_t call;     // May need a better solution b/c pretty sure this stack frame will never get cleared.
	call.code = STARTUP;

	k_SetCall(&call);

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
	__asm(" 	BL	KernelCall_handler");	/* r0 is MSP */
	__asm(" 	POP	{r4-r11}");
	__asm(" 	POP 	{PC}");

	/* Trapping source is PSP - save r4-r11 on psp stack (MSP is active stack) */
	__asm("RtnViaPSP:");
	__asm(" 	mrs 	r0,psp");
	__asm("		stmdb 	r0!,{r4-r11}");	/* Store multiple, decrement before */
	__asm("		msr	psp,r0");
	__asm(" 	BL	KernelCall_handler");	/* r0 Is PSP */
	  
	/* Restore r4..r11 from trapping process stack  */
	__asm(" 	mrs 	r0,psp");
	__asm("		ldmia 	r0!,{r4-r11}");	/* Load multiple, increment after */
	__asm("		msr	psp,r0");
	__asm(" 	POP 	{PC}");
}


void KernelCall_handler(cpu_context_t* proc_cpu)
{
    kernel_call_t* call = k_GetCall(proc_cpu);

    switch(call->code) {
        case PROC_CREATE: {
            call->retval = k_ProcessCreate(call->argv[0], call->argv[1], (void(*)())(call->argv[2]));
        } break;

        case STARTUP: {
            set_PSP((uint32_t)(running->sp) + 8 * sizeof(uint32_t));

            __asm(" movw    LR,#0xFFFD");  /* Lower 16 [and clear top 16] */
            __asm(" movt    LR,#0xFFFF");  /* Upper 16 only */
            __asm(" bx  LR");          /* Force return to PSP */
        } break;

        case GETID: {
            call->retval = running->id;
        } break;

        case NICE: {
            if (LinkPCB(running, call->argv[0]) == HANDLE_SUCCESS) {
                call->retval = HANDLE_SUCCESS;
                running->priority = call->argv[0];
            }
            else {
                call->retval = (uint32_t)INVALID_PRIORITY;
            }
        } break;

        case SEND: {

        } break;

        case RECV: {

        } break;

        case TERMINATE: {

        } break;
    }
}

uint32_t k_ProcessCreate(uint32_t pid, uint32_t priority, void (*proc_program)())
{
    pcb_t* newPCB = (pcb_t*)malloc(sizeof(pcb_t));
    newPCB->sp = (uint32_t*)malloc(STACKSIZE);
    newPCB->id = pid;
    newPCB->priority = priority;

    cpu_context_t* cpu = (cpu_context_t*)newPCB->sp;
    cpu->psr = PSR_INIT_VAL;
    cpu->lr = (uint32_t)&terminate;
    cpu->pc = (uint32_t)proc_program;

    newPCB->next = NULL;
    newPCB->prev = NULL;

    int retval = LinkPCB(newPCB, newPCB->priority);

    if (retval < 0) {   // Something went wrong with pcb linking
      free(newPCB->sp);
      free(newPCB);
    }

    return retval;
}




