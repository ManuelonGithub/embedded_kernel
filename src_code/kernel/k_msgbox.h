

#ifndef K_MSGBOX_H
#define K_MSGBOX_H

#include "k_types.h"

void k_MsgBoxBind(pmsgbox_t* box, pcb_t* owner);
void k_MsgBoxUnbind(pmsgbox_t* box);

inline pmsg_t* k_pMsgAllocate(uint8_t* data, uint32_t size);
inline void k_pMsgDeallocate(pmsg_t** msg);
inline uint32_t k_pMsgRecv(pmsg_t* dst, pmsg_t* src);
inline uint32_t k_pMsgSend(pmsg_t* msg, pmsgbox_t* box);


#endif // K_MSGBOX_H



