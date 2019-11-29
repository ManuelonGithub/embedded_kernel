/**
 * @file    k_processes.h
 * @brief   Contains all functions and entities related to
 *          processes and process control blocks.
 * @details This module should not be exposed to user programs.
 * @author  Manuel Burnay
 * @date    2019.10.23 (Created)
 * @date    2019.10.23 (Last Modified)
 */

#ifndef     K_PROCESSES_H
#define     K_PROCESSES_H

#include "k_types.h"

void process_init();

pid_t k_pcreate(process_attr_t* attr, void (*program)(), void (*terminate)());
pcb_t* k_AllocatePCB(pid_t id);
inline void k_DeallocatePCB(pid_t id);

pcb_t* GetPCB(pid_t id);
void ChangeProcessPriority(pid_t id, priority_t new);

#endif	//  K_PROCESSES_H
