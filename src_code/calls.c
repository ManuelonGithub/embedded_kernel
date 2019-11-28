/**
 * @file    calls.c
 * @brief   Contains all the kernel call functions that user programs have access to.
 * @author  Manuel Burnay
 * @date    2019.10.23 (Created)
 * @date    2019.11.03 (Last Modified)
 */

#include <stdlib.h>
#include "k_cpu.h"
#include "calls.h"

/**
 * @brief   Sets up the kernel call and gives CPU control to the kernel to service
 *          the call.
 * @param   [in] code: Code associated with the kernel call to be serviced.
 * @param   [in,out] arg: Kernel call argument relevant to the call done.
 * @return  A 32-bit value associated with the call done.
 */
inline k_ret_t kcall(k_code_t code, k_arg_t arg)
{
    k_call_t call = {.code = code, .arg = arg};
    SetCallReg(&call);
    SVC();
    return call.retval;
}

/**
 * @brief   Requests the creation and registration of a new process in kernel space.
 */
pid_t pcreate(process_attr_t* attr, void (*proc_program)())
{
    uint32_t args[] = {(uint32_t)attr, (uint32_t)(proc_program)};
    return (pid_t)kcall(PCREATE, (k_arg_t)args);
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
    return (pid_t)kcall(GETPID, NULL);
}

/**
 * @brief   Requests that the running process' priority to be changed.
 * @param   [in] newPriority: New priority level for the running process to run.
 * @return  The value of the running's priority after the call was serviced.
 *          If the return value is different than the new priority value,
 *          then the change of priority was unsuccessful.
 */
priority_t nice(priority_t newPriority)
{
    return (priority_t)kcall(NICE, (k_arg_t)&newPriority);
}

/**
 * @brief   Binds a message box to the running process.
 * @param   [in] Box ID to bind with (*).
 * @return  The box ID that was bound to the process.
 *          If the returned value is 0, a bind wasn't able to be made.
 * @details (*) A process can set the bind ID to 0 if no particular box ID is
 *          desired.
 */
pmbox_t bind(pmbox_t box)
{
    return (pmbox_t)kcall(BIND, (k_arg_t)&box);
}

/**
 * @brief   Unbinds a message box from the running process.
 * @param   [in] Box ID to unbind from.
 * @return  0 if the unbind process was successful,
 *          else it'll return the box ID attempted to be unbound.
 */
pmbox_t unbind(pmbox_t box)
{
    return (pmbox_t)kcall(UNBIND, (k_arg_t)&box);
}

/**
 * @brief   Send a message to a process.
 * @param   [in] dst: Destination message box for the message.
 * @param   [in] src: Source message box for the message.
 * @param   [in] data: Message data to be sent.
 * @param   [in] size: Size of the message data.
 * @return  Amount of bytes able to send to destination.
 *          This does not guarantee that all bytes will be received however,
 *          as message can be placed on hold until the receiver asks for a message,
 *          and cannot take all the contents of the message.
 */
size_t send(pmbox_t dst, pmbox_t src, uint8_t* data, uint32_t size)
{
    pmsg_t msg = {.dst = dst, .src = src, .blocking = false};

#ifdef REAL_TIME_MODE
    if (size < MSG_MAX_SIZE)    msg.size = size;
    else                        msg.size = MSG_MAX_SIZE;

    memcpy(msg.data, data, size);
#else
    msg.data = data;
    msg.size = size;
#endif

    return (size_t)kcall(SEND, (k_arg_t)&msg);
}

size_t send_sync(pmbox_t dst, pmbox_t src, uint8_t* data, uint32_t size)
{
    pmsg_t msg = {.dst = dst, .src = src, .blocking = true};

#ifdef REAL_TIME_MODE
    if (size < MSG_MAX_SIZE)    msg.size = size;
    else                        msg.size = MSG_MAX_SIZE;

    memcpy(msg.data, data, size);
#else
    msg.data = data;
    msg.size = size;
#endif

    return (size_t)kcall(SEND, (k_arg_t)&msg);
}

/**
 * @brief   Recieves a message from a process.
 * @param   [in] dst: Destination message box for the message.
 * @param   [in] src: Source message box for the message. (WIP) (*)
 * @param   [in] data: Pointer to location where message data will be sent to.
 * @param   [in] size: Maximum message size supported.
 * @return  Amount of bytes recieved.
 */
size_t recv(pmbox_t dst, pmbox_t src, uint8_t* data, uint32_t size)
{
    pmsg_t msg = {.dst = dst, .src = src, .size = size, .blocking = true};

#ifndef REAL_TIME_MODE
    msg.data = data;
#endif

    kcall(RECV, (k_arg_t)&msg);

#ifdef  REAL_TIME_MODE
    memcpy(data, msg.data, size);
#endif

    // retval for this call is irrelevant b/c msg may not be receive at "call" time.
    return (size_t)msg.size;
}

size_t recv_async(pmbox_t dst, pmbox_t src, uint8_t* data, uint32_t size)
{
    pmsg_t msg = {.dst = dst, .src = src, .size = size, .blocking = false};

#ifndef REAL_TIME_MODE
    msg.data = data;
#endif

    kcall(RECV, (k_arg_t)&msg);

#ifdef  REAL_TIME_MODE
    memcpy(data, msg.data, size);
#endif

    // retval for this call is irrelevant b/c msg may not be receive at "call" time.
    return (size_t)msg.size;
}

size_t send_user(char* str)
{
    return (size_t)kcall(SEND_USER, (k_arg_t)str);
}

size_t recv_user(char* buf, uint32_t max_size)
{
    uint32_t args[] = {(uint32_t)buf, max_size};

    return (size_t)kcall(RECV_USER, (k_arg_t)args);
}



