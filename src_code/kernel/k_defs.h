/**
 * @file    k_defs.h
 * @brief   Contains all kernel configuration definitions.
 * @author  Manuel Burnay
 * @date    2019.11.20  (Created)
 * @date    2019.11.21  (Last Modified)
 */


#ifndef K_DEFINITIONS_H
#define K_DEFINITIONS_H

#include "bitmap.h"

/************************** Scheduler Related Definitions **************************/

#define PRIORITY_LEVELS 5   /// Priority levels supported by the kernel.
#define IDLE_LEVEL      5   /// Index to the Idle queue

#define LOWEST_PRIORITY 4   /// Lowest Priority supported by the kernel
#define DEF_PRIORITY    3
#define PRIV_PRIORITY   0

/**
 * @brief   Total amount of process levels the kernel scheduler accepts.
 *          +1 for the "Idle" queue
 */
#define PROCESS_QUEUES  PRIORITY_LEVELS+1

/*************************** Process Related Definitions ***************************/

#define STACKSIZE       2048    /// Stack size allocated for the processes.

#define PROC_RUNTIME    100     /// Time quantum of a process in ms

#define PID_MAX         16      /// Maximum Processes supported.

#define IDLE_ID         0

#if (PID_MAX/BITMAP_WIDTH == 0)
#define PID_BITMAP_SIZE  1
#else
    #define PID_BITMAP_SIZE  PID_MAX/BITMAP_WIDTH
#endif

/** @brief Error value for when an interaction with processes goes wrong. */
#define PROC_ERR        -1

#define REAL_TIME_MODE   /// Enables "Real Time Systems" process management

typedef enum PROC_STATE {
    UNASSIGNED, WAITING_TO_RUN, RUNNING, BLOCKED, TERMINATED
} proc_state;

/***************************** IPC Related Definitions *****************************/

#define BOXID_MAX   16     /// Amount of Message boxes supported by the kernel

#define MSG_MAX     32     /// Amount of allocated messages in RT mode

#define MSG_MAX_SIZE 64    /// Max message size for messages in RT mode

#if (MSGBOXES_MAX/BITMAP_WIDTH == 0)
#define MSGBOX_BITMAP_SIZE  1
#else
    #define MSGBOX_BITMAP_SIZE  BOXID_MAX/BITMAP_WIDTH
#endif

#if (MSG_MAX/BITMAP_WIDTH == 0)
#define MSG_BITMAP_SIZE  1
#else
    #define MSG_BITMAP_SIZE  BOXID_MAX/BITMAP_WIDTH
#endif

/** @brief Error value for when an interaction with processes goes wrong. */
#define BOX_ERR     PROC_ERR

#define ANY_BOX     BOXID_MAX

#define IO_BOX      15

typedef enum MSGBOX_MODES {RX_ONLY, TX_ONLY, RX_TX} msgbox_mode_t;

typedef enum AUTO_UNBIND {ON_SEND, ON_RECV, OFF} auto_unbind_t;

/************************ Kernel Calls Related Definitions *************************/

typedef enum KERNEL_CALL_CODES {
    PCREATE, STARTUP, GETPID, NICE, BIND, UNBIND,
    SEND, RECV, REQUEST, GETBOX, SEND_USER, RECV_USER, TERMINATE
} k_code_t; /** Kernel Calls supported */

#endif // K_DEFINITIONS_H






