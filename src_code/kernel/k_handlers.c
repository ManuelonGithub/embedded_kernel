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
#include "k_calls.h"
#include "calls.h"
#include "k_cpu.h"
#include "k_mailbox.h"
#include "double_link_list.h"

pcb_t* running;
pmsgbox_t* free_mbox;
pmsgbox_t mbox[SYS_MAILBOXES];
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

    SystemTick_init(1000);  // 1000hz rate -> system tick triggers every milisecond

    scheduler_init();

    ProcessCreate(0, IDLE_LEVEL, &idle);
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
            call->retval = k_ProcessCreate(
                    call->argv[0],
                    call->argv[1],
                    call->argv[2],
                    (void(*)())(call->argv[3])
            );
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

        } break;

        case UNBIND: {

        } break;

        case SEND: {

        } break;

        case RECV: {

        } break;

        case TERMINATE: {
            // Unlink Process from Process Queue
            // Unbind any mailboxes
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
int32_t k_ProcessCreate(process_t* p, pid_t id, priority_t prio, void (*proc_program)())
{
    pcb_t* pcb;
    int32_t retval = -1;

    if (id == 0)  id = FindFreePID();

    // The ladder to process creation success!
    if (AvailablePID(id)) {                     // PID is valid
        if (k_CreatePCB(pcb, id)) {             // PCB was successfully allocated
            InitProcessContext(&newPCB->sp, proc_program, &terminate);

            if (LinkPCB(newPCB, priority)) {    // PCB was successfully linked into its queue
                retval = 0;
                SetPIDbit(id);
                if (p != NULL)  *p = id;
            }
            else {
                k_Delete_PCB();
            }
        }
    }

    return 0; // todo: give the ret value a #define
}

/**
 * @brief   Binds a mailbox to the running process.
 * @param   [in] mbox_no: The mailbox number to bind to the process.
 * @return  -1 if a mailbox wasn't able to be bound,
 *          otherwise the bound mailbox number is returned.
 * @details If the mailbox value passed is -1, this procedure will instead find
 *          an available mailbox to bind to the process.
 *          If an available mailbox isn't found, or the requested mailbox is already taken,
 *          no mailbox will be bound to the process.
 */
int32_t k_BindMailbox(uint32_t mbox_no)
{
    int32_t retval = -1;

    if (mbox_no == 0) {
        if (free_mbox != NULL) {        // If there are mailboxes left
            free_mbox->owner = running;
            retval = free_mbox->ID;
            mbox_no = free_mbox->ID;

            list_unlink((node_t*)free_mbox);
            free_mbox = free_mbox->next;

            // Linking mailbox to PCB (in case process gets terminated before mailboxes were unbound)
            if (running->msgbox == NULL) {     // Only maiblox bound to process
                running->msgbox = free_mbox;
            }
            else {
                list_link((node_t*)free_mbox, (node_t*)running->msgbox);
            }
        }
    }
    else if (mbox_no < SYS_MAILBOXES && mbox[mbox_no].owner == NULL) {
        mbox[mbox_no].owner = running;
        retval = mbox_no;

        list_unlink((node_t*)&mbox[mbox_no]);   // Unlink mailbox from the "free" list
        if (&mbox[mbox_no] == free_mbox)    free_mbox = free_mbox->next;    // Move free list pointer if it pointed to mailbox

        // Linking mailbox to PCB (in case process gets terminated before mailboxes were unbound)
        if (running->msgbox == NULL) {     // Only maiblox bound to process
            running->msgbox = &mbox[mbox_no];
        }
        else {
            list_link((node_t*)&mbox[mbox_no].owner, (node_t*)running->msgbox);
        }
    }

    return retval;
}

/**
 * @brief   Unbinds a mailbox from the running process.
 * @param   [in] mbox_no: Mailbox number to be unbound from the running process.
 * @return  -1 if the mailbox wasn't able to be unbound,
 *          otherwise the mailbox number for the unbound mailbox is returned.
 * @details This procedure also makes sure that all messages in the mailbox are erased.
 *          Unlike the binding procedure, it does require an explicit mailbox value owned
 *          by the running process in order to unbind it.
 */
int32_t k_UnbindMailbox(int32_t mbox_no)
{
    int32_t retval = -1;
    ipc_msg_t* msg;

    if (mbox_no < SYS_MAILBOXES && mbox[mbox_no].owner == running) {
        retval = mbox_no;

        // Erase pending messages if there are any
        if (mbox[mbox_no].front_msg != NULL) {
            msg = mbox[mbox_no].front_msg->next;
                                                     // Procedure unlinks message in front
            while (msg->next != msg) {               // In front of the "head" message in the MB
                list_unlink((node_t*)msg);           // And then erases it.
                free(msg);                           // Procedure keeps going until all but the head message is left
                msg = mbox[mbox_no].front_msg->next;
            }

            free(msg);                      // Head message is then erased here.
            mbox[mbox_no].front_msg = NULL;
        }
    }

    // Unlinking mailbox from the running's PCB
    if (running->msgbox == &mbox[mbox_no]) running->msgbox = mbox[mbox_no].next;
    list_unlink((node_t*)&mbox[mbox_no]);

    return retval;
}

int32_t k_SendMessage(int32_t dst, int32_t src, uint8_t* data, uint32_t size)
{
    // 1. Create msg out of size & msg_data,
    // 2. Place msg in the destination mailbox,
    // 3. If destination is blocked, unblock it and queue it
    if (dst >= SYS_MAILBOXES || src >= SYS_MAILBOXES ||
            mbox[dst].owner == NULL || mbox[src].owner != running
}

int32_t k_ReceiveMessage(int32_t dst, int32_t src, uint8_t* data, uint32_t size)
{
    // 1. Check if dst box has a message
    //   1.1. If so, memcpy data until there are no more bytes in message or size has reached
    //   1.2. If not:
    //       1.2.1. Remove process from the process queues by unlinking it
    //       1.2.2. Set the waiting flag on the mailbox
    //       1.2.3. Trigger PendSV.
    if (dst >= SYS_MAILBOXES || mbox[dst].owner != running) return -1;

    if (mbox[dst].front_msg == NULL) {
        // block process
        // 1. Unlink process from Process queue

        mbox[dst].waiting = true;
        PendSV();
    }
    else {
        ipc_msg_t* msg = mbox[dst].front_msg;
        if (msg->size < size)   size = msg->size;

        memcpy(data, msg->data, size);
    }

    return size;
}


void k_Terminate()
{
    // 1. Unlink process from its process queue
    // 2. Unbind all mailbox from process
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
    pid_bitmap[(pid >> 3)] |= (1 << (pid & 7));
}

inline void ClearPIDbit(pid_t pid)
{
    pid_bitmap[(pid >> 3)] &= ~(1 << (pid & 7));
}

inline bool AvailablePID(pid_t pid)
{
    return (pid_bitmap[pid >> 3] & (1 << (pid & 7)));
}




