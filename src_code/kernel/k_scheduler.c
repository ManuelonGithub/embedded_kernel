/**
 * @file    k_scheduler.c
 * @brief   Contains all the supporting functionality to schedule process in the kernel.
 * @details This module should not be exposed to user programs.
 * @author  Manuel Burnay
 * @date    2019.10.23  (Created)
 * @date    2019.11.03  (Last Modified)
 */

#include <stdio.h>
#include "k_scheduler.h"
#include "double_link_list.h"

pcb_t* ProcessQueue[PROCESS_QUEUES];

/**
 * @brief   Initializes the Process Queue's "heads" to null.
 * @todo    Check if since it's a global variable that all values are null from the getgo.
 *          If so this function is not needed.
 */
void scheduler_init()
{
    int i;
    for (i = 0; i < PROCESS_QUEUES; i++) {
        ProcessQueue[i] = NULL;
    }
}

/**
 * @brief   Links a PCB into a specific priority queue.
 * @param   [in, out] PCB: pointer to PCB element to link into the respective process queue.
 * @return  False if process wasn't able be linked (invalid priority level)
 *          True if it was successfully linked.
 * @details This function is used to also move PCBs in and out of the blocked queue
 *          and to place the idle process in the idle process queue.
 *          This poses a potential risk that processes may be initialized with a "priority"
 *          lower than what is allowed, but that will only cause that process to never run.
 */
bool LinkPCB(pcb_t *PCB, uint32_t proc_lvl)
{
    if (proc_lvl > PROCESS_QUEUES)  return false;

    /*
     * If the process was previously linked to other PCBs,
     * Severe those links before moving the PCB to a new queue.
     */
    if (PCB->next != NULL && PCB->prev != NULL) {
        UnlinkPCB(PCB);
    }

    if (ProcessQueue[proc_lvl] == NULL) {    // If the queue where the PCB is being moved to is empty.
        ProcessQueue[proc_lvl] = PCB;
        PCB->next = PCB;
        PCB->prev = PCB;
    }
    else {
        list_link((node_t*)PCB, (node_t*)ProcessQueue[proc_lvl]);
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

    list_unlink((node_t*)pcb);
}

/**
 * @brief   Determines which PCB should run next.
 * @return  Pointer to PCB of the next process that should run.
 * @details This function does not perform any process switching.
 *          It simply iterates through the process queues to find
 *          the next available process to run.
 * @todo    Take in the running process so it can be compared with the selected process
 *          b/c if the selected process has the same priority as the running process
 *          and the running process' time allotted run time hasn't elapsed,
 *          then a new process shouldn't be selected.
 */
pcb_t* Schedule()
{
    pcb_t* front;
    pcb_t* retval = NULL;
    int i = 0;

    while (i < PRIORITY_LEVELS && retval == NULL) {
        front = ProcessQueue[i];
        if (front != NULL) {        // If this priority queue isn't empty.
            // Check if running process in this priority
            // If so check its run quantum
            // If it hasn't run out, don't make the switch

            retval = front;         // The process in front of queue is determined to run next.
            ProcessQueue[i] = front->next;    // The front of queue then moves to the next process to run.
        }
        i++;
    }

    if (retval == NULL) {
        retval = ProcessQueue[IDLE_LEVEL];  // We are assuming here that there will always be an idle process.
    }                                       // Which if not then this is a weird self-prank

    return retval;
}

/**
 * @brief   Finds the highest priority level with processes in its queue.
 * @return  -1 if no queue has a process in them,
 *          otherwise the highest priority level with a process.
 */
inline int32_t GetHighestPriority()
{
    int i = 0;
    while (i < PRIORITY_LEVELS) {
        if (ProcessQueue[i] != NULL)    return i;   // Stops progression of function if a process if found.
        i++;                                        // Makes for some ugly code, but efficient.
    }

    return NO_PROCESS_FOUND;
}

