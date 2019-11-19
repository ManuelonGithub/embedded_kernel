

#ifndef K_MAILBOX_H
#define K_MAILBOX_H

#include "k_types.h"

#define SYS_MSGBOXES 32

void k_MsgBoxBind(pmsgbox_t* box, pcb_t* owner);
void k_MsgBoxUnbind(pmsgbox_t* box);

inline ipc_msg_t* k_IPCMsgCreate(uint8_t* data, uint32_t size);
inline void k_DeleteIPCMsg(ipc_msg_t** msg);
inline uint32_t k_IPCMsgRecv(ipc_msg_t* msg, uint8_t* data, uint32_t size);
inline uint32_t k_IPCMsgSend(ipc_msg_t* msg, pmsgbox_t* box);


#endif // K_MAILBOX_H



