/**
 * @file    calls.c
 * @brief   Contains all the kernel call functions that user programs have access to.
 * @author  Manuel Burnay
 * @date    2019.10.23 (Created)
 * @date    2019.11.03 (Last Modified)
 */

#include <stdlib.h>
#include "k_calls.h"
#include "calls.h"

/**
 * @brief   Requests the creation and registration of a new process in kernel space.
 */
int ProcessCreate(uint32_t pid, uint32_t priority, void (*proc_program)())
{
    k_call_t call;
    uint32_t args[] = {pid, priority, (uint32_t)(proc_program)};

    call.code = PROC_CREATE;
    call.argc = sizeof(args)/sizeof(args[0]);
    call.argv = args;

    k_SetCall(&call);

    SVC();

    return call.retval;
}

/**
 * @brief   Requests the termination of the running process.
 */
void terminate(void)
{
	k_call_t call;
	call.code = TERMINATE;

	k_SetCall(&call);

	SVC();
}

/**
 * @brief   Requests the process ID of the running process.
 * @return  The process ID value.
 */
uint32_t getpid(void)
{
    k_call_t call;
    call.code = GETID;

    k_SetCall(&call);

    SVC();

    return call.retval;
}

/**
 * @brief   Requests that the running process' running priority to be changed.
 * @param   [in] newPriority: New priority level for the running process to run.
 * @return  New priority value if priority change was successful.
 *          Any other value means the priority change was unsuccessful.
 */
uint32_t nice(uint32_t newPriority)
{
    k_call_t call;
    call.code = NICE;
    call.argc = 1;
    call.argv = &newPriority;

    k_SetCall(&call);

    SVC();

    return call.retval;
}

