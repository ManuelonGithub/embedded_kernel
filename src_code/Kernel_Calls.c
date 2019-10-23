/**
 * @file    Kernel_Calls.c
 * @brief   Contains all the kernel functions that user programs have access to.
 * @author  Manuel Burnay
 * @date    2019.10.23 (Created)
 * @date    2019.10.23 (Last Modified)
 */


#include "Kernel.h"
#include "Kernel_Processes.h"
#include "Kernel_Scheduler.h"


int reg_proc(void (*proc_func)(), uint32_t pid, uint32_t priority)
{
    // Check if another process has this pid
    // if (FindPID(pid))     return INVALID_PID;
    pcb_handle_code_t retval;

	pcb_t* newPCB = malloc(sizeof(pcb_t));
	newPCB->sp = malloc(STACKSIZE);
	newPCB->id = pid;
	newPCB->priority = priority;

	// Initialize CPU context by pushing register values.

	retval = LinkPCB(newPCB);

	if (retval < 0) {   // Something went wrong with pcb linking
	    free(newPCB->sp);
	    free(newPCB);
	}

	return retval;
}

void terminate(void)
{
	// call kernell handler to terminate running process.
}

int getid()
{
	volatile struct kcallargs getidarg; /* Volatile to actually reserve space on stack */
	getidarg.code = GETID;

	/* Assign address if getidarg to R7 */
	assignR7((unsigned long) &getidarg);

	SVC();

	return getidarg.rtnvalue;
}
