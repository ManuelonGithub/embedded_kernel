
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

    ipc_msg_t* msg;

    // Erase all messages inside the box
    if (box->front_msg != NULL) {
        msg = box->front_msg->next;
                                                 // Procedure unlinks message
        while (msg->next != box->front_msg) {    // In front of the "head" message in the MB
            list_unlink((node_t*)msg);           // And then erases it.
            k_DeleteIPCMsg(msg);                 // Procedure keeps going until all but the head message is left
            msg = box->front_msg->next;
        }

        k_DeleteIPCMsg(msg); // Head message is then erased here.
        box->front_msg = NULL;
    }

    // Reset the box's ownership
    box->owner = NULL;
}

inline ipc_msg_t* k_IPCMsgCreate(uint8_t* data, uint32_t size)
{
    ipc_msg_t* msg = malloc(sizeof(ipc_msg_t));
    if (msg == NULL) return NULL;

    msg->data = malloc(size);
    if (msg->data == NULL) {
        free(msg);
        return NULL;
    }

    memcpy(msg->data, data, size);

    return msg;
}

inline void k_DeleteIPCMsg(ipc_msg_t** msg)
{
    free((*msg)->data);
    free((*msg));
    (*msg) = NULL;
}

inline uint32_t k_IPCMsgRecv(ipc_msg_t* msg, uint8_t* data, uint32_t size)
{
    if (size > msg->size)   size = msg->size;   // Truncate if not enough bytes in msg
    memcpy(data, msg->data, size);
    return size;
}

inline uint32_t k_IPCMsgSend(ipc_msg_t* msg, pmsgbox_t* box)
{
    list_link((node_t*)msg, (node_t*)box->front_msg);
    return msg->size;
}

