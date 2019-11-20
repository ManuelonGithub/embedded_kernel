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


inline k_ret_t kcall(k_code_t code, k_arg_t arg)
{
    k_call_t call = {.code = code, .arg = arg};
    k_SetCall(&call);
    SVC();
    return (proc_t)call.retval;
}

/**
 * @brief   Requests the creation and registration of a new process in kernel space.
 */
proc_t pcreate(pid_t pid, priority_t priority, void (*proc_program)())
{
    uint32_t args[] = {(uint32_t)pid, (uint32_t)priority, (uint32_t)(proc_program)};
    return (proc_t)kcall(PCREATE, (k_arg_t)args);
}

/**
 * @brief   Requests the termination of the running process.
 */
void terminate(void)
{
	kcall(TERMINATE, NULL);
}

/**
 * @brief   Requests the process ID of the running process.
 * @return  The process ID value.
 */
pid_t getpid(void)
{
    return (pid_t)kcall(GETID, NULL);
}

/**
 * @brief   Requests that the running process' running priority to be changed.
 * @param   [in] newPriority: New priority level for the running process to run.
 * @return  New priority value if priority change was successful.
 *          Any other value means the priority change was unsuccessful.
 */
priority_t nice(priority_t newPriority)
{
    return (priority_t)kcall(NICE, (k_arg_t)&newPriority);
}

pmbox_t bind(pmbox_t box)
{
    return (pmbox_t)kcall(BIND, (k_arg_t)&box);
}

pmbox_t unbind(pmbox_t box)
{
    return (pmbox_t)kcall(UNBIND, (k_arg_t)&box);
}

size_t send(pmbox_t dst, pmbox_t src, uint8_t* data, uint32_t size)
{
    pmsg_t msg = {.dst = dst, .src = src, .data = data, .size = size};

    return (size_t)kcall(SEND, (k_arg_t)&msg);
}

size_t recv(pmbox_t dst, pmbox_t src, uint8_t* data, uint32_t size)
{
    pmsg_t msg = {.dst = dst, .src = src, .data = data, .size = size};

    kcall(SEND, (k_arg_t)&msg);

    // retval for this call is irrelevant b/c msg may not be receive at "call" time.
    return (size_t)msg.size;
}
