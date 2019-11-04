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


int ProcessCreate(uint32_t pid, uint32_t priority, void (*proc_program)());
void terminate(void);
uint32_t getpid(void);
uint32_t nice(uint32_t newPriority);


#endif // CALLS_H
