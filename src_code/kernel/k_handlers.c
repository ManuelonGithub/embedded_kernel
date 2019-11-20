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
#include "k_handlers.h"
#include "k_scheduler.h"
#include "k_processes.h"
#include "calls.h"
#include "k_cpu.h"
#include "k_msgbox.h"
#include "dlist.h"
#include "uart.h"

pcb_t* running;
pmsgbox_t* free_box;
pmsgbox_t mbox[SYS_MSGBOXES];
pid_bitmap_t pid_bitmap;
uart_descriptor_t uart;

void KernelCall_handler(k_call_t* call);

pid_t FindFreePID();
inline void SetPIDbit(pid_t pid);
inline void ClearPIDbit(pid_t pid);
inline bool AvailablePID(pid_t pid);

/**
 * @brief   Generic Idle process used by the kernel.
 */
void idle()
{
    while(1) {}
}

void output_proc()
{
    pmbox_t box = bind(SYS_MSGBOXES-1);
//    uint8_t buffer[CIRCULAR_BUFFER_SIZE];
//    uint32_t msg_head, msg_tail;

    circular_buffer_t buffer;
    circular_buffer_init(&buffer);


    while(1) {
        buffer.wr_ptr = recv(box, 0, (uint8_t*)buffer.data, CIRCULAR_BUFFER_SIZE);

        while (buffer.rd_ptr < buffer.wr_ptr) {
            buffer.rd_ptr +=
                    UART0_put((buffer.data+buffer.rd_ptr), buffer_size(&buffer));
        }

        buffer.wr_ptr = 0;
        buffer.rd_ptr = 0;
    }

}

/**
 * @brief   Initializes kernel data structures, drivers, and critical processes.
 */
void kernel_init()
{
    PendSV_init();

    SystemTick_init(1000);  // 1000 Hz rate -> system tick triggers every milisecond

    uart.echo = false;
    UART0_Init(&uart);

    scheduler_init();

    int i;

    free_box = &mbox[0];
    mbox[0].ID = 0;

    for (i = 1; i < SYS_MSGBOXES; i++) {
        mbox[i].ID = i;
        mbox[i-1].next = &mbox[i];
        mbox[i].prev = &mbox[i-1];
    }

    pcreate(0, IDLE_LEVEL, &idle);
    // Set the initial running process to be running
    // This avoids any "NULL" checks of the running process,
    // As it'll always be associated with a process.
    running = Schedule();

	// register the server processes
    pcreate(0, 0, &output_proc);
}

/**
 * @brief   Starts the kernel's run-mode.
 * @details When a kernel is in run mode, user processes are able to run.
 */
inline void kernel_start()
{
    k_call_t call;
    call.code = STARTUP;

    k_SetCall(&call);

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
    SystemTick_pause();

    SaveProcessContext();
    running->sp = (uint32_t*)GetPSP();

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
            call->retval = k_pcreate((pcreate_args_t*)call->arg);
        } break;

        case STARTUP: {
            /*
             * This block of code is here to counter-act what PendSV would do to a
             * Non-initialized process.
             * While admittedly a bit ugly, it does allow for both PendSV and this Call handler
             * to be as efficient as possible at "steady-state" operation.
             * Here we're making the assumption that running is set to be the idle process.
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

        case GETID: {
            call->retval = (int32_t)running->id;
        } break;

        case NICE: {
            call->retval = LinkPCB(running, (priority_t)call->arg[0]);
            PendSV();
        } break;

        case BIND: {
            call->retval = k_Bind((pmbox_t)call->arg[0]);
        } break;

        case UNBIND: {
            call->retval = k_Unbind((pmbox_t)call->arg[0]);
        } break;

        case SEND: {
            call->retval = k_SendMessage((pmsg_t*)call->arg);
        } break;

        case RECV: {
            if (!k_ReceiveMessage((pmsg_t*)call->arg)) {
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
 * @param   [in] proc_program: Pointer to start of the program the process will execute.
 * @retval  Returns 0 if the process was successfully created.
 *          -1 If the process' creation was unsuccessful,
 *
 *          it'll return a negative value detailing the error source:
 *          -1 if bad process attribute (PID or priority)
 *          -2 if process wasn't able to be allocated in memory
 */
proc_t k_pcreate(pcreate_args_t* args)
{
    pcb_t* pcb = NULL;
    proc_t retval = 0;

    if (args->id == 0)  args->id = FindFreePID();

    // The ladder to process creation success!
    if (args->id < PID_MAX && AvailablePID(args->id)) {                     // PID is valid
        if (k_CreatePCB(&pcb, args->id)) {             // PCB was successfully allocated
            InitProcessContext(&pcb->sp, args->proc_program, &terminate);

            if (LinkPCB(pcb, args->prio)) {    // PCB was successfully linked into its queue
                retval = args->id;  // set return value to ID
                SetPIDbit(args->id);
            }
            else {
                k_DeletePCB(&pcb);
            }
        }
    }

    return retval;
}

/**
 * @brief   Binds a message box to the running process.
 * @param   [out] b: pointer to a valid msgbox id variable.
 *          This variable gets the id of the msgbox that was successfully
 *          bound to the process, which is then used when the process wishes
 *          to send/receive messages from the msgbox.
 * @param   [in] box_no: The message box number to bind to the process.
 * @return  -1 if a message box wasn't able to be bound,
 *          0 if a successful bind was performed.
 * @details If the message box value passed is 0,
 *          this procedure will instead find
 *          an available message box to bind to the process.
 *          If an available message box isn't found,
 *          or the requested msgbox is already taken,
 *          no message box will be bound to the process.
 */
pmbox_t k_Bind(pmbox_t id)
{
    pmsgbox_t* box;

    if (id == 0 && free_box != NULL ) {
        box = free_box;
        free_box = free_box->next;
    }
    else if (id < SYS_MSGBOXES && mbox[id].owner == NULL) {
        box = &mbox[id];
        // Move free list pointer if it pointed to msgbox
        if (&mbox[id] == free_box)    free_box = free_box->next;
    }
    else    return 0;   // Wasn't able to bind mailbox, return 0

    k_MsgBoxBind(box, running);
    id = box->ID;

    return id;
}

/**
 * @brief   Unbinds a msgbox from the running process.
 * @param   [in] mbox_no: msgbox number to be unbound from the running process.
 * @return  -1 if the msgbox wasn't able to be unbound,
 *          otherwise the msgbox number for the unbound msgbox is returned.
 * @details This procedure also makes sure that all messages in the msgbox are erased.
 *          Unlike the binding procedure, it does require an explicit msgbox value owned
 *          by the running process in order to unbind it.
 */
pmbox_t k_Unbind(pmbox_t id)
{
    if (id < SYS_MSGBOXES && mbox[id].owner == running) {
        k_MsgBoxUnbind(&mbox[id]);
        if (free_box == NULL) {
            mbox[id].list.next = NULL;
            mbox[id].list.prev = NULL;
            free_box = &mbox[id];
        }
        else {
            dLink(&mbox[id].list, &free_box->list);
        }

        // the free_box list is a FIFO,
        // so the "newest" available box is at the front of the list
        free_box = &mbox[id];
        id = 0;
    }

    return id;
}

size_t k_SendMessage(pmsg_t* msg)
{
    bool err = (msg->dst >= SYS_MSGBOXES     || msg->src >= SYS_MSGBOXES  ||
                mbox[msg->dst].owner == NULL || mbox[msg->src].owner != running);

    uint32_t retval = 0;

    if (!err) {
        if (mbox[msg->dst].wait_msg != NULL) {  // todo: check if wait msg's src is the dst's mailbox
            retval = k_pMsgRecv(mbox[msg->dst].wait_msg, msg);
            LinkPCB(mbox[msg->dst].owner, mbox[msg->dst].owner->priority);
            PendSV();
        }
        else {
            // Allocate Message
            pmsg_t* msg_out = k_pMsgAllocate(msg->data, msg->size);
            msg_out->dst = msg->dst;
            msg_out->src = msg->src;
            // Send if message allocation was successful
            if (msg_out != NULL)    retval = k_pMsgSend(msg_out, &mbox[msg->dst]);
        }
    }

    return retval;
}

bool k_ReceiveMessage(pmsg_t* dst_msg)
{
    // Initialized will all possible error conditions checked
    bool retval = (
        dst_msg == NULL || dst_msg->dst >= SYS_MSGBOXES ||
        mbox[dst_msg->dst].owner != running
    );

    // If no errors were found
    if (!retval) {
        if (mbox[dst_msg->dst].front_msg == NULL) {
            mbox[dst_msg->dst].wait_msg = dst_msg;
            retval = false;
        }
        else {
            // todo: create a "specified source" check system

            pmsg_t* src_msg = mbox[dst_msg->dst].front_msg;

            mbox[dst_msg->dst].front_msg = mbox[dst_msg->dst].front_msg->next;

            if (mbox[dst_msg->dst].front_msg == src_msg) {
                mbox[dst_msg->dst].front_msg = NULL;
            }
            else {
                dUnlink(&src_msg->list);
            }

            k_pMsgRecv(dst_msg, src_msg);
            k_pMsgDeallocate(&src_msg);

            retval = true;
        }
    }

    return retval;
}

void k_Terminate()
{
    // 1. Unlink process from its process queue
    UnlinkPCB(running);

    pmsgbox_t* box;
    // 2. Unbind all message boxes from process
    while (running->msgbox != NULL) {
        box = running->msgbox;
        running->msgbox = running->msgbox->next;
        k_Unbind(box->ID);
    }

    // 3. Erase PCB
    k_DeletePCB(&running);

    // 4. Schedule a new process
    running = Schedule();
    SetPSP((uint32_t)running->sp);
    running->timer = PROC_RUNTIME;

    // 5. Reset the System timer
    SystemTick_reset();
}

// todo: Move this business to k_processes module
pid_t FindFreePID()
{
    pid_t i = 0;
    bool pid_found = false;
    while (i < PID_MAX && !pid_found) {
        if (!AvailablePID(i))   i++;
        else                    pid_found = true;
    }

    return i;   // pid 0 indicates there is no available PID (PID 0 is always the idle process).
}

inline void SetPIDbit(pid_t pid)
{
    /*
     * bitmap is comprised of an array of bytes (smallest addressable element).
     * So it find a specific bit within the bitmap, two values need to be derived:
     * (pid >> 3) derives the byte index of the bitmap associated with the bit (pid).
     * (1 << (pid & 7)) derives the location of the bit (pid) within the addressed byte.
     */
    pid_bitmap.byte[(pid >> 3)] |= (1 << (pid & 7));
}

inline void ClearPIDbit(pid_t pid)
{
    pid_bitmap.byte[(pid >> 3)] &= ~(1 << (pid & 7));
}

inline bool AvailablePID(pid_t pid)
{
    return !(pid_bitmap.byte[pid >> 3] & (1 << (pid & 7)));
}




