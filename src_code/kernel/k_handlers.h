/**
 * @file    k_handlers.h
 * @brief   Defines all the functions and entities pertaining towards
 *          the kernel's trap handlers and initializer functions.
 * @details This module should not be exposed to user programs.
 * @author  Manuel Burnay
 * @date    2019.10.23 (Created)
 * @date    2019.11.21 (Last Modified)
 */

#ifndef 	K_HANDLERS_H
#define		KERNEL_HANDLERS_H

#include <stdint.h>
#include "k_types.h"
#include "k_calls.h"

typedef struct pcreate_args_ {
    pid_t       id;
    priority_t  prio;
    void        (*proc_program)();
} pcreate_args_t;

typedef struct active_IO_ {
    uint8_t active_pid[PID_MAX/8];
    uint8_t active_box[SYS_MSGBOXES/8];
    uint8_t input_queue[SYS_MSGBOXES/8];
} active_IO_t;

void kernel_init();
inline void kernel_start();

proc_t k_pcreate(pcreate_args_t* args);
void k_Terminate();

#endif  //	K_HANDLERS_H
