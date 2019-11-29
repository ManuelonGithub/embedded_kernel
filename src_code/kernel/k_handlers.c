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
#include "k_cpu.h"
#include "k_messaging.h"
#include "dlist.h"
#include "uart.h"
#include "systick.h"
#include "k_terminal.h"

pcb_t *running, *pTerminal, *pIdle;
uart_descriptor_t uart;

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
            call->retval = k_pcreateCall((pcreate_args_t*)call->arg);
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

//        case SEND_USER: {
//            k_SendUser((char*)call->arg);
//        } break;

//        case RECV_USER: {
//            k_RecvUser()
//        } break;

        default: {

        } break;
    }
}

inline pid_t k_pcreateCall(pcreate_args_t* arg)
{
    return k_pcreate(arg->attr, arg->proc_program, &terminate);
}

inline pid_t getpidCall()
{
    return (pid_t)running->id;
}

inline priority_t niceCall(priority_t* new)
{
    if ((*new) > 0 && (*new) < PRIORITY_LEVELS) {
        LinkPCB(running, (*new));
    }

    PendSV();

    return running->priority;
}

inline pmbox_t k_bindCall(pmbox_t* box)
{
    return k_MsgBoxBind((*box), running);
}

inline pmbox_t k_unbindCall(pmbox_t* box)
{
    return k_MsgBoxUnbind((*box), running);
}

inline pmbox_t k_getboxCall()
{
    pmbox_t retval = FindSet(running->owned_box, 0, BOXID_MAX);
    if (retval == BOXID_MAX) retval = (pmbox_t)BOX_ERR;

    return retval;
}

inline void k_sendCall(pmsg_t* msg, size_t* retsize)
{
    if (msg->dst < BOXID_MAX && msgbox[msg->src].owner == running) {
        k_MsgSend(msg, retsize);
    }
    else {
        *retsize = 0;
    }
}

inline void k_recvCall(pmsg_t* msg, size_t* retsize)
{
    if (msg->dst < BOXID_MAX && msgbox[msg->dst].owner == running) {
        k_MsgRecv(msg, retsize);
    }
    else {
        *retsize = 0;
    }
}

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

//size_t k_SendUser(char* str)
//{
//    // Find a message box for the process to use
//    // Search the process' boxes
//    pmbox_t box = FindSet(running->owned_box, 0, BOXID_MAX);
//
//    if (box == BOXID_MAX) {
//        // Process has no msg box, assigning one to it
//        // todo: make this assignment temporary
//        box = k_MsgBoxBind(ANY_BOX, running);
//
//        if (box == (pmbox_t)BOX_ERR)    return 0;
//    }
//
//    // Set up metadata to send IO server
//    IO_metadata_t meta = {
//         .box_id = box,
//         .is_send = true,
//         .proc_id = running->id,
//         .size = strlen(str),
//         .send_data = (uint8_t*)str
//    };
//
//    // Set up request message
//    pmsg_t msg = {
//         .dst = IO_BOX,
//         .src = box,
//         .data = (uint8_t*)&meta,
//         .size = sizeof(IO_metadata_t),
//         .blocking = false
//    };
//
//    // Send IO request to server
//    if (k_MsgSend(&msg) != 0) {
//        // Request successfully sent.
//        // Set up request reply rx message
//        msg.dst = box;
//        msg.src = IO_BOX;
//        msg.data = NULL;
//        msg.size = strlen(str);
//        msg.blocking = true;
//
//        k_MsgRecv(&msg);
//    }
//
//    return 0;
//}

//size_t k_UserRecv(char* dst, uint32_t max_size)
//{
//    // Find a message box for the process to use
//    // Search the process' boxes
//    pmbox_t box = FindSet(running->owned_box, 0, BOXID_MAX);
//
//    if (box == BOXID_MAX) {
//        // Process has no msg box, assigning one to it
//        // todo: make this assignment temporary
//        box = k_MsgBoxBind(ANY_BOX, running);
//
//        if (box == (pmbox_t)BOX_ERR)    return 0;
//    }
//
//    // Set up metadata to send IO server
//    IO_metadata_t meta = {
//         .box_id = box,
//         .is_send = false,
//         .proc_id = running->id,
//         .size = max_size,
//         .send_data = NULL
//    };
//
//    pmsg_t msg = {
//         .dst = IO_BOX,
//         .src = box,
//         .data = (uint8_t*)&meta,
//         .size = sizeof(IO_metadata_t),
//         .blocking = false
//    };
//
//    // Send IO request to server
//    if (k_MsgSend(&msg) != 0) {
//        // Request successfully sent.
//        // Set up request reply rx message
//        msg.dst = box;
//        msg.src = IO_BOX;
//        msg.data = (uint8_t*)dst;
//        msg.size = max_size;
//        msg.blocking = true;
//
//        k_MsgRecv(&msg);
//    }
//    else {
//        msg.size = 0;
//    }
//
//    return msg.size;
//}



