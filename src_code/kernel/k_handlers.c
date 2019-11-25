/**
 * @file    k_handlers.c
 * @brief   Contains all functions pertaining towards
 *          the kernel's trap handlers and initializer functions.
 * @details This module should not be exposed to user programs.
 * @author  Manuel Burnay
 * @date    2019.10.23 (Created)
 * @date    2019.11.21 (Last Modified)
 */

#include <stdlib.h>
#include <stdio.h>
#include "k_handlers.h"
#include "k_scheduler.h"
#include "k_processes.h"
#include "calls.h"
#include "k_cpu.h"
#include "k_messaging.h"
#include "dlist.h"
#include "uart.h"
#include "systick.h"
#include "k_terminal.h"

pcb_t *running;
uart_descriptor_t uart;

void p_KernelCall_handler(k_call_t* call);

/**
 * @brief   Generic Idle process used by the kernel.
 */
void idle()
{
    while (1) {}
}

/**
 * @brief   Initializes kernel data structures, drivers, and critical processes.
 */
void kernel_init()
{
    PendSV_init();

    process_init();
    k_MsgInit();

    SysTick_Init(1000);  // 1000 Hz rate -> system tick triggers every milisecond

    uart.echo = false;
    UART0_Init(&uart);

    pcreate(0, IDLE_LEVEL, &idle);
    // Set the initial running process to be running
    // This avoids any "NULL" checks of the running process,
    // As it'll always be associated with a process.
    running = Schedule();
    k_MsgBoxBind(0, running);

	// register the server processes
    pcreate(0, 0, &output_manager);
}

/**
 * @brief   Starts the kernel's run-mode.
 * @details When a kernel is in run mode, user processes are able to run.
 */
inline void kernel_start()
{
    k_code_t code = STARTUP;
    SetCallReg((k_call_t*)&code);

    SVC();
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
        // Call the process clean function
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
    SysTick_Stop();

    SaveProcessContext();
    running->sp = (uint32_t*)GetPSP();

    running = Schedule();

    SetPSP((uint32_t)running->sp);
    RestoreProcessContext();

    running->timer = PROC_RUNTIME;

    SysTick_Reset();
    SysTick_Start();
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

    SysTick_Stop();

    if (TrapSource() == KERNEL) {   // Check if the trap was called by the kernel
        // In which case the kernel needs to save its own context
        SaveContext();
        p_KernelCall_handler(GetCallReg());
        RestoreContext();
    }
    else {                      // Trap was called by a process
        SaveProcessContext();   // So the process' context is saved
        p_KernelCall_handler(GetCallReg());
        RestoreProcessContext();
    }

    SysTick_Start();

    RestoreTrapReturn();
}

/**
 * @brief   Kernel Call Handler function.
 * @param   [in, out] call:
 *              Pointer to call structure where the call's code and arguments reside.
 * @details This function is in charge of analyzing
 *          the kernel call structure passed to the trap
 *          and service the call if its parameters are valid.
 */
void p_KernelCall_handler(k_call_t* call)
{
    switch(call->code) {
        case PCREATE: {
            call->retval = k_pcreate((pcreate_args_t*)call->arg);
        } break;

        case STARTUP: {
            /*
             * This block of code is here to counter-act what PendSV would do to a
             * Non-initialized process.
             * While admittedly a bit ugly,
             * it does allow for both PendSV and this Call handler
             * to be as efficient as possible at "steady-state" operation.
             * Here we're making the assumption
             * that running is set to be the idle process.
             */

            // Initializes the process stack pointer to the idle process stack
            SetPSP((uint32_t)running->sp);

            /*
             * This essentially removes the initial "non-essential"
             * cpu context in the process' stack, which is necessary since
             * PendSV will save the current "non-essential" cpu context
             * onto the process stack at the beginning of its handler.
             */
            RestoreProcessContext();

            PendSV();
        } break;

        case GETPID: {
            call->retval = (int32_t)running->id;
        } break;

        case NICE: {
            call->retval = LinkPCB(running, (priority_t)call->arg[0]);
            PendSV();
        } break;

        case BIND: {
            call->retval = k_MsgBoxBind((pmbox_t)call->arg[0], running);
        } break;

        case UNBIND: {
            call->retval = k_MsgBoxUnbind((pmbox_t)call->arg[0], running);
        } break;

        case SEND: {
            call->retval = k_MsgSend((pmsg_t*)call->arg, running);
        } break;

        case RECV: {
            if (!k_MsgRecv((pmsg_t*)call->arg, running)) {
                UnlinkPCB(running);
                PendSV();
            }
            // "Message size" retval is taken care by the call function.
            // b/c this call may not have the desired retval when its called
            // i.e. when there is no message to receive
            call->retval = 0;
        } break;

        case TERMINATE: {
            k_Terminate();
        } break;

        default: {

        } break;
    }
}

/**
 * @brief   Creates a process and registers it in kernel space.
 * @param   [in] pid: Unique process identifier value.
 * @param   [in] priortity: Priority level that the process will run in.
 * @param   [in] proc_program:
 *              Pointer to start of the program the process will execute.
 * @retval  Returns the process ID that was created.
 *          0 if a process wasn't able to be created.
 */
proc_t k_pcreate(pcreate_args_t* args)
{
    pcb_t* pcb = k_AllocatePCB(args->id);
    proc_t retval = 0;

    if (pcb != NULL) {             // PCB was successfully allocated
        InitProcessContext(&pcb->sp, args->proc_program, &terminate);

        if (LinkPCB(pcb, args->prio)) {
            // PCB was successfully linked into its queue
            retval = pcb->id;  // set return value to ID
        }
        else {
            k_DeallocatePCB(pcb->id);
        }
    }

    return retval;
}

/**
 * @brief   Terminates the running process.
 * @details Unbinds all message boxes and de-allocates the process.
 */
void k_Terminate()
{
    // 1. Unlink process from its process queue
    UnlinkPCB(running);

    // 2. Unbind all message boxes from process
    k_MsgBoxUnbindAll(running);

    // 3. Erase PCB
    k_DeallocatePCB(running->id);

    // 4. Schedule a new process
    running = Schedule();
    SetPSP((uint32_t)running->sp);
    running->timer = PROC_RUNTIME;

    // 5. Reset the System timer
    SysTick_Reset();
}



