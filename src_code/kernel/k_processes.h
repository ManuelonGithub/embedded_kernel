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

pcb_t* k_AllocatePCB(pid_t id);
inline void k_DeallocatePCB(pid_t id);

pcb_t* k_CreatePCB(pid_t id);

pcb_t* GetPCB(pid_t id);

pid_t FindFreePID();
inline void SetPIDbit(pid_t pid);
inline void ClearPIDbit(pid_t pid);
inline bool AvailablePID(pid_t pid);

#endif	//  K_PROCESSES_H
