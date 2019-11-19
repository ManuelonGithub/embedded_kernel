/**
 * @file    calls.h
 * @brief   Defines all kernel calls that the user has access to.
 * @details This is the module that opens the kernel to the user programs.
 *          This includes kernel calls and process creation.
 * @author  Manuel Burnay
 * @date    2019.10.22  (Created)
 * @date    2019.11.03  (Last Modified)
 */

#ifndef CALLS_H
#define CALLS_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "k_types.h"


int32_t pcreate(proc_t* p, pid_t pid, priority_t priority, void (*proc_program)());
void terminate(void);
pid_t getpid(void);
priority_t nice(priority_t newPriority);
int32_t bind(pmbox_t* box, uint32_t box_no);
int32_t unbind(pmbox_t* box);

#endif // CALLS_H
