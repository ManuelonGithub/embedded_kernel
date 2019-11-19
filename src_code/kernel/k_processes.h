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

#define STACKSIZE	1024

#define PROC_RUNTIME    100

bool k_CreatePCB(pcb_t** pcb, pid_t id);
inline void k_DeletePCB(pcb_t** pcb);



#endif	//  K_PROCESSES_H
