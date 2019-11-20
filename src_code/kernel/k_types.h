

#ifndef K_TYPES_H
#define K_TYPES_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "dlist.h"
#include "k_defs.h"

typedef uint32_t    id_t;   /// System ID type alias
typedef id_t        pmbox_t; /// Message Box ID type alias
typedef id_t        proc_t; /// Process type alias

/** @brief  Internal Inter-process message structure */
typedef struct pmsg_ {
    union {
        struct {
            struct pmsg_*   next;
            struct pmsg_*   prev;
        };
        node_t list;
    };

    pmbox_t     src;
    pmbox_t     dst;
    size_t      size;
    uint8_t*    data;
} pmsg_t;

/** @brief  Inter-process communication Message box structure */
typedef struct pmsgbox_ {
    union {
        struct {
            struct pmsgbox_*    next;
            struct pmsgbox_*    prev;
        };
        node_t list;
    };

    struct pcb_*    owner;
    pmsg_t*         front_msg;
    pmsg_t*         wait_msg;
    pmbox_t         ID;
} pmsgbox_t;

typedef id_t        pid_t;      /// Process ID type alias
typedef uint32_t    priority_t; /// Process priority type alias

/** @brief  Process control block structure */
typedef struct pcb_ {
    union {
        struct {
            struct pcb_*    next;
            struct pcb_*    prev;
        };
        node_t list;
    };

    pid_t       id;
    priority_t  priority;
    uint32_t*   sp_top;
    uint32_t*   sp;
    uint32_t    timer;
    pmsgbox_t*  msgbox;
} pcb_t;

typedef struct pid_bitmap_ {
    uint8_t byte[((PID_MAX)/8)];
} pid_bitmap_t;

//typedef struct kernel_data_ {
//    pcb_t* running;
//    pmsgbox_t* free_mbox;
//    pmsgbox_t mbox[SYS_MSGBOXES];
//    pid_bitmap_t pid_bitmap;
//} kernel_t;

typedef uint32_t*   k_arg_t;
typedef uint32_t    k_ret_t;

#endif
