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

/**
 * @brief   Total amount of process levels the kernel scheduler accepts.
 *          +1 for the "Idle" queue
 */
#define PROCESS_QUEUES  PRIORITY_LEVELS+1

/*************************** Process Related Definitions ***************************/

#define STACKSIZE       2048    /// Stack size allocated for the processes.

#define PROC_RUNTIME    100     /// Time quantum of a process in ms

#define PID_MAX         16      /// Maximum Processes supported.

#define RTS_PROCESSES           /// Enables "Real Time Systems" process management

typedef enum PROC_STATE {
    UNALLOCATED, WAITING_TO_RUN, RUNNING, BLOCKED, TERMINATED
} proc_state;

/***************************** IPC Related Definitions *****************************/

#define BOXID_MAX   16     /// Amount of Message boxes supported by the kernel

#define IDLE_BOX    0

#define OUT_BOX     15

#define IN_BOX      14

/************************ Kernel Calls Related Definitions *************************/

typedef enum KERNEL_CALL_CODES {
    PCREATE, STARTUP, GETPID, NICE, BIND, UNBIND,
    SEND, RECV, GETONHOLD, WAKE_TERMINAL, TERMINATE
} k_code_t; /** Kernel Calls supported */

#endif // K_DEFINITIONS_H
