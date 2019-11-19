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
#include "k_mailbox.h"
#include "double_link_list.h"

pcb_t* running;
pmsgbox_t* free_box;
pmsgbox_t mbox[SYS_MSGBOXES];
pid_bitmap_t pid_bitmap;

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

/**
 * @brief   Initializes kernel data structures, drivers, and critical processes.
 */
void kernel_init()
{
    PendSV_init();

    SystemTick_init(1000);  // 1000 Hz rate -> system tick triggers every milisecond

    scheduler_init();

    int i;

    free_box = &mbox[0];
    mbox[0].ID = 0;

    for (i = 1; i < SYS_MSGBOXES; i++) {
        mbox[i].ID = i;
        mbox[i-1].next = &mbox[i];
    }

    pcreate(NULL, 0, IDLE_LEVEL, &idle);
    running = Schedule();   // This causes running to always be pointing to a valid process.
                            // This will make the "null" check in PendSV unnecessary

	// register the server processes
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
        if (running != NULL)    RestoreProcessContext();
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
            call->retval = k_pcreate((pcreate_args_t*)call->argv);
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
            call->retval = LinkPCB(running, call->argv[0]);
            PendSV();
        } break;

        case BIND: {
            call->retval = k_Bind((bind_args_t*)call->argv);
        } break;

        case UNBIND: {
            call->retval = k_Unbind((pmbox_t*)call->argv[0]);
        } break;

        case SEND: {

        } break;

        case RECV: {

        } break;

        case TERMINATE: {
            // Unlink Process from Process Queue
            // Unbind any message boxes
            // De-allocate stack
            // de-allocate pcb
            // set new running process

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
int32_t k_pcreate(pcreate_args_t* args)
{
    pcb_t* pcb = NULL;
    int32_t retval = -1;

    if (args->id == 0)  args->id = FindFreePID();

    // The ladder to process creation success!
    if (AvailablePID(args->id)) {                     // PID is valid
        if (k_CreatePCB(&pcb, args->id)) {             // PCB was successfully allocated
            InitProcessContext(&pcb->sp, args->proc_program, &terminate);

            if (LinkPCB(pcb, args->prio)) {    // PCB was successfully linked into its queue
                retval = 0;
                SetPIDbit(args->id);
                if (args->p != NULL)  *args->p = args->id;
            }
            else {
                k_DeletePCB(&pcb);
            }
        }
    }

    return retval; // todo: give the ret value a #define
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
int32_t k_Bind(bind_args_t* args)
{
    pmsgbox_t* box = NULL;

    /*
     * This function is structured differently than others
     * (I usually use 'retvals' instead of multiple return points)
     * to improve the efficiency of the function.
     */

    if (args->box == NULL)  return -1;  // A valid pointer to a msgbox_id is required.

    if (args->box_no == 0 && free_box != NULL ) {
        box = free_box;
        free_box = free_box->next;
    }
    else if (args->box_no < SYS_MSGBOXES && mbox[args->box_no].owner == NULL) {
        box = &mbox[args->box_no];
        if (&mbox[args->box_no] == free_box)    free_box = free_box->next;    // Move free list pointer if it pointed to msgbox
    }
    else    return -1;  // Invalid input | No available boxes | Box is taken

    k_MsgBoxBind(box, running);
    *(args->box) = box->ID;

    return 0;


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
int32_t k_Unbind(pmbox_t* box)
{
    int32_t retval = -1;
    int32_t box_no = *box;

    if (box_no < SYS_MSGBOXES && mbox[box_no].owner == running) {
        retval = 0;
        k_MsgBoxUnbind(&mbox[box_no]);
        if (free_box != NULL) {
            list_link((node_t*)&mbox[box_no], (node_t*)free_box);
        }

        free_box = &mbox[box_no];
    }

    return retval;
}

uint32_t k_SendMessage(int32_t dst, int32_t src, uint8_t* data, uint32_t size)
{
    bool err = (dst >= SYS_MSGBOXES     || src >= SYS_MSGBOXES  ||
                mbox[dst].owner == NULL || mbox[src].owner != running);

    uint32_t retval = 0;

    if (!err) {
        if (mbox[dst].waiting) {
            ipc_msg_t msg = {.next = NULL, .prev = NULL, .size = size, .data = data};
            k_call_t* dst_call = GetProcessCall(mbox[dst].owner->sp);
            recv_args_t* dst_args = (recv_args_t*)dst_call->argv;

            retval = k_IPCMsgRecv(&msg, dst_args->data, dst_args->size);

            LinkPCB(mbox[dst].owner, mbox[dst].owner->priority);
            PendSV();
        }
        else {
            // Create Message
            ipc_msg_t* msg = k_IPCMsgCreate(data, size);

            retval = k_IPCMsgSend(msg, &mbox[dst]);
        }
    }

    return retval;
}

uint32_t k_ReceiveMessage(int32_t dst, int32_t src, uint8_t* data, uint32_t size)
{
    // 1. Check if dst box has a message
    //   1.1. If so, memcpy data until there are no more bytes in message or size has reached
    //   1.2. If not:
    //       1.2.1. Remove process from the process queues by unlinking it
    //       1.2.2. Set the waiting flag on the msgbox
    //       1.2.3. Trigger PendSV.
    if (dst >= SYS_MSGBOXES || mbox[dst].owner != running) return 0;

    if (mbox[dst].front_msg == NULL) {
        // block process
        // 1. Unlink process from Process queue

        mbox[dst].waiting = true;
        PendSV();
    }
    else {
        ipc_msg_t* msg = mbox[dst].front_msg;
        mbox[dst].front_msg = mbox[dst].front_msg->next;

        k_IPCMsgRecv(msg, data, size);
        k_DeleteIPCMsg(&msg);
    }

    return size;
}


void k_Terminate()
{
    // 1. Unlink process from its process queue
    // 2. Unbind all message boxes from process
    // 3. Link process to the terminate queue
    // 4. PendSV
}

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




