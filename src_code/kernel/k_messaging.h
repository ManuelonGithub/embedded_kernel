
/**
 * @file    k_messaging.h
 * @brief   Contains all definitions and function prototypes regarding
 *          inter-process communications via messaging.
 * @author  Manuel Burnay
 * @date    2019.11.18 (Created)
 * @date    2019.11.21 (Last Modified)
 */

#ifndef K_MESSAGING_H
#define K_MESSAGING_H

#include "k_types.h"

void k_MsgInit();

pmbox_t k_MsgBoxBind(pmbox_t id, pcb_t* proc);
pmbox_t k_MsgBoxUnbind(pmbox_t id, pcb_t* proc);

void k_MsgBoxUnbindAll(pcb_t* proc);

inline pmsg_t* k_pMsgAllocate(uint8_t* data, uint32_t size);
inline void k_pMsgDeallocate(pmsg_t** msg);

size_t k_MsgSend(pmsg_t* msg, pcb_t* proc);
bool k_MsgRecv(pmsg_t* dst_msg,  pcb_t* proc);

inline uint32_t k_pMsgTransfer(pmsg_t* dst, pmsg_t* src);

void k_MsgClearAll(pmbox_t id);

inline pid_t OwnerID(pmbox_t boxID);

#endif // K_MESSAGING_H

