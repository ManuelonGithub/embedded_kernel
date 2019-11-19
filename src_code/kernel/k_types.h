

#ifndef K_TYPES_H
#define K_TYPES_H

#include <stdint.h>
#include <stdbool.h>

/** @brief  Internal Inter-process message structure */
typedef struct ipc_msg_ {
    struct ipc_msg_*    next;
    struct ipc_msg_*    prev;
    uint32_t            size;
    uint8_t*            data;
} ipc_msg_t;

typedef uint32_t    id_t; /// System ID type alias

/** @brief  Inter-process communication Message box structure */
typedef struct pmsgbox_ {
    struct pmsgbox_*    next;
    struct pmsgbox_*    prev;
    struct pcb_*        owner;
    ipc_msg_t*          front_msg;
    bool                waiting;
    id_t                ID;
} pmsgbox_t;

typedef id_t    pmbox_t; /// Message Box type alias

typedef id_t        pid_t;      /// Process ID type alias
typedef uint32_t    priority_t; /// Process priority type alias

/** @brief  Process control block structure */
typedef struct pcb_ {
    struct pcb_*    next;
    struct pcb_*    prev;
    pid_t           id;
    priority_t      priority;
    uint32_t*       sp_top;
    uint32_t*       sp;
    uint32_t        timer;
    pmsgbox_t*      msgbox;
} pcb_t;

typedef pid_t   proc_t;

#define PID_MAX 256

typedef struct pid_bitmap_ {
    uint8_t byte[((PID_MAX)/8)];
} pid_bitmap_t;

//typedef struct kernel_data_ {
//    pcb_t* running;
//    pmsgbox_t* free_mbox;
//    pmsgbox_t mbox[SYS_MSGBOXES];
//    pid_bitmap_t pid_bitmap;
//} kernel_t;

#endif
