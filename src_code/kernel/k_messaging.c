
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

pmsgbox_t mbox[SYS_MSGBOXES];
uint8_t box_bitmap[SYS_MSGBOXES/8];

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
    if (id == 0)    id = FindClear(box_bitmap, 0, SYS_MSGBOXES);

    if (id < SYS_MSGBOXES && mbox[id].owner == NULL) {
        // Set the box's owner
        mbox[id].owner = owner;

        SetBit(box_bitmap, id);
        SetBit(owner->box_bitmap, id);
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
    pmsgbox_t* box = &mbox[id];

    if (id < SYS_MSGBOXES && box->owner == proc) {
        k_MsgClearAll(box);

        // Clear all pending messages

        // Reset the box's ownership
        box->owner = NULL;

        ClearBit(box_bitmap, id);
        ClearBit(proc->box_bitmap, id);

        // Reset the return box ID
        id = 0;
    }

    return id;
}

void k_MsgBoxUnbindAll(pcb_t* proc)
{
    uint32_t min = FindSet(proc->box_bitmap, 0, SYS_MSGBOXES);

    while (min != SYS_MSGBOXES) {
        k_MsgBoxUnbind(min, proc);
        min = FindSet(proc->box_bitmap, min, SYS_MSGBOXES);
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
    bool err = (msg->dst >= SYS_MSGBOXES     || msg->src >= SYS_MSGBOXES  ||
                mbox[msg->dst].owner == NULL || mbox[msg->src].owner != proc);

    size_t retval = 0;

    if (!err) {
        if (mbox[msg->dst].wait_msg != NULL) {
            retval = k_pMsgTransfer(mbox[msg->dst].wait_msg, msg);

            mbox[msg->dst].wait_msg->src = msg->src;
            mbox[msg->dst].wait_msg->size = retval;

            LinkPCB(mbox[msg->dst].owner, mbox[msg->dst].owner->priority);

            ClearBit(mbox[msg->src].OnHold_bitmap, msg->dst);

            PendSV();
        }
        else {
            // Allocate Message
            pmsg_t* msg_out = k_pMsgAllocate(msg->data, msg->size);
            msg_out->dst = msg->dst;
            msg_out->src = msg->src;

            // Send if message allocation was successful
            if (msg_out != NULL) {
                if (mbox[msg->dst].front_msg == NULL) {
                    mbox[msg->dst].front_msg = msg;
                }

                dLink(&msg->list, &mbox[msg->dst].front_msg->list);

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
bool k_MsgRecv(pmsg_t* dst_msg,  pcb_t* proc)
{
    // Initialized will all possible error conditions checked
    bool retval = (dst_msg == NULL || dst_msg->dst >= SYS_MSGBOXES ||
                   mbox[dst_msg->dst].owner != proc ||
                   mbox[dst_msg->src].owner == NULL);

    // If no errors were found
    if (!retval) {
        if (mbox[dst_msg->dst].front_msg == NULL) {
            mbox[dst_msg->dst].wait_msg = dst_msg;

            SetBit(mbox[dst_msg->src].OnHold_bitmap, dst_msg->dst);

            retval = false;
        }
        else {
            // todo: create a "specified source" check system

            pmsg_t* src_msg = mbox[dst_msg->dst].front_msg;

            mbox[dst_msg->dst].front_msg = mbox[dst_msg->dst].front_msg->next;

            if (mbox[dst_msg->dst].front_msg == src_msg) {
                mbox[dst_msg->dst].front_msg = NULL;
            }
            else {
                dUnlink(&src_msg->list);
            }

            k_pMsgTransfer(dst_msg, src_msg);
            k_pMsgDeallocate(&src_msg);

            retval = true;
        }
    }

    return retval;
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
pmbox_t k_GetOnHoldMsg(pmbox_t src_box, pmbox_t search_box)
{
    if (search_box != 0 && search_box < SYS_MSGBOXES) {
        if (!GetBit(mbox[src_box].OnHold_bitmap, search_box)) {
            search_box = SYS_MSGBOXES;
        }
    }
    else {
        search_box = FindSet(mbox[src_box].OnHold_bitmap, 0, SYS_MSGBOXES);
    }

    return search_box;
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

    while (box->front_msg != NULL) {
        msg = box->front_msg;
        box->front_msg = box->front_msg->next;

        if (msg == box->front_msg)  box->front_msg = NULL;

        k_pMsgDeallocate(&msg);
    }
}

inline pid_t OwnerID(pmbox_t boxID)
{
    return mbox[boxID].owner->id;
}
