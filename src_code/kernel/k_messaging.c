
/**
 * @file    k_messaging.c
 * @brief   Contains all message and message box allocation management and all
 *          supporting functionality regarding IPC via messages.
 * @author  Manuel Burnay
 * @date    2019.11.18 (Created)
 * @date    2019.11.21 (Last Modified)
 */

#include <stdio.h>
#include <stdlib.h>
#include "k_messaging.h"
#include "k_scheduler.h"
#include "dlist.h"
#include "k_cpu.h"
#include "bitmap.h"

pmsgbox_t msgbox[BOXID_MAX];
bitmap_t available_box[MSGBOX_BITMAP_SIZE];

/**
 * @brief   Initalizes the Messaging Module.
 * @details Initializes the free message box list.
 */
void k_MsgInit()
{
    return;
}

/**
 * @brief   Binds a message box to a process.
 * @param   [in] id: Box ID of the box to be bound to the process. (*)
 * @param   [in,out] owner: Pointer to process to have a box bound to.
 * @return  Box ID that was bound to the process.
 *          0 if it wasn't possible to bind a box to the process.
 * @details If box ID is 0, then the next available box is bound to the process.
 */
pmbox_t k_MsgBoxBind(pmbox_t id, pcb_t* owner)
{
    if (id == 0)    id = FindClear(available_box, 0, BOXID_MAX);

    if (id < BOXID_MAX && msgbox[id].owner == NULL) {
        // Set the box's owner
        msgbox[id].owner = owner;

        SetBit(available_box, id);
        SetBit(owner->owned_box, id);
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
    pmsg_t* msg = malloc(sizeof(pmsg_t));
    if (msg == NULL) return NULL;

    msg->data = malloc(size);
    if (msg->data == NULL) {
        free(msg);
        return NULL;
    }
    msg->list.next = NULL;
    msg->list.prev = NULL;
    msg->size = size;

    memcpy(msg->data, data, size);

    return msg;
}

/**
 * @brief   De-allocates a message
 */
inline void k_pMsgDeallocate(pmsg_t** msg)
{
    free((*msg)->data);
    free((*msg));
    *msg = NULL;
}

/**
 * @brief   Sends a message from one process to another.
 * @param   [in] msg: Message to be sent to a process.
 * @param   [in] proc: Process that sent the message.
 * @return  Amount of bytes successfully sent to the other process.
 * @details If a message was sent to a process that was awaiting the message,
 *          then this function places that process back into its scheduling queue
 *          and calls the scheduler trap to re-evaluate the running process.
 */
size_t k_MsgSend(pmsg_t* msg, pcb_t* proc)
{
    bool err = (msg->dst >= BOXID_MAX     || msg->src >= BOXID_MAX  ||
                msgbox[msg->dst].owner == NULL || msgbox[msg->src].owner != proc);

    size_t retval = 0;

    pmsg_t* msg_out;

    if (!err) {
        if (msgbox[msg->dst].waitany_msg != NULL) {
            msgbox[msg->dst].send_msgq->size =
                    k_pMsgTransfer(msgbox[msg->dst].send_msgq, msg);

            msgbox[msg->dst].waitany_msg->src = msg->src;

            msgbox[msg->dst].waitany_msg = NULL;

            LinkPCB(msgbox[msg->dst].owner, msgbox[msg->dst].owner->priority);

            PendSV();

            return msgbox[msg->dst].send_msgq->size;
        }

        if (msgbox[msg->src].send_msgq != NULL) {
            if (msgbox[msg->src].send_msgq->src == msg.dst) {
                // receiver msg found
                // transfer
                // unlink msg
                //
            }
        }
        else {
            // Allocate Message
            msg_out = k_pMsgAllocate(msg->data, msg->size);
            msg_out->dst = msg->dst;
            msg_out->src = msg->src;

            // Send if message allocation was successful
            if (msg_out != NULL) {
                if (msgbox[msg->dst].recv_msgq == NULL) {
                    msgbox[msg->dst].recv_msgq = msg;
                }

                dLink(&msg->list, &msgbox[msg->dst].recv_msgq->list);

                retval = msg->size;
            }
        }
    }

    return retval;
}

/**
 * @brief   Recieves a message from a process to another.
 * @param   [in,out] dst_msg:
 *              Pointer to the receiver's message slot.
 *              A message that is awaiting to be received will be copied here.
 * @param   [in,out] proc: Process that is asking for a message.
 * @return  True if a message was received,
 *          False if not.
 */
bool k_MsgRecv(pmsg_t* msg,  pcb_t* proc)
{
    // Initialized will all possible error conditions checked
    bool retval = false;

    pmsgbox_t* box = &msgbox[msg->dst];

    pmsg_t* src_msg;

    // No messages in the receive queue
    if (box->recv_msgq == NULL) {
        if (msg->src == 0) {
            box->waitany_msg = msg;
        }
        else {
            dLink(&msg->list, &msgbox[msg->src].send_msgq->list);
        }

        return false;
    }

    // Looking for any message or if front of queue is from specified source
    if (msg->src == 0 || msg->src == box->recv_msgq->src) {
        src_msg = box->recv_msgq;
        box->recv_msgq = box->recv_msgq->next;

        box->recv_msgq = box->recv_msgq->next;

        if (box->recv_msgq == src_msg) {
            box->recv_msgq = NULL;
        }
        else {
            dUnlink(&src_msg->list);
        }

        k_pMsgTransfer(msg, src_msg);
        k_pMsgDeallocate(&src_msg);

        return true;
    }

    // Looking for a specific message source
    src_msg = box->recv_msgq->next;

    while (src_msg != box->recv_msgq && !retval) {
        if (msg->src == src_msg->src) {
            // Message with specific source was found
            dUnlink(&src_msg->list);
            k_pMsgTransfer(msg, src_msg);
            k_pMsgDeallocate(&src_msg);

            return true;
        }
        else {
            src_msg = src_msg->next;
        }
    }

    // Message from specific source was not found
    dLink(&msg->list, &msgbox[msg->src].send_msgq->list);

    return false;
}

/**
 * @brief   Gets message box ID that is awaiting a message from the source box.
 * @param   [in] src_box: Box to search on-hold messages.
 * @param   [in] search_box: Box ID to look for. (*)
 * @return  search_box if the search box is awaiting a message.
 *          A valid box ID if a message box is awaiting a message and no
 *          valid search_box was submitted.
 *          An invalid box ID (255) if no message boxes are awaiting messages.
 * @details If search_box is set to 0, then the function will instead
 *          look through the on-hold bitmap for the first instance of a
 *          message box awaiting a message.
 */
pmbox_t k_GetWaitingBox(pmbox_t src_box, pmbox_t search_box)
{
    pmsg_t* msg, *msgq = msgbox[src_box].send_msgq;

    if (msgq == NULL) {
        return  BOXID_MAX;
    }

    if (search_box == 0 || msgq->src == search_box) {
        return msgq->src;
    }

    msg = msgq->next;

    while (msg != msgq) {
        if (msg->src == search_box) {
            return msg->src;
        }
        else {
            msg = msg->next;
        }
    }

    return BOXID_MAX;
}

/**
 * @brief   Transfers a message to another.
 * @param   [in,out] dst: Pointer to message that will be overwritten
 * @param   [in] src: Pointer to src message whose contents will be copied.
 * @return  Amount of bytes successfully copied from one message to another.
 */
inline uint32_t k_pMsgTransfer(pmsg_t* dst, pmsg_t* src)
{
    // Truncate if not enough space in dst
    if (dst->size > src->size)   dst->size = src->size;

    memcpy(dst->data, src->data, dst->size);
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

inline pid_t OwnerPID(pmbox_t boxID)
{
    return msgbox[boxID].owner->id;
}
