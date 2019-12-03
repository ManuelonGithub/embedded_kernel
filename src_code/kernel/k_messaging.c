
/**
 * @file    k_messaging.c
 * @brief   Contains all message and message box allocation management and all
 *          supporting functionality regarding IPC via messages.
 * @author  Manuel Burnay
 * @date    2019.11.18 (Created)
 * @date    2019.11.29 (Last Modified)
 */

#include <stdio.h>
#include <stdlib.h>
#include "k_messaging.h"
#include "k_scheduler.h"
#include "dlist.h"
#include "k_cpu.h"
#include "bitmap.h"

pmsgbox_t   msgbox[BOXID_MAX];
bitmap_t    available_box[MSGBOX_BITMAP_SIZE];

bitmap_t    available_msg[MSG_BITMAP_SIZE];
pmsg_t      msg_table[MSG_MAX];
uint8_t     msg_buffer[MSG_MAX][MSG_MAX_SIZE];

/**
 * @brief   Initalizes the Messaging Module.
 */
void k_MsgInit()
{
    ClearBitRange(available_box, 0, BOXID_MAX);

    ClearBitRange(available_msg, 0, MSG_MAX);

    int i;
    for(i = 0; i < MSG_MAX; i++) {
        msg_table[i].id = i;
        msg_table[i].data = msg_buffer[i];
    }

    return;
}

/**
 * @brief   Binds a message box to a process.
 * @param   [in] id: Box ID of the box to be bound to the process. (*)
 * @param   [in,out] owner: Pointer to process to have a box bound to.
 * @return  Box ID that was bound to the process.
 *          BOX_ERR if it wasn't possible to bind a box to the process.
 * @details If box ID is ANY_BOX, then an available box is bound to the process.
 */
pmbox_t k_MsgBoxBind(pmbox_t id, pcb_t* owner)
{
    if (id == ANY_BOX)    id = FindClear(available_box, 0, BOXID_MAX);

    if (id < BOXID_MAX && msgbox[id].owner == NULL) {
        // Set the box's owner
        msgbox[id].owner = owner;

        SetBit(available_box, id);
        SetBit(owner->owned_box, id);
    }
    else {
        id = (pmbox_t)BOX_ERR;
    }

    return id;
}

/**
 * @brief   Unbinds a message box from a process.
 * @param   [in] id: Box ID to be unbound from process.
 * @param   [in,out] proc: Process to have the box unbound from.
 * @return  0 if the unbind was successful,
 *          otherwise the box ID that was attempted to be unbound.
 */
pmbox_t k_MsgBoxUnbind(pmbox_t id, pcb_t* proc)
{
    pmsgbox_t* box = &msgbox[id];

    if (id < BOXID_MAX && box->owner == proc) {
        k_MsgClearAll(box);

        // Clear all pending messages

        // Reset the box's ownership
        box->owner = NULL;

        ClearBit(available_box, id);
        ClearBit(proc->owned_box, id);

        // Reset the return box ID
        id = 0;
    }

    return id;
}

/**
 * @brief   Unbinds all message boxes bound to a process.
 * @param   [in,out] proc: pointer to process PCB to unbind all boxes from.
 */
void k_MsgBoxUnbindAll(pcb_t* proc)
{
    uint32_t min = FindSet(proc->owned_box, 0, BOXID_MAX);

    while (min != BOXID_MAX) {
        k_MsgBoxUnbind(min, proc);
        min = FindSet(proc->owned_box, min, BOXID_MAX);
    }
}

/**
 * @brief   Allocates message and fills its data and size.
 * @param   [in] data: Pointer to the message data to be copied to the message.
 * @param   [in] size: Size of the message data.
 * @return  Allocated message if the allocation was successful,
 *          NULL if it was unsuccessful.
 */
inline pmsg_t* k_pMsgAllocate(uint8_t* data, uint32_t size)
{
    pmsg_t* msg = NULL;

    uint32_t i = FindClear(available_msg, 0, MSG_MAX);

    if (i >= MSG_MAX)   return NULL;

    msg = &msg_table[i];
    SetBit(available_msg, i);
    if (size > MSG_MAX_SIZE)    msg->size = MSG_MAX_SIZE;
    else                        msg->size = size;

    msg->list.next = NULL;
    msg->list.prev = NULL;

    if (data != NULL) {
        memcpy(msg->data, data, size);
    }
    else {
        msg->size = 0;
    }

    return msg;
}

/**
 * @brief   De-allocates a message
 * @param   [in,out] msg: pointer to message pointer to be deallocated.
 */
inline void k_pMsgDeallocate(pmsg_t** msg)
{
    ClearBit(available_msg, (*msg)->id);
    *msg = NULL;
}

/**
 * @brief   Sends a message from one process to another.
 * @param   [in] msg: Message to be sent to a process.
 * @param   [out] retsize: Amount of bytes successfully sent to message box.
 * @details If a message was sent to a process that was awaiting the message,
 *          then this function places that process back into its scheduling queue
 *          and calls the scheduler trap to re-evaluate the running process.
 */
void k_MsgSend(pmsg_t* msg, size_t* retsize)
{
    size_t size = 0;

    pmsg_t* msg_out;

    pmsgbox_t* dst_box = &msgbox[msg->dst];

    bool wait_msg_good =
            dst_box->wait_msg != NULL && (dst_box->wait_msg->src == ANY_BOX ||
                    dst_box->wait_msg->src == msg->src);

    if (wait_msg_good) {
        size = k_pMsgTransfer(dst_box->wait_msg, msg);

        // Remove link from the Receiver's box
        if (dst_box->retsize != NULL)   *dst_box->retsize = size;
        dst_box->wait_msg = NULL;

        LinkPCB(dst_box->owner, dst_box->owner->priority);

        PendSV();
    }
    else {
        // Allocate Message
        msg_out = k_pMsgAllocate(msg->data, msg->size);
        msg_out->dst = msg->dst;
        msg_out->src = msg->src;

        // Send if message allocation was successful
        if (msg_out != NULL) {
            if (msgbox[msg->dst].recv_msgq == NULL) {
                msgbox[msg->dst].recv_msgq = msg_out;
            }

            dLink(&msg_out->list, &msgbox[msg->dst].recv_msgq->list);

            size = msg_out->size;
        }
    }

    if (retsize != NULL)    *retsize = size;
}

/**
 * @brief   Recieves a message from a process to another.
 * @param   [in,out] dst_msg:
 *              Pointer to the receiver's message slot.
 *              A message that is awaiting to be received will be copied here.
 * @param   [out] retsize: Number of bytes successfully received.
 * @details If there isn't any messages to receive,
 *          the dst_msg and retsize's addresses are copied onto the receiver's
 *          message box and the process that owns the message box is then blocked
 *          while it awaits for another process to send it a message.
 */
void k_MsgRecv(pmsg_t* msg, size_t* retsize)
{
    pmsgbox_t* dst_box = NULL;

    pmsg_t* src_msg = NULL;

    (*retsize) = 0;

    // If no errors were found
    dst_box = &msgbox[msg->dst];

    if (dst_box->recv_msgq == NULL) {
        // No messages to receive at the time.
        // Link Rx message onto message box
        dst_box->wait_msg = msg;
        dst_box->retsize = retsize;
        UnlinkPCB(dst_box->owner);
        dst_box->owner->state = BLOCKED;
        PendSV();
    }
    else if (msg->src == ANY_BOX || dst_box->recv_msgq->src == msg->src){
        src_msg = dst_box->recv_msgq;

        dst_box->recv_msgq = dst_box->recv_msgq->next;

        if (dst_box->recv_msgq == src_msg) {
            dst_box->recv_msgq = NULL;
        }

        dUnlink(&src_msg->list);

        (*retsize) = k_pMsgTransfer(msg, src_msg);
        k_pMsgDeallocate(&src_msg);
    }
    else {
        // Search Recv queue for specific message box source
        src_msg = k_SearchMessageList(dst_box->recv_msgq, msg->src);

        if (src_msg == NULL) {
            // If message was not found
            // Link Rx message onto message box
            dst_box->wait_msg = msg;
            dst_box->retsize = retsize;
            UnlinkPCB(dst_box->owner);
            dst_box->owner->state = BLOCKED;
            PendSV();
        }
        else {
            // Message was found
            // Unlink it from Recv queue
            dUnlink(&src_msg->list);

            // Transfer message
            (*retsize) = k_pMsgTransfer(msg, src_msg);
            k_pMsgDeallocate(&src_msg);
        }
    }
}

/**
 * @brief   Transfers a message to another.
 * @param   [in,out] dst: Pointer to message that will be overwritten
 * @param   [in] src: Pointer to src message whose contents will be copied.
 * @return  Amount of bytes successfully transfered from one message to another.
 * @details If the destination doesn't have a valid data pointer,
 *          data won't be transfered, but the size of the "would-be" transfer is
 *          still recorded.
 *          This allows for messages that just want the size of the message
 *          to be possible.
 */
inline uint32_t k_pMsgTransfer(pmsg_t* dst, pmsg_t* src)
{
    // Truncate if not enough space in dst
    if (dst->size > src->size)   dst->size = src->size;
    if (dst->data != NULL && src->data != NULL) {
        memcpy(dst->data, src->data, dst->size);
    }
    dst->src = src->src;

    return dst->size;
}

/**
 * @brief   Clears all Messages currently in the message box.
 * @param   [in,out] box: Message box to clear messages from.
 */
void k_MsgClearAll(pmsgbox_t* box)
{
    pmsg_t* msg;

    while (box->recv_msgq != NULL) {
        msg = box->recv_msgq;
        box->recv_msgq = box->recv_msgq->next;

        if (msg == box->recv_msgq)  box->recv_msgq = NULL;

        k_pMsgDeallocate(&msg);
    }
}

/**
 * @brief   Searches through message list for a message with a particular
 *          message box as its source.
 * @param   [in] msg: pointer to message list entry point.
 * @param   [in] box: box ID to search message's source for.
 * @return  pointer to message whose source has a matching box ID.
 *          NULL if no messages in message list has a source matching the box ID.
 */
pmsg_t* k_SearchMessageList(pmsg_t* msg, pmbox_t box)
{
    pmsg_t* search;

    if (msg == NULL)    return NULL;
    if (msg->src == box) return msg;

    search = msg->next;

    while (search != msg) {
        if (search->src == box) return search;
        else search = search->next;
    }

    return NULL;
}
