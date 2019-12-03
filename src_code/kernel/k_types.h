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
        node_t list;    /**< list node where other messages connect to. */
    };

    pmbox_t     src;    /**< Box ID where message was sent from. */
    pmbox_t     dst;    /**< Box ID where message is meant to go. */
    size_t      size;   /**< Size of the message contents (in Bytes). */
    id_t        id;     /**< Internal ID number used for msg allocation. */
    uint8_t*    data;   /**< Pointer to Location of the message data. */
} pmsg_t;

/** @brief  Message box attributes structure. **WIP** */
typedef struct msgbox_attr_ {
    pmbox_t         id;             /**< Message Box ID. */
    msgbox_mode_t   mode;           /**< Message box mode/permissions. */
} msgbox_attr_t;

/** @brief  Inter-process communication Message box structure */
typedef struct pmsgbox_ {
    struct pcb_*    owner;      /**< Pointer to owner PCB */
    pmbox_t         id;         /**< Message box ID */
    msgbox_mode_t   mode;       /**< Message box mode/permissions. **WIP** */
    pmsg_t*         recv_msgq;  /**< Pointer to the receive message list queue. */
    pmsg_t*         wait_msg;   /**< Pointer to a pending receive request message. */
    size_t*         retsize;
} pmsgbox_t;

typedef id_t        pid_t;      /// Process id type alias
typedef uint32_t    priority_t; /// Process priority type alias

/** @brief  Process attributes structure.
 *          Used to configure a process with the attributes.
 */
typedef struct process_attr_ {
    pid_t       id;         /**< Process ID. */
    priority_t  priority;   /**< Process priority. */
    char        name[32];   /**< Process name. */
} process_attr_t;

/** @brief  Process control block structure */
typedef struct pcb_ {
    union {
        struct {
            struct pcb_*    next;
            struct pcb_*    prev;
        };
        node_t list;    /**< List node used for priority queuing. */
    };

    pid_t       id;         /**< Process ID. */
    priority_t  priority;   /**< Process priority. */
    char        name[32];   /**< Process name. */
    uint32_t    sp_top[STACKSIZE/sizeof(uint32_t)]; /**< Process stack. */
    uint32_t*   sp;         /**< Process stack pointer. */
    int32_t     timer;      /**< Process timer. */
    proc_state  state;      /**< Process state */
    bitmap_t    owned_box[MSGBOX_BITMAP_SIZE];      /**< Process owned box' bitmap. */
} pcb_t;

typedef uint32_t*   k_arg_t;    /// Kernel call argument type alias
typedef uint32_t    k_ret_t;    /// Kernel call return value type alias

/** @brief Kernel Call structure */
typedef struct kernel_call_arguments_ {
    k_code_t    code;   /**< Kernel call code. */
    k_ret_t     retval; /**< Kernel call return value. */
    k_arg_t     arg;    /**< Kernel call argument, */
} k_call_t ;

/**
 * @brief   IO request metadata structure.
 *          Data structure used to perform requests to the IO server.
 *          for both input and output.
 */
typedef struct IO_metadata_ {
    pmbox_t     box_id;     /**< Msg box that the request originated from. */
    pid_t       proc_id;    /**< Process ID that made the request. */
    bool        is_send;    /**< True if the request is to output data. */
    size_t      size;       /**< Size of request data (for both input and output) */
    uint8_t*    send_data;  /**< Pointer to output data buffer */
} IO_metadata_t;

/**
 * @brief   data format sent from the UART driver to the IO server.
 */
typedef struct uart_msgdata_ {
    pmbox_t     box_id; /**< Box ID. Should always be the IO_BOX. */
    char        c;      /**< character to send to IO server. */
} uart_msgdata_t;

#endif
