
#include <stdio.h>
#include <stdlib.h>
#include "k_msgbox.h"
#include "dlist.h"


void k_MsgBoxBind(pmsgbox_t* box, pcb_t* owner)
{
    // Unlink box from its current place
    dUnlink((node_t*)box);

    // Set the box's owner
    box->owner = owner;

    // Link box onto the owner PCB
    if (owner->msgbox == NULL)  owner->msgbox = box;
    dLink(&box->list, &owner->msgbox->list);
}

void k_MsgBoxUnbind(pmsgbox_t* box)
{
    // Unlink box from owner
    if (box->owner->msgbox == box) {
        box->owner->msgbox = NULL;
    }
    else {
        dUnlink(&box->list);
    }

    pmsg_t* msg;

    while (box->front_msg != NULL) {
        msg = box->front_msg;
        box->front_msg = box->front_msg->next;

        if (msg == box->front_msg)  box->front_msg = NULL;

        k_pMsgDeallocate(&msg);
    }

    // Reset the box's ownership
    box->owner = NULL;
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

inline uint32_t k_pMsgRecv(pmsg_t* dst, pmsg_t* src)
{
    if (dst->size > src->size)   dst->size = src->size;   // Truncate if not enough bytes in msg
    memcpy(dst->data, src->data, dst->size);
    return dst->size;
}

inline uint32_t k_pMsgSend(pmsg_t* msg, pmsgbox_t* box)
{
    if (box->front_msg == NULL)     box->front_msg = msg;

    dLink(&msg->list, &box->front_msg->list);
    return msg->size;
}

