/**
 * @file    k_handlers.c
 * @brief   Contains all functions pertaining towards
 *          the kernel's trap handlers and initializer functions.
 * @details This module should not be exposed to user programs.
 * @author  Manuel Burnay
 * @date    2019.10.23 (Created)
 * @date    2019.11.03 (Last Modified)
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
#include "k_cpu.h"

pcb_t* running;

void KernelCall_handler(k_call_t* call);

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
    ProcessCreate(0, IDLE_LEVEL, &idle);
	// register the server processes
}

/**
 * @brief   Starts the kernel's run-mode.
 * @details When a kernel is in run mode, user processes are able to run.
 */
inline void kernel_start() {
	running = Schedule();

	k_call_t call;     // May need a better solution b/c pretty sure this stack frame will never get cleared.
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
 * @details Trap handler has been structured so it's as CPU-generic as possible.
 *          CPU-specific implementation is done in the k_cpu module.
 */
void SVC_handler() 
{
    SaveTrapReturn();   // save the trap return address

    if (TrapSource() == KERNEL) {   // Check if the trap was called by the kernel
        SaveContext();              // In which case the trap needs to save its own context
        KernelCall_handler(k_GetCall());
        RestoreContext();
    }
    else {                      // Trap was called by a process
        SaveProcessContext();   // So the process' context is saved
        KernelCall_handler(k_GetCall());
        RestoreProcessContext();
    }

    RestoreTrapReturn();
}

/**
 * @brief   Kernel Call Handler function.
 * @param   [in, out] call: Pointer to call structure where the call's code and arguments reside.
 * @details This function is in charge of analyzing the kernel call structure passed to the trap
 *          and service the call if its parameters are valid.
 */
void KernelCall_handler(k_call_t* call)
{
    switch(call->code) {
        case PROC_CREATE: {
            call->retval = k_ProcessCreate(call->argv[0], call->argv[1], (void(*)())(call->argv[2]));
        } break;

        case STARTUP: {
            //todo: Call PendSV instead of doing this
            set_PSP((uint32_t)(running->sp) + 8 * sizeof(uint32_t));

            __asm(" movw    LR,#0xFFFD");  /* Lower 16 [and clear top 16] */
            __asm(" movt    LR,#0xFFFF");  /* Upper 16 only */
            __asm(" bx  LR");          /* Force return to PSP */
        } break;

        case GETID: {
            call->retval = running->id;
        } break;

        case NICE: {
            call->retval = LinkPCB(running, call->argv[0]);
            // Also need to check if PendSV needs to be called.
        } break;

        case SEND: {

        } break;

        case RECV: {

        } break;

        case TERMINATE: {

        } break;

        default: {

        } break;
    }
}

/**
 * @brief   Creates a process and registers it in kernel space.
 * @param   [in] pid: Unique process identifier value.
 * @param   [in] priortity: Priority level that the process will run in.
 * @param   [in] proc_program: Pointer to start of the program the process will execute.
 * @retval  Returns -1 if the process wasn't able to be created,
 *          otherwise will return the process' ID.
 */
uint32_t k_ProcessCreate(uint32_t pid, uint32_t priority, void (*proc_program)())
{
    pcb_t* newPCB = (pcb_t*)malloc(sizeof(pcb_t));
    newPCB->sp_bottom = (uint32_t*)malloc(STACKSIZE);
    newPCB->id = pid;

    newPCB->sp = newPCB->sp_bottom;

    InitProcessContext((cpu_context_t*)newPCB->sp, proc_program, &terminate);

    newPCB->next = NULL;
    newPCB->prev = NULL;

    int retval = LinkPCB(newPCB, priority);

    if (retval < 0) {   // Something went wrong with pcb linking
      free(newPCB->sp);
      free(newPCB);
    }

    return retval;
}




