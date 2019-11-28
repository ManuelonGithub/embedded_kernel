/**
 * @file    calls.h
 * @brief   Defines all kernel calls that the user has access to.
 * @details This is the module that opens the kernel to the user programs.
 *          This includes kernel calls and process creation.
 * @author  Manuel Burnay
 * @date    2019.10.22  (Created)
 * @date    2019.11.21  (Last Modified)
 */

#ifndef CALLS_H
#define CALLS_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "k_types.h"

inline k_ret_t kcall(k_code_t code, k_arg_t arg);

pid_t pcreate(process_attr_t* attr, void (*proc_program)());
void terminate(void);
pid_t getpid(void);

priority_t nice(priority_t newPriority);

pmbox_t bind(pmbox_t box);
pmbox_t unbind(pmbox_t box);

size_t send(pmbox_t dst, pmbox_t src, uint8_t* data, uint32_t size);
size_t send_sync(pmbox_t dst, pmbox_t src, uint8_t* data, uint32_t size);

size_t recv(pmbox_t dst, pmbox_t src, uint8_t* data, uint32_t size);
size_t recv_async(pmbox_t dst, pmbox_t src, uint8_t* data, uint32_t size);

size_t send_user(char* str);
size_t recv_user(char* buf, uint32_t max_size);

#endif // CALLS_H
