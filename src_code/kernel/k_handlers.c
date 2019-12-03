/**
 * @file    k_handlers.c
 * @brief   Contains all functions pertaining towards
 *          the kernel's trap handlers and initializer functions.
 * @details This module should not be exposed to user programs.
 * @author  Manuel Burnay
 * @date    2019.10.23 (Created)
 * @date    2019.11.29 (Last Modified)
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "k_handlers.h"
#include "k_scheduler.h"
#include "k_processes.h"
#include "k_cpu.h"
#include "k_messaging.h"
#include "dlist.h"
#include "uart.h"
#include "systick.h"
#include "k_terminal.h"

pcb_t *running, *pTerminal, *pIdle;
uart_descriptor_t uart;

extern pcb_t   proc_table[PID_MAX];
extern pmsgbox_t msgbox[BOXID_MAX];

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
    LinkPCB(pTerminal, PRIV0_PRIORITY);
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
        KernelCall_handler(GetCallReg());
        RestoreContext();
    }
    else {                      // Trap was called by a process
        SaveProcessContext();   // So the process' context is saved
        KernelCall_handler(GetCallReg());
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
void KernelCall_handler(k_call_t* call)
{
    switch(call->code) {
        case PCREATE: {
            call->retval = k_pcreateCall((pcreate_args_t*)call->arg);
        } break;

        case STARTUP: {
            running = Schedule();
            // Initializes the process stack pointer to the idle process stack
            SetPSP((uint32_t)running->sp);

            RestoreProcessContext();

            running->timer = PROC_RUNTIME;

            // Reset the System timer
            SysTick_Reset();
            SysTick_Start();

            StartProcess();
        } break;

        case GETPID: {
            call->retval = getpidCall();
        } break;

        case NICE: {
            call->retval = niceCall((priority_t*)call->arg);
        } break;

        case BIND: {
            call->retval = k_bindCall((pmbox_t*)call->arg);
        } break;

        case UNBIND: {
            call->retval = k_unbindCall((pmbox_t*)call->arg);
        } break;

        case GETBOX: {
            call->retval = k_getboxCall();
        } break;

        case SEND: {
            k_sendCall((pmsg_t*)call->arg, &call->retval);
        } break;

        case RECV: {
            k_recvCall((pmsg_t*)call->arg, &call->retval);
        } break;

        case REQUEST: {
            k_requestCall((request_args_t*)call->arg, &call->retval);
        } break;

        case TERMINATE: {
            k_Terminate();
        } break;

        case GET_NAME: {
            k_getnameCall((char*)call->arg);
        } break;

        case SET_NAME: {
            k_setnameCall((char*)call->arg);
        } break;

        default: {
        } break;
    }
}

/**
 * @brief   Performs all operations required for process allocation.
 * @param   [in] arg: pointer to a pcreate arguments structure.
 * @return  Process ID of allocated process.
 *          PROC_ERR if allocation failed.
 */
inline pid_t k_pcreateCall(pcreate_args_t* arg)
{
    return k_pcreate(arg->attr, arg->proc_program, &terminate);
}

/**
 * @brief   Performs all operations required for retrieving the running process' ID.
 * @return  Running process' ID.
 */
inline pid_t getpidCall()
{
    return (pid_t)running->id;
}

/**
 * @brief   Performs all operations required for changing the user process' priority.
 * @param   new: pointer to new priority value.
 * @return  Running process' priority after all operations are complete.
 * @details This function ensures the user process doesn't change
 *          to an invalid/unallowed priority.
 */
inline priority_t niceCall(priority_t* new)
{
    if ((*new) > PRIV1_PRIORITY && (*new) < PRIORITY_LEVELS) {
        LinkPCB(running, (*new));
    }

    PendSV();

    return running->priority;
}

/**
 * @brief   Performs all operations required for binding
 *          a message box to running process.
 * @param   [in] pointer to box ID to be used in binding procedure.
 * @returns Box ID bound to process.
 *          BOX_ERR if bound procedure failed.
 */
inline pmbox_t k_bindCall(pmbox_t* box)
{
    return k_MsgBoxBind((*box), running);
}

/**
 * @brief   Performs all operations required for unbinding
 *          a message box to running process.
 * @param   [in] pointer to box ID to be used in unbinding procedure.
 * @returns 0 if unbinding procedure was successful,
 *          supplied box ID otherwise.
 */
inline pmbox_t k_unbindCall(pmbox_t* box)
{
    return k_MsgBoxUnbind((*box), running);
}

/**
 * @brief   Performs all operations required to retrieve a
 *          bound message box to the running process.
 * @returns Box ID of a box bound to process.
 *          BOX_ERR if no boxes are bound to process.
 */
inline pmbox_t k_getboxCall()
{
    pmbox_t retval = FindSet(running->owned_box, 0, BOXID_MAX);
    if (retval == BOXID_MAX) retval = (pmbox_t)BOX_ERR;

    return retval;
}

/**
 * @brief   Performs all operations required to send a message from a message
 *          box belonging to the running process to another message box.
 * @param   [in] msg: message to send to a message box.
 * @param   [out] retsize: number of bytes successfully sent.
 */
inline void k_sendCall(pmsg_t* msg, size_t* retsize)
{
    if (msg->dst < BOXID_MAX && msgbox[msg->src].owner == running) {
        k_MsgSend(msg, retsize);
    }
    else {
        *retsize = 0;
    }
}

/**
 * @brief   Performs all operations required to receive a message from a message
 *          box to a message box belonging to the running process.
 * @param   [out] msg: destination of message to be received from a message box.
 * @param   [out] retsize: number of bytes successfully received.
 */
inline void k_recvCall(pmsg_t* msg, size_t* retsize)
{
    if (msg->dst < BOXID_MAX && msgbox[msg->dst].owner == running) {
        k_MsgRecv(msg, retsize);
    }
    else {
        *retsize = 0;
    }
}

/**
 * @brief   Performs all operations required to perform the request transaction
 *          between a message box belonging to the running process to another.
 * @param   [in,out] args: Request transaction arguments.
 * @param   [out] retsize:
 *              number of bytes successfully received on the reply message.
 */
inline void k_requestCall(request_args_t* arg, size_t* retsize)
{
    if (arg->req_msg->dst < BOXID_MAX &&
            msgbox[arg->req_msg->src].owner == running) {

        k_MsgSend(arg->req_msg, retsize);
        if ((*retsize) != 0) {
            k_MsgRecv(arg->ret_msg, retsize);
        }
    }
    else {
        (*retsize) = 0;
    }
}

/**
 * @brief   Performs all operations required to
 *          retrieve the name of the running process.
 * @param   [out] Pointer to character string to copy the process' name to.
 */
inline void k_getnameCall(char* str)
{
    strcpy(running->name, str);
}

/**
 * @brief   Performs all operations required to
 *          set the name of the running process.
 * @param   [out] Pointer to character string to set the process' name to.
 */
inline void k_setnameCall(char* str)
{
    if (strlen(str) < 31)   strcpy(str, running->name);
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

/**
 * @brief   Generic Idle process used by the kernel.
 */
void idle()
{
    while (1) {}
}



