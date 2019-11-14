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
#include "k_processes.h"
#include "k_cpu.h"

typedef enum KERNEL_CALL_CODES {
	PROC_CREATE, STARTUP, GETID, NICE, BIND, UNBIND, SEND, RECV, TERMINATE
} call_codes_t; /** Kernel Calls supported */

typedef struct kernel_call_arguments_ {
    call_codes_t    code;
	uint32_t        retval;
	uint32_t        argc;
	uint32_t*       argv;
} k_call_t ; /** Kernel Call argument structure */

/** @brief  Sets up the Kernel call parameters for the trap handler to retrieve. */
inline void k_SetCall(volatile k_call_t* call) {
	SetCallReg((void*)call);
}

/** @brief  Retrieves the Kernel call parameters for the trap handler. */
inline k_call_t* k_GetCall() {
	return (k_call_t*)GetCallReg();
}

#endif // K_CALLS_H
