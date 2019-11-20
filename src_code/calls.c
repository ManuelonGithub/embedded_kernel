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
proc_t pcreate(pid_t pid, priority_t priority, void (*proc_program)())
{
    k_call_t call;
    uint32_t args[] = {(uint32_t)pid, (uint32_t)priority, (uint32_t)(proc_program)};

    call.code = PROC_CREATE;
    call.arg = args;

    k_SetCall(&call);

    SVC();

    return (proc_t)call.retval;
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
pid_t getpid(void)
{
    k_call_t call;
    call.code = GETID;

    k_SetCall(&call);

    SVC();

    return (pid_t)call.retval;
}

/**
 * @brief   Requests that the running process' running priority to be changed.
 * @param   [in] newPriority: New priority level for the running process to run.
 * @return  New priority value if priority change was successful.
 *          Any other value means the priority change was unsuccessful.
 */
priority_t nice(priority_t newPriority)
{
    k_call_t call;
    call.code = NICE;
    call.arg = &newPriority;

    k_SetCall(&call);

    SVC();

    return (priority_t)call.retval;
}

pmbox_t bind(pmbox_t box)
{
    k_call_t call;

    call.code = BIND;
    call.arg = &box;

    k_SetCall(&call);

    SVC();

    return (pmbox_t)call.retval;
}

pmbox_t unbind(pmbox_t box)
{
    k_call_t call;

    call.code = UNBIND;
    call.arg = &box;

    k_SetCall(&call);

    SVC();

    return (pmbox_t)call.retval;
}

size_t send(pmbox_t dst, pmbox_t src, uint8_t* data, uint32_t size)
{
    pmsg_t msg = {.dst = dst, .src = src, .data = data, .size = size};

    k_call_t call;

    call.code = SEND;
    call.arg = (k_arg_t)&msg;

    k_SetCall(&call);

    SVC();

    return (pmbox_t)call.retval;
}

size_t recv(pmbox_t dst, pmbox_t src, uint8_t* data, uint32_t size)
{
    pmsg_t msg = {.dst = dst, .src = src, .data = data, .size = size};

    k_call_t call;

    call.code = RECV;
    call.arg = (k_arg_t)&msg;

    k_SetCall(&call);

    SVC();

    // retval for this call is irrelevant b/c msg may not be receive at "call" time.
    return (pmbox_t)msg.size;
}
