/**
 * @file    Kernel_Scheduler.h
 * @brief   Defines all the functions and entities related to kernel scheduling.
 * @details This module should not be exposed to user programs.
 * @author  Manuel Burnay
 * @date    2019.10.23  (Created)
 * @date    2019.10.23  (Last Modified)
 */

#ifndef     KERNEL_SCHEDULER_H
#define     KERNEL_SCHEDULER_H

#include "Kernel_Processes.h"

#define PRIORITY_LEVELS 5   /** Priority levels supported by the kernel. */
#define IDLE_LEVEL      5   /** Index to the Idle queue */
#define BLOCKED_LEVEL   6   /** Index to the Blocked queue */

/**
 * @brief   Total amount of process queues the kernel scheduler contains.
 * @details Five queues for the priority levels,
 *          and then two more for the "Idle" process and the block process queue.
 */
#define PROCESS_QUEUES  PRIORITY_LEVELS+2

void scheduler_init();
void LinkPCB(pcb_t *newPCB, uint32_t priority);


#endif	//  KERNEL_SCHEDULER_H
