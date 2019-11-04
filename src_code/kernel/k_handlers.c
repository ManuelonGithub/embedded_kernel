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
    PendSV_init();

    SystemTick_init(1000);  // 1000hz rate -> system tick triggers every milisecond

    scheduler_init();
    ProcessCreate(0, IDLE_LEVEL, &idle);
	// register the server processes
}

/**
 * @brief   Starts the kernel's run-mode.
 * @details When a kernel is in run mode, user processes are able to run.
 */
inline void kernel_start() {
    PendSV();
}

/**
 * @brief   System Tick Exception handler.
 * @details Manages the running process' allotted runtime
 *          and to provide the system an accurate time-keeping system.
 * @todo    Implement a sleep feature for the processes.
 */
void SystemTick_handler(void)
{
    running->timer--;
    if (running->timer == 0) {
        PendSV();
    }

    // Do time queue thing

}

/**
 * @brief   Pending Supervisor Call trap handler.
 * @details When this trap is called, the system will evaluate
 *          Which process should run next.
 *          It also performs the process switching.
 */
void PendSV_handler(void)
{
    DISABLE_IRQ();  // Disable interrupts to procedure doesn't get corrupted

    // Because PendSV is called on start-up and when a process termination occurs,
    // It needs to account for when there is no running process.
    if (running != NULL) {  // Only save running process context is there is a running process
        SaveProcessContext();
        running->sp = (uint32_t*)GetPSP();
    }

    running = Schedule();
    SetPSP((uint32_t)running->sp);
    RestoreProcessContext();

    running->timer = PROC_RUNTIME;

    SystemTick_reset();
    SystemTick_resume();

    ENABLE_IRQ();

    StartProcess();
}

/**
 * @brief   Supervisor Call trap handler.
 * @details Trap handler has been structured so it's as CPU-generic as possible.
 *          CPU-specific implementation is done in the k_cpu module.
 */
void SVC_handler() 
{
    SaveTrapReturn();   // save the trap return address

    SystemTick_pause();

    if (TrapSource() == KERNEL) {   // Check if the trap was called by the kernel
        SaveContext();              // In which case the trap needs to save its own context (since the trap itself is the kernel)
        KernelCall_handler(k_GetCall());
        RestoreContext();
    }
    else {                      // Trap was called by a process
        SaveProcessContext();   // So the process' context is saved
        KernelCall_handler(k_GetCall());
        RestoreProcessContext();
    }

    SystemTick_resume();

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
            PendSV();
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
    newPCB->sp_top = (uint32_t*)malloc(STACKSIZE);
    newPCB->id = pid;

    newPCB->sp = newPCB->sp_top+STACKSIZE;

    InitProcessContext(&newPCB->sp, proc_program, &terminate);

    newPCB->next = NULL;
    newPCB->prev = NULL;

    int retval = LinkPCB(newPCB, priority);

    if (retval < 0) {   // Something went wrong with pcb linking
      free(newPCB->sp);
      free(newPCB);
    }

    return retval;
}




