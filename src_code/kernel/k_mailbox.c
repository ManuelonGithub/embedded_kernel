
#include <stdio.h>
#include <stdlib.h>
#include "k_mailbox.h"
#include "double_link_list.h"


void k_MsgBoxBind(pmsgbox_t* box, pcb_t* owner)
{
    // Unlink box from its current place
    list_unlink((node_t*)box);

    // Set the box's owner
    box->owner = owner;

    // Link box onto the owner PCB
    if (owner->msgbox == NULL) {
        owner->msgbox = box;
    }
    else {
        list_link((node_t*)box, (node_t*)owner->msgbox);
    }
}

void k_MsgBoxUnbind(pmsgbox_t* box)
{
    // Unlink box from owner
    if (box->owner->msgbox == box) {
        box->owner->msgbox = NULL;
    }
    else {
        list_unlink((node_t*)box);
    }

    pmsg_t* msg;

    // Erase all messages inside the box
    if (box->front_msg != NULL) {
        msg = box->front_msg->next;
                                                 // Procedure unlinks message
        while (msg->next != box->front_msg) {    // In front of the "head" message in the MB
            list_unlink((node_t*)msg);           // And then erases it.
            k_pMsgDeallocate(&msg);                 // Procedure keeps going until all but the head message is left
            msg = box->front_msg->next;
        }

        k_pMsgDeallocate(&msg); // Head message is then erased here.
        box->front_msg = NULL;
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
    msg->next = NULL;
    msg->prev = NULL;
    msg->size = size;

    memcpy(msg->data, data, size);

    return msg;
}

inline void k_pMsgDeallocate(pmsg_t** msg)
{
    free((*msg)->data);
    free((*msg));
    (*msg) = NULL;
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

    list_link((node_t*)msg, (node_t*)box->front_msg);
    return msg->size;
}

