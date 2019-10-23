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
	GETID, NICE, SEND, RECV, TERMINATE
} call_codes_t; /** Kernel Calls supported */

typedef struct kernel_call_arguments_ {
	unsigned int code;
	unsigned int rtnvalue;
	unsigned int arg1;
	unsigned int arg2;
} call_args_t; /** Kernel Call argument structure */ 

int reg_proc(void (*proc_func)(), uint32_t pid, uint32_t priority);
void terminate(void);


#endif	//  KERNEL_H
