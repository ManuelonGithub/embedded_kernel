/**
 * @file    calls.c
 * @brief   Contains all the kernel call functions that user programs have access to.
 * @author  Manuel Burnay
 * @date    2019.10.23 (Created)
 * @date    2019.11.02 (Last Modified)
 */

#include <stdlib.h>
#include "k_calls.h"
#include "calls.h"
#include "cpu.h"


int ProcessCreate(void (*proc_program)(), uint32_t pid, uint32_t priority)
{
    kernel_call_t call;
    uint32_t args[] = {pid, priority, (uint32_t)(proc_program)};

    call.code = PROC_CREATE;
    call.argc = sizeof(args)/sizeof(args[0]);
    call.argv = args;

    k_SetCall(&call);

    SVC();

    return call.retval;
}

void terminate(void)
{
	// call kernel handler to terminate running process.
}

//int getid()
//{
//	volatile struct kcallargs getidarg; /* Volatile to actually reserve space on stack */
//	getidarg.code = GETID;
//
//	/* Assign address if getidarg to R7 */
//	assignR7((unsigned long) &getidarg);
//
//	SVC();
//
//	return getidarg.rtnvalue;
//}
