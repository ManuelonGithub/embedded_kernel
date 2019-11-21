/**
 * @file    k_scheduler.h
 * @brief   Defines all the functions and entities related to kernel scheduling.
 * @details This module should not be exposed to user programs.
 * @author  Manuel Burnay
 * @date    2019.10.23  (Created)
 * @date    2019.11.21  (Last Modified)
 */

#ifndef     K_SCHEDULER_H
#define     K_SCHEDULER_H

#include "k_types.h"

void scheduler_init();
bool LinkPCB(pcb_t *newPCB, priority_t proc_lvl);
void UnlinkPCB(pcb_t* pcb);
pcb_t* Schedule();

#endif	//  K_SCHEDULER_H
