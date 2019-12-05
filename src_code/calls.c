/**
 * @file    calls.c
 * @brief   Contains all the kernel call functions that user programs have access to.
 * @author  Manuel Burnay
 * @date    2019.10.23 (Created)
 * @date    2019.11.29 (Last Modified)
 */

#include <stdlib.h>
#include <string.h>
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
 * @param   [in] attr: Pointer to process attributes to configure the process with.
 * @param   [in] proc_program: Pointer to reentrant function for the process to run.
 * @return  Process ID of the allocated process.
 *          PROC_ERR if a process failed to be allocated.
 */
pid_t pcreate(process_attr_t* attr, void (*proc_program)())
{
    pcreate_args_t args = {.attr = attr, .proc_program = proc_program};
    return (pid_t)kcall(PCREATE, (k_arg_t)&args);
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
 * @param   [in] newPriority:
 *              New priority level for the running process to run.
 *              Process can be set to a priority
 *              between HIGH_PRIORITY and LOWEST_PRIORITY.
 * @return  The value of the running's priority after the call was serviced.
 *          If the return value is different than the new priority value,
 *          then the change of priority was unsuccessful.
 * @details This is a preemptive call. When the call is done the kernel scheduler
 *          will re-evaluate which process will run based on their priorities.
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
 * @details (*) A process can set the bind ID to ANY_BOX if no particular box ID is
 *          required.
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
 * @brief   Gets the BOX ID of a box bound to the process.
 * @return  Box ID of box bound to process.
 *          BOX ERR if no boxes are bound.
 */
pmbox_t getbox()
{
    return (pmbox_t)kcall(GETBOX, NULL);
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
    pmsg_t msg = {.dst = dst, .src = src, .data = data, .size = size};

    return (size_t)kcall(SEND, (k_arg_t)&msg);
}

/**
 * @brief   Recieves a message from a process.
 * @param   [in] dst: Destination message box for the message.
 * @param   [in] src: Source message box for the message.
 * @param   [out] data: Pointer to location where message data will be sent to.
 * @param   [in] size: Maximum message size supported.
 * @param   [out] src_ret:
 *              If not NULL, the mailbox src ID that
 *              sent the message received will be copied here.
 * @return  Amount of bytes received.
 * @details This is a preemptive call. The process will block if no messages can
 *          be received at the time of the kernel call.
 */
size_t recv(pmbox_t dst, pmbox_t src, uint8_t* data, uint32_t size, pmbox_t* src_ret)
{
    pmsg_t msg = {.dst = dst, .src = src, .data = data, .size = size};

    size_t retval = kcall(RECV, (k_arg_t)&msg);

    if (src_ret != NULL)    *src_ret = msg.src;

    return retval;
}

/**
 * @brief   Performs a request transaction to a process.
 * @param   [in] dst: Message box to perform the request transaction.
 * @param   [in] src:
 *                  Source message box where the
 *                  request transaction will be engaged from.
 * @param   [in] req: Request message data to be sent.
 * @param   [in] req_size: Size of the request message data.
 * @param   [out] ret: Pointer to location where reply message data will be sent to.
 * @param   [in] ret_max: Maximum size allowed for the reply message data.
 * @param   Number of bytes received by the return message.
 * @details A request transation simply consists of
 *          a send+recv to a particular process. This call simply does it both in
 *          kernel space to improve performance, as it is a common interaction
 *          between processes.
 */
size_t request(pmbox_t dst, pmbox_t src,
               uint8_t* req, size_t req_size, uint8_t* ret, size_t ret_max)
{
    pmsg_t req_msg = {.dst = dst, .src = src, .data = req, .size = req_size};
    pmsg_t ret_msg = {.dst = src, .src = dst, .data = ret, .size = ret_max};

    request_args_t args = {.req_msg = &req_msg, .ret_msg = &ret_msg};

    return kcall(REQUEST, (k_arg_t)&args);
}

/**
 * @brief   Send a character string to IO server to be displayed to user.
 * @param   [in] box: Box ID where user data will be sent from.
 * @param   [in] str: pointer to character string.
 * @return  Number of bytes successfully displayed to the user.
 * @details This isn't a inherent kernel call, it is simply a "wrapper" function to
 *          a request kernel call where the transaction's parameter/requirements
 *          are taken care of. It is an "expensive" operation though, which has
 *          3 to 4 kernel calls in it in order to get all the parameters
 *          (valid message box & process ID).
 *          Function is useful though as it takes care of filling out the request
 *          data for a valid "output to user" interaction with the IO server.
 */
size_t send_user(pmbox_t box, char* str)
{
    // Set up metadata to send IO server
    IO_metadata_t meta = {
         .is_send = true,
         .proc_id = getpid(),
         .size = strlen(str),
         .send_data = (uint8_t*)str
    };

    return request(
            IO_BOX, box,
            (uint8_t*)&meta, sizeof(IO_metadata_t),
            NULL, strlen(str)
            );
}

/**
 * @brief   Receives a character string from the IO server (generated by the user).
 * @param   [in] box: Box ID to receive user input to.
 * @param   [in] str: pointer to character string to copy string t.
 * @return  Number of bytes successfully received from the IO server.
 * @details This isn't a inherent kernel call, it is simply a "wrapper" function to
 *          a request kernel call where the transaction's parameter/requirements
 *          are taken care of. It is an "expensive" operation though, which has
 *          3 to 4 kernel calls in it in order to get all the parameters
 *          (valid message box & process ID).
 *          Function is useful though as it takes care of filling out the request
 *          data for a valid "get user input" interaction with the IO server.
 */
size_t recv_user(pmbox_t box, char* buf, uint32_t max_size)
{
    // Set up metadata to send IO server
    IO_metadata_t meta = {
         .is_send = false,
         .proc_id = getpid(),
         .size = max_size,
         .send_data = NULL
    };

    return request(
            IO_BOX, box,
            (uint8_t*)&meta, sizeof(IO_metadata_t),
            (uint8_t*)buf, max_size
            );
}

/**
 * @brief   Gets process name.
 * @param   [out] dst_str: pointer to character buffer to place the process' name into.
 */
void get_name(char* dst_str)
{
    kcall(GET_NAME, (k_arg_t)dst_str);
}

/**
 * @brief   Sets the process name.
 * @param   [out] src_str: pointer to character string to set the process' name to.
 */
void set_name(char* src_str)
{
    kcall(SET_NAME, (k_arg_t)src_str);
}



