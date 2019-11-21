/**
 * @file    k_defs.h
 * @brief   Contains all kernel configuration definitions.
 * @author  Manuel Burnay
 * @date    2019.11.20  (Created)
 * @date    2019.11.21  (Last Modified)
 */


#ifndef K_DEFINITIONS_H
#define K_DEFINITIONS_H


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

#define STACKSIZE 2048      /// Stack size allocated for the processes.

#define PROC_RUNTIME 100    /// Time quantum of a process in ms

#define PID_MAX 256         /// Maximum PID value supported


/***************************** IPC Related Definitions *****************************/

#define SYS_MSGBOXES 32     /// Amount of Message boxes supported by the kernel


/************************ Kernel Calls Related Definitions *************************/

typedef enum KERNEL_CALL_CODES {
    PCREATE, STARTUP, GETPID, NICE, BIND, UNBIND, SEND, RECV, TERMINATE
} k_code_t; /** Kernel Calls supported */

#endif // K_DEFINITIONS_H
