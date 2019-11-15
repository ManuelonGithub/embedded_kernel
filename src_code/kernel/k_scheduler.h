/**
 * @file    k_scheduler.h
 * @brief   Defines all the functions and entities related to kernel scheduling.
 * @details This module should not be exposed to user programs.
 * @author  Manuel Burnay
 * @date    2019.10.23  (Created)
 * @date    2019.11.02  (Last Modified)
 */

#ifndef     K_SCHEDULER_H
#define     K_SCHEDULER_H

#include "k_types.h"

#define PRIORITY_LEVELS 5   /** Priority levels supported by the kernel. */
#define IDLE_LEVEL      5   /** Index to the Idle queue */

#define LOWEST_PRIORITY 4
/**
 * @brief   Total amount of process levels the kernel scheduler accepts.
 * @details Five queues for the priority levels,
 *          and then one more for the idle process.
 */
#define PROCESS_QUEUES  PRIORITY_LEVELS+1

enum PCB_LINK_RETURN_VALUES {INVALID_PRIORITY = -1, LINK_SUCCESS};

#define NO_PROCESS_FOUND -1 /// Value returned if there was a query into a processes queue but no process was found.

void scheduler_init();
int32_t LinkPCB(pcb_t *newPCB, uint32_t proc_lvl);
void UnlinkPCB(pcb_t* pcb);
pcb_t* Schedule();

#endif	//  K_SCHEDULER_H
