
#include <stdio.h>
#include <stdlib.h>
#include "k_messaging.h"
#include "k_scheduler.h"
#include "dlist.h"
#include "k_cpu.h"

pmsgbox_t* free_box;
pmsgbox_t mbox[SYS_MSGBOXES];


void k_MsgInit()
{
    int i;

    free_box = &mbox[0];
    mbox[0].ID = 0;

    for (i = 1; i < SYS_MSGBOXES; i++) {
        mbox[i].ID = i;
        mbox[i-1].next = &mbox[i];
        mbox[i].prev = &mbox[i-1];
    }
}

pmbox_t k_MsgBoxBind(pmbox_t id, pcb_t* proc)
{
    pmsgbox_t* box;

    if (id == 0 && free_box != NULL ) {
        box = free_box;
        free_box = free_box->next;
    }
    else if (id < SYS_MSGBOXES && mbox[id].owner == NULL) {
        box = &mbox[id];

        // Move free list pointer if it pointed to msgbox
        if (&mbox[id] == free_box)    free_box = free_box->next;
    }
    else    return 0;   // Wasn't able to bind mailbox, return 0

    // Unlink box from its current place
    dUnlink((node_t*)box);

    // Set the box's owner
    box->owner = proc;

    // Link box onto the owner PCB
    if (proc->msgbox == NULL)  proc->msgbox = box;
    dLink(&box->list, &proc->msgbox->list);

    return box->ID;
}

pmbox_t k_MsgBoxUnbind(pmbox_t id, pcb_t* proc)
{
    pmsgbox_t* box = &mbox[id];

    if (id < SYS_MSGBOXES && box->owner == proc) {
        // Unlink box from owner
        if (proc->msgbox == box) {
            proc->msgbox = NULL;
        }
        else {
            dUnlink(&box->list);
        }

        pmsg_t* msg;

        // Clear all pending messages
        while (box->front_msg != NULL) {
            msg = box->front_msg;
            box->front_msg = box->front_msg->next;

            if (msg == box->front_msg)  box->front_msg = NULL;

            k_pMsgDeallocate(&msg);
        }

        // Reset the box's ownership
        box->owner = NULL;

        // Link box back to the free list
        if (free_box == NULL) {
            mbox[id].next = NULL;
            mbox[id].prev = NULL;
            free_box = &mbox[id];
        }
        else {
            dLink(&mbox[id].list, &free_box->list);
        }

        // the free_box list is a FIFO,
        // so the "newest" available box is at the front of the list
        free_box = &mbox[id];

        // Reset the return box ID
        id = 0;
    }

    return id;
}

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

inline void k_pMsgDeallocate(pmsg_t** msg)
{
    free((*msg)->data);
    free((*msg));
    *msg = NULL;
}

size_t k_MsgSend(pmsg_t* msg, pcb_t* proc)
{
    bool err = (msg->dst >= SYS_MSGBOXES     || msg->src >= SYS_MSGBOXES  ||
                mbox[msg->dst].owner == NULL || mbox[msg->src].owner != proc);

    size_t retval = 0;

    if (!err) {
        if (mbox[msg->dst].wait_msg != NULL) {
            // todo: check if wait msg's src is the dst's mailbox
            retval = k_pMsgTransfer(mbox[msg->dst].wait_msg, msg);
            LinkPCB(mbox[msg->dst].owner, mbox[msg->dst].owner->priority);
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

bool k_MsgRecv(pmsg_t* dst_msg,  pcb_t* proc)
{
    // Initialized will all possible error conditions checked
    bool retval = (dst_msg == NULL || dst_msg->dst >= SYS_MSGBOXES ||
                   mbox[dst_msg->dst].owner != proc);

    // If no errors were found
    if (!retval) {
        if (mbox[dst_msg->dst].front_msg == NULL) {
            mbox[dst_msg->dst].wait_msg = dst_msg;
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


inline uint32_t k_pMsgTransfer(pmsg_t* dst, pmsg_t* src)
{
    // Truncate if not enough space in dst
    if (dst->size > src->size)   dst->size = src->size;

    memcpy(dst->data, src->data, dst->size);
    return dst->size;
}

