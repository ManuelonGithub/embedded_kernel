

#ifndef K_MAILBOX_H
#define K_MAILBOX_H

#include "k_types.h"

#define SYS_MAILBOXES 32

void mailbox_init(pmsgbox_t* mbox, uint32_t count);
uint32_t mailbox_bind(pmsgbox_t* box, void* owner);
uint32_t mailbox_unbind(pmsgbox_t* box);

#endif // K_MAILBOX_H
