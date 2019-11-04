/**
 * @file    k_scheduler.c
 * @brief   Contains all the supporting functionality to schedule process in the kernel.
 * @details This module should not be exposed to user programs.
 * @author  Manuel Burnay
 * @date    2019.10.23  (Created)
 * @date    2019.11.03  (Last Modified)
 */

#include <stdio.h>
#include <stdbool.h>
#include "k_scheduler.h"

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
 * @return  PCB link return code. See pcb_link_code_t for more details.
 * @details This function is used to also move PCBs in and out of the blocked queue
 *          and to place the idle process in the idle process queue.
 *          This poses a potential risk that processes may be initialized with a "priority"
 *          lower than what is allowed, but that will only cause that process to never run.
 */
pcb_handle_code_t LinkPCB(pcb_t *PCB, uint32_t proc_lvl)
{
    if (proc_lvl > PROCESS_QUEUES)  return INVALID_PRIORITY;

    pcb_t* front = ProcessQueue[proc_lvl];

    /*
     * If the process was previously linked to other PCBs,
     * Severe those links before moving the PCB to a new queue.
     */
    if (PCB->next != NULL && PCB->prev != NULL) {
        if (PCB->next == PCB || PCB->prev == PCB) { // The PCB is the only element in the queue.
            ProcessQueue[proc_lvl] = NULL;
        }
        else {
            PCB->next->prev = PCB->prev;
            PCB->prev->next = PCB->next;
        }
    }

    if (front == NULL) {    // If the queue where the PCB is being moved to is empty.
        ProcessQueue[proc_lvl] = PCB;
        PCB->next = PCB;
        PCB->prev = PCB;
    }
    else {
        PCB->next = front;
        PCB->prev = front->prev;
        front->prev->next = PCB;
        front->prev = PCB;
    }

    PCB->priority = proc_lvl;

    return HANDLE_SUCCESS;
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
    }                                       // Which if not then this is weird self-prank

    return retval;
}



