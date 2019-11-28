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
#include <string.h>
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

pcb_t *running, *pTerminal, *pIdle;
uart_descriptor_t uart;

active_IO_t IO;

#ifdef REAL_TIME_MODE

extern pcb_t   proc_table[PID_MAX];

#else

extern pcb_t*  proc_table[PID_MAX];

#endif

extern pmsgbox_t msgbox[BOXID_MAX];

void k_KernelCall_handler(k_code_t code);
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

    process_attr_t pattr = {
         .id = 0,
         .priority = 0,
         .name = "idle"
    };

    pIdle = GetPCB(k_pcreate(&pattr, &idle, &terminate));
    LinkPCB(pIdle, IDLE_LEVEL);

    // Register the Terminal server process
    strcpy(pattr.name, "terminal");

    pTerminal = GetPCB(k_pcreate(&pattr, &terminal, &terminate));
    LinkPCB(pTerminal, PRIV_PRIORITY);
}

/**
 * @brief   Starts the kernel's run-mode.
 * @details When a kernel is in run mode, user processes are able to run.
 */
inline void kernel_start()
{
    k_call_t call;
    call.code = STARTUP;

    SetCallReg(&call);
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
    }

    if (!UART0_empty() && pTerminal->priority != 0) {
        LinkPCB(pTerminal, 0);
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
    SysTick_Stop();

    SaveProcessContext();
    running->sp = (uint32_t*)GetPSP();

    if (running->state == RUNNING)  running->state = WAITING_TO_RUN;
    running = Schedule();
    running->state = RUNNING;

    SetPSP((uint32_t)running->sp);
    RestoreProcessContext();

    running->timer = PROC_RUNTIME;

    SysTick_Reset();
    SysTick_Start();
    ENABLE_IRQ();
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


void k_KernelCall_handler(k_code_t code)
{
    switch(code) {
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
//            RestoreContext();
            /*
             * This essentially removes the initial "non-essential"
             * cpu context in the process' stack, which is necessary since
             * PendSV will save the current "non-essential" cpu context
             * onto the process stack at the beginning of its handler.
             */
            RestoreProcessContext();

            PendSV();
        } break;

        default: {

        } break;
    }
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
            call->retval = k_pcreate(
                    (process_attr_t*)call->arg[0],
                    (void (*)())call->arg[1],
                    &terminate);
        } break;

        case STARTUP: {
            running = Schedule();
            // Initializes the process stack pointer to the idle process stack
            SetPSP((uint32_t)running->sp);

            /*
             * This essentially removes the initial "non-essential"
             * cpu context in the process' stack, which is necessary since
             * PendSV will save the current "non-essential" cpu context
             * onto the process stack at the beginning of its handler.
             */
            RestoreProcessContext();

            running->timer = PROC_RUNTIME;

            // Reset the System timer
            SysTick_Reset();
            SysTick_Start();

            StartProcess();
        } break;

        case GETPID: {
            call->retval = (int32_t)running->id;
        } break;

        case NICE: {
            if (call->arg[0] > 0 && call->arg[0] < PRIORITY_LEVELS) {
                LinkPCB(running, (priority_t)call->arg[0]);
            }
            call->retval = running->priority;
            PendSV();
        } break;

        case BIND: {
            call->retval = k_MsgBoxBind((pmbox_t)call->arg[0], running);
        } break;

        case UNBIND: {
            call->retval = k_MsgBoxUnbind((pmbox_t)call->arg[0], running);
        } break;

        case SEND: {
            if (((pmsg_t*)call->arg)->dst < BOXID_MAX &&
                msgbox[((pmsg_t*)call->arg)->src].owner == running) {
                call->retval = k_MsgSend((pmsg_t*)call->arg);
            }
            else {
                call->retval = 0;
            }
        } break;

        case RECV: {
            if (((pmsg_t*)call->arg)->dst < BOXID_MAX &&
                msgbox[((pmsg_t*)call->arg)->dst].owner == running) {
                k_MsgRecv((pmsg_t*)call->arg);
            }
            else {
                ((pmsg_t*)call->arg)->size = 0;
            }

            // "Message size" retval is taken care by the call function.
            // b/c this call may not have the desired retval when its called
            // i.e. when there is no message to receive
            call->retval = 0;
        } break;

        case TERMINATE: {
            k_Terminate();
        } break;

        case SEND_USER: {
            if (GetBit(IO.active_pid, running->id) && !IO.output_off) {
                UART0_puts((char*)call->arg);
                call->retval = strlen((char*)call->arg);
            }
        } break;

        case RECV_USER: {
            if (GetBit(IO.active_pid, running->id) && IO.in_proc == 0) {
                IO.in_proc = running->id;
                IO.proc_inbuf = (char*)call->arg[0];
                IO.inbuf_max = (size_t)call->arg[1];
                IO.ret_size = (size_t*)&call->retval;
                IO.output_off = true;

                UnlinkPCB(running);
                running->state = BLOCKED;
                PendSV();
            }
            else {
                call->retval = 0;
            }
        } break;

        default: {

        } break;
    }
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

    // 4. Remove it from IO active PID bitmap
    ClearBit(IO.active_pid, running->id);

    // 5. Schedule a new process
    running = Schedule();
    SetPSP((uint32_t)running->sp);
    running->timer = PROC_RUNTIME;

    // 6. Reset the System timer
    SysTick_Reset();
}



