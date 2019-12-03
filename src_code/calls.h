/**
 * @file    calls.h
 * @brief   Defines all kernel calls that the user has access to.
 * @details This is the module that opens the kernel to the user programs.
 *          This includes kernel calls and process creation.
 * @author  Manuel Burnay
 * @date    2019.10.22  (Created)
 * @date    2019.11.29  (Last Modified)
 */

#ifndef CALLS_H
#define CALLS_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "k_types.h"

/**
 * @brief   Argument structure of a process-create kernel call
 * @details Contains two arguments:
 *          attr: pointer to process attribute structure. Can be NULL.
 *          proc_program: pointer to reentrant function that the process will run.
 */
typedef struct pcreate_args_ {
    process_attr_t* attr;
    void (*proc_program)();
}pcreate_args_t;

/**
 * @brief   Argument structure of a message box bind kernel call
 * @details Contains two arguments:
 *          attr: pointer to message box attribute structure. Can be NULL.
 *          box: box ID number used in the binding procedure.
 */
typedef struct bind_args_ {
    msgbox_attr_t* attr;
    pmbox_t box;
} bind_args_t;

/**
 * @brief   Argument structure of a Request Kernel call.
 * @details Contains two arguments:
 *          req_msg: Pointer to message that will be sent to destination message box.
 *          ret_msg: Pointer to message to receive from the destination message box.
 */
typedef struct request_args_ {
    pmsg_t* req_msg;
    pmsg_t* ret_msg;
} request_args_t;


inline k_ret_t kcall(k_code_t code, k_arg_t arg);

pid_t pcreate(process_attr_t* attr, void (*proc_program)());
void terminate(void);
pid_t getpid(void);

priority_t nice(priority_t newPriority);

pmbox_t bind(pmbox_t box);
pmbox_t unbind(pmbox_t box);
pmbox_t getbox();

size_t send(pmbox_t dst, pmbox_t src, uint8_t* data, uint32_t size);
size_t recv(pmbox_t dst, pmbox_t src, uint8_t* data, uint32_t size);

size_t request(pmbox_t dst, pmbox_t src,
               uint8_t* req, size_t req_size, uint8_t* ret, size_t ret_max);

size_t send_user(char* str);
size_t recv_user(char* buf, uint32_t max_size);

void get_name(char* dst_str);
void set_name(char* src_str);

#endif // CALLS_H
