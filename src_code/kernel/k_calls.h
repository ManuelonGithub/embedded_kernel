

#ifndef K_CALLS_H
#define K_CALLS_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "k_processes.h"

typedef enum KERNEL_CALL_CODES {
	PROC_CREATE, STARTUP, GETID, NICE, SEND, RECV, TERMINATE
} call_codes_t; /** Kernel Calls supported */

typedef struct kernel_call_arguments_ {
    call_codes_t    code;
	uint32_t        retval;
	uint32_t        argc;
	uint32_t*       argv;
} kernel_call_t ; /** Kernel Call argument structure */

inline void k_SetCall(volatile kernel_call_t* call) {
    __asm("     mov r7,r0");
}

inline kernel_call_t* k_GetCall(cpu_context_t* cpu) {
	return (kernel_call_t*)cpu->r7;
}

#endif // K_CALLS_H
