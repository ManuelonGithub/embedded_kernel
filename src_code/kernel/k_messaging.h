

#ifndef K_MSGBOX_H
#define K_MSGBOX_H

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

#endif // K_MSGBOX_H



