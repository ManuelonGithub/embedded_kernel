/**
 * @file    k_scheduler.c
 * @brief   Contains The System's process queues and all the supporting
 *          functionality to schedule process in the kernel.
 * @details This module should not be exposed to user programs.
 * @author  Manuel Burnay
 * @date    2019.10.23  (Created)
 * @date    2019.11.21  (Last Modified)
 */

#include <stdio.h>
#include "k_scheduler.h"
#include "dlist.h"

pcb_t* ProcessQueue[PROCESS_QUEUES];

/**
 * @brief   Links a PCB into a specific priority queue.
 * @param   [in,out] PCB:
 *              pointer to PCB element to link into the respective process queue.
 * @return  False if process wasn't able be linked (invalid priority level)
 *          True if it was successfully linked.
 * @details This function is also used to place the
 *          idle process in the idle process queue.
 *          This poses a potential risk that processes
 *          may be initialized with a "priority" lower than what is allowed,
 *          but that will only cause that process to never run.
 */
bool LinkPCB(pcb_t *PCB, priority_t proc_lvl)
{
    if (proc_lvl > PROCESS_QUEUES)  return false;

    /*
     * If the process was previously linked to other PCBs,
     * Severe those links before moving the PCB to a new queue.
     */
    if (PCB->next != NULL && PCB->prev != NULL) {
        UnlinkPCB(PCB);
    }

    if (ProcessQueue[proc_lvl] == NULL) {
        // If the queue where the PCB is being moved to is empty.
        ProcessQueue[proc_lvl] = PCB;
        PCB->next = PCB;
        PCB->prev = PCB;
    }
    else {
        dLink(&PCB->list, &ProcessQueue[proc_lvl]->list);
    }

    PCB->priority = proc_lvl;

    return true;
}

/**
 * @brief   Unlinks PCB from its Process queue.
 * @param   [in, out] pcb: Pointer to PCB to be unlinked from its queue.
 * @details This function can only be called if the pcb has an established priority,
 *          otherwise it will fault.
 */
void UnlinkPCB(pcb_t* pcb)
{
    if (ProcessQueue[pcb->priority] == pcb) {
        if (pcb == pcb->next)   ProcessQueue[pcb->priority] = NULL;
        else                    ProcessQueue[pcb->priority] = pcb->next;
    }

    dUnlink(&pcb->list);
}

/**
 * @brief   Determines which PCB should run next.
 * @return  Pointer to PCB of the next process that should run.
 * @details This function does not perform any process switching.
 *          It simply iterates through the process queues to find
 *          the next available process to run.
 * @todo    Remove the IDLE process queue
 */
pcb_t* Schedule()
{
    pcb_t* front;
    pcb_t* retval = NULL;
    int i = 0;

    while (i < PRIORITY_LEVELS && retval == NULL) {
        front = ProcessQueue[i];
        if (front != NULL) {    // If this priority queue isn't empty.
            // The process in front of queue is determined to run next.
            retval = front;

            // The front of queue then moves to the next process to run.
            ProcessQueue[i] = front->next;
        }
        i++;
    }

    if (retval == NULL) {
        // We are assuming here that there will always be an idle process.
        // Which if not then this is a weird self-prank
        retval = ProcessQueue[IDLE_LEVEL];
    }

    return retval;
}

