/**
 * @file    k_defs.h
 * @brief   Contains all kernel configuration definitions.
 * @author  Manuel Burnay
 * @date    2019.11.20  (Created)
 * @date    2019.11.29  (Last Modified)
 */


#ifndef K_DEFINITIONS_H
#define K_DEFINITIONS_H

#include "bitmap.h"

/************************** Scheduler Related Definitions **************************/

#define PRIORITY_LEVELS 5   /// Priority levels supported by the kernel.
#define IDLE_LEVEL      5   /// Index to the Idle queue

/** @brief Lowest priority supported by the system*/
#define LOWEST_PRIORITY PRIORITY_LEVELS-1
#define HIGH_PRIORITY   2   /// Highest priority that a user process can run on.
#define USER_PRIORITY   3   /// Default priority for user processes

#define PRIV0_PRIORITY  0   /// Privileged priority 0
#define PRIV1_PRIORITY  1   /// Privileged priority 1

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
#define PID_BITMAP_SIZE  1      /// Bitmap array size to cover all processes.
#else
    #define PID_BITMAP_SIZE  PID_MAX/BITMAP_WIDTH
#endif

/** @brief Error value for when an interaction with processes goes wrong. */
#define PROC_ERR        -1

typedef enum PROC_STATE {
    UNASSIGNED, WAITING_TO_RUN, RUNNING, BLOCKED, TERMINATED
} proc_state;   /// All possible states for the kernel processes to be in.

/***************************** IPC Related Definitions *****************************/

#define BOXID_MAX   16     /// Amount of Message boxes supported by the kernel

#define MSG_MAX     32     /// Amount of allocated messages in RT mode

#define MSG_MAX_SIZE 64    /// Max message size for messages in RT mode

#if (MSGBOXES_MAX/BITMAP_WIDTH == 0)
#define MSGBOX_BITMAP_SIZE  1   /// Bitmap array size to cover all allocated message.
#else
    #define MSGBOX_BITMAP_SIZE  BOXID_MAX/BITMAP_WIDTH
#endif

#if (MSG_MAX/BITMAP_WIDTH == 0)
#define MSG_BITMAP_SIZE  1      /// Bitmap array size to cover all message boxes.
#else
    /** @brief  Bitmap array size to cover all message boxes.*/
    #define MSG_BITMAP_SIZE  BOXID_MAX/BITMAP_WIDTH
#endif

/** @brief Error value for when an interaction with processes goes wrong. */
#define BOX_ERR     PROC_ERR

/** @brief Indicator that box ID is unimportant for the current operation. */
#define ANY_BOX     BOXID_MAX

#define IO_BOX      15  /// Reserved box ID for the IO server.

/** @brief All supported modes for a message box' interaction (WIP). */
typedef enum MSGBOX_MODES {RX_ONLY, TX_ONLY, RX_TX} msgbox_mode_t;

/** @brief All supported modes for a message box' auto-unbind feature (WIP). */
typedef enum AUTO_UNBIND {ON_SEND, ON_RECV, OFF} auto_unbind_t;

/************************ Kernel Calls Related Definitions *************************/

typedef enum KERNEL_CALL_CODES {
    PCREATE, STARTUP, GETPID, NICE,
    BIND, UNBIND, SEND,   RECV,
    REQUEST, GETBOX, SEND_USER, RECV_USER,
    GET_NAME, SET_NAME, TERMINATE
} k_code_t; /** All Kernel Calls supported to the user. */

#endif // K_DEFINITIONS_H






