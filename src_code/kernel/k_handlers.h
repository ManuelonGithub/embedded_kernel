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
#include "k_types.h"
#include "k_calls.h"

void kernel_init();
inline void kernel_start();

proc_t k_pcreate(pcreate_args_t* args);
pmbox_t k_Bind(pmbox_t id);
pmbox_t k_Unbind(pmbox_t id);
size_t k_SendMessage(pmsg_t* msg);
bool k_ReceiveMessage(pmsg_t* dst_msg);

#endif  //	K_HANDLERS_H
