/**
 * @file    k_calls.h
 * @brief   Defines all priviledged functions and entities
 *          regarding kernel calls
 * @author  Manuel Burnay
 * @date    2019.11.02  (Created)
 * @date    2019.11.03  (Last Modified)
 */

#ifndef K_CALLS_H
#define K_CALLS_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "k_types.h"
#include "k_cpu.h"

typedef struct kernel_call_arguments_ {
    k_code_t    code;
    k_ret_t     retval;
	k_arg_t     arg;
} k_call_t ; /** Kernel Call argument structure */

typedef struct pcreate_args_ {
    pid_t       id;
    priority_t  prio;
    void        (*proc_program)();
} pcreate_args_t;

/** @brief  Sets up the Kernel call parameters for the trap handler to retrieve. */
inline void k_SetCall(volatile k_call_t* call) {
	SetCallReg((void*)call);
}

/** @brief  Retrieves the Kernel call parameters for the trap handler. */
inline k_call_t* k_GetCall() {
	return (k_call_t*)GetCallReg();
}

#endif // K_CALLS_H
