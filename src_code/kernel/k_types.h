/**
 * @file    k_types.h
 * @brief   Defines all data types used through the kernel
 * @author  Manuel Burnay
 * @date    2019.11.19 (Created)
 * @date    2019.11.21 (Last Modified)
 */

#ifndef K_TYPES_H
#define K_TYPES_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "dlist.h"
#include "bitmap.h"
#include "k_defs.h"

typedef uint32_t    id_t;       /// System ID type alias
typedef id_t        pmbox_t;    /// Message Box ID type alias

/** @brief Inter-process message structure */
typedef struct pmsg_ {
    // This union makes the coding for the linked list a lot cleaner
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
    id_t        id;
    uint8_t*    data;
} pmsg_t;

typedef struct msgbox_attr_ {
    pmbox_t id;
    msgbox_mode_t mode;
    auto_unbind_t auto_unbind;
} msgbox_attr_t;

/** @brief  Inter-process communication Message box structure */
typedef struct pmsgbox_ {
    struct pcb_*    owner;
    pmbox_t         id;
    msgbox_mode_t   mode;
    auto_unbind_t   auto_unbind;
    pmsg_t*         recv_msgq;  // Messages to be received
    pmsg_t*         wait_msg;   // Pointer to a pending RECV message
    size_t*         retsize;
} pmsgbox_t;

typedef id_t        pid_t;      /// Process id type alias
typedef uint32_t    priority_t; /// Process priority type alias

typedef struct process_attr_ {
    pid_t       id;
    priority_t  priority;
    char        name[32];
} process_attr_t;

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
    char        name[32];

#ifdef  REAL_TIME_MODE

    uint32_t    sp_top[STACKSIZE/sizeof(uint32_t)];

#else

    uint32_t*   sp_top;

#endif

    uint32_t*   sp;
    int32_t     timer;
    proc_state  state;
    bitmap_t    owned_box[MSGBOX_BITMAP_SIZE];
} pcb_t;

typedef uint32_t*   k_arg_t;    /// Kernel call argument type alias
typedef uint32_t    k_ret_t;    /// Kernel call return value type alias

/** @brief Kernel Call structure */
typedef struct kernel_call_arguments_ {
    k_code_t    code;
    k_ret_t     retval;
    k_arg_t     arg;
} k_call_t ;

typedef struct IO_metadata_ {
    pmbox_t     box_id;
    pid_t       proc_id;
    bool        is_send;
    size_t      size;
    uint8_t*    send_data;
} IO_metadata_t;

typedef struct uart_msgdata_ {
    pmbox_t     box_id;
    char        c;
} uart_msgdata_t;

#endif
