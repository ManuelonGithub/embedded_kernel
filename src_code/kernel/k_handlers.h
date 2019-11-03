/**
 * @file    k_handlers.h
 * @brief   Defines all the functions and entities pertaining towards
 *          the kernel's trap handlers and initializer functions.
 * @details This module should not be exposed to user programs.
 * @author  Manuel Burnay
 * @date    2019.10.23 (Created)
 * @date    2019.10.23 (Last Modified)
 */

#ifndef 	K_HANDLERS_H
#define		KERNEL_HANDLERS_H

#include <stdint.h>

void kernel_init();
inline void kernel_start();

uint32_t k_ProcessCreate(uint32_t pid, uint32_t priority, void (*proc_program)());

#endif  //	K_HANDLERS_H
