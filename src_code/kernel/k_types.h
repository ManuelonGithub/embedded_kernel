

#ifndef K_TYPES_H
#define K_TYPES_H

#include <stdint.h>
#include <stdbool.h>

/** @brief  Internal Inter-process message structure */
typedef struct ipc_message_ {
    struct ipc_message_* next;
    struct ipc_message_* prev;
    uint32_t             size;
    uint8_t*             data;
} ipc_msg_t;

typedef uint32_t    id_t; /// System ID type alias

/** @brief  Process Mailbox structure */
typedef struct pmsgbox_descriptor_ {
    struct pmsgbox_descriptor_*     next;
    struct pmsgbox_descriptor_*     prev;
    void*       owner;
    ipc_msg_t*  front_msg;
    bool        waiting;
    id_t        ID;
} pmsgbox_t;

typedef id_t        pid_t;      /// Process ID type alias
typedef uint32_t    priority_t; /// Process priority type alias

/** @brief  Process control block structure */
typedef struct pcb_ {
    struct pcb_*    next;
    struct pcb_*    prev;
    pid_t            id;
    priority_t      priority;
    uint32_t*       sp_top;
    uint32_t*       sp;
    pmsgbox_t*      msgbox;
    uint32_t        timer;
} pcb_t;

typedef pid_t       process_t;  /// Process type alias

/** @brief  Process Initialization attributes */
typedef struct p_attributes_ {
    pid_t       id;
    priority_t  priority;
} p_attributes_t;

#define PID_MAX 256

typedef struct pid_bitmap_ {
    uint8_t byte[((PID_MAX)/8)];
} pid_bitmap_t;

typedef struct kernel_data_ {
    pcb_t* running;
    pmsgbox_t* free_mbox;
    pmsgbox_t mbox[SYS_MAILBOXES];
    pid_bitmap_t pid_bitmap;
} kernel_t;

#endif
