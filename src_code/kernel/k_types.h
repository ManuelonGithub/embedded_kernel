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

#if (PID_MAX/BITMAP_WIDTH == 0)
#define PID_BITMAP_SIZE  1
#else
    #define PID_BITMAP_SIZE  PID_MAX/BITMAP_WIDTH
#endif

#if (MSGBOXES_MAX/BITMAP_WIDTH == 0)
#define MSGBOX_BITMAP_SIZE  1
#else
    #define MSGBOX_BITMAP_SIZE  BOXID_MAX/BITMAP_WIDTH
#endif

typedef uint32_t    id_t;       /// System ID type alias
typedef id_t        pmbox_t;    /// Message Box ID type alias
typedef id_t        proc_t;     /// Process type alias

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
    uint8_t*    data;
} pmsg_t;

/** @brief  Inter-process communication Message box structure */
typedef struct pmsgbox_ {
    struct pcb_*    owner;
    pmsg_t*         recv_msgq;  // Messages to be received
    pmsg_t*         send_msgq;  // Messages waiting on a send
    pmsg_t*         wait_msg;   // Pointer to "rx any" message request
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

#ifdef  RTS_PROCESSES

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

#endif
