/**
 * @file    Kernel_Handlers.h
 * @brief   Defines all the functions and entities pertaining towards
 *          the kernel's trap handlers and initializer functions.
 * @details This module should not be exposed to user programs.
 * @author  Manuel Burnay
 * @date    2019.10.23 (Created)
 * @date    2019.10.23 (Last Modified)
 */

#ifndef 	KERNEL_HANDLERS_H
#define		KERNEL_HANDLERS_H

void kernel_init();
inline void kernel_start();

#endif  //	KERNEL_HANDLERS_H
