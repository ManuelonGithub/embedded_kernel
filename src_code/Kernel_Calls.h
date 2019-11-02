/**
 * @file    Kernel.h
 * @brief   Defines all functions and entities that user programs have access to.
 * @details This is the module that opens the kernel to the user programs.
 *          This includes kernel calls and process creation.
 * @author  Manuel Burnay
 * @date    2019.10.22  (Created)
 * @date    2019.10.23  (Last Modified)
 */

#ifndef     KERNEL_H
#define     KERNEL_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum KERNEL_CALL_CODES {
	PROC_CREATE, GETID, NICE, SEND, RECV, TERMINATE
} call_codes_t; /** Kernel Calls supported */

typedef struct kernel_call_arguments_ {
	uint32_t    call_code;
	uint32_t    retval;
	uint32_t    argc;
	uint32_t*   argv;
} kernel_call_t ; /** Kernel Call argument structure */



int process_create(void (*proc_program)(), uint32_t pid, uint32_t priority);
void terminate(void);


#endif	//  KERNEL_H
