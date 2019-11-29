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
#include "calls.h"

void kernel_init();
inline void kernel_start();

inline pid_t k_pcreateCall(pcreate_args_t* arg);
inline pid_t getpidCall();
inline priority_t niceCall(priority_t* new);
inline pmbox_t k_bindCall(pmbox_t* box);
inline pmbox_t k_unbindCall(pmbox_t* box);
inline pmbox_t k_getboxCall();
inline void k_sendCall(pmsg_t* msg, size_t* retsize);
inline void k_recvCall(pmsg_t* msg, size_t* retsize);
inline void k_requestCall(request_args_t* arg, size_t* retsize);

void k_Terminate();

size_t k_SendUser(char* str);
size_t k_UserRecv(char* dst, uint32_t max_size);

#endif  //	K_HANDLERS_H
