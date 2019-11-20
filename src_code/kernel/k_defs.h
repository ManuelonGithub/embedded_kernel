


#ifndef K_DEFINITIONS_H
#define K_DEFINITIONS_H

#define PRIORITY_LEVELS 5   /** Priority levels supported by the kernel. */
#define IDLE_LEVEL      5   /** Index to the Idle queue */

#define LOWEST_PRIORITY 4   /// Lowest Priority supported by the kernel

#define STACKSIZE 2048      /// Stack size allocated for the processes.

#define PROC_RUNTIME 100    /// Time quantum of a process in ms

#define SYS_MSGBOXES 32     /// Amount of Message boxes supported by the kernel

#define PID_MAX 256         /// Maximum PID value supported

typedef enum KERNEL_CALL_CODES {
    PCREATE, STARTUP, GETID, NICE, BIND, UNBIND, SEND, RECV, TERMINATE
} k_code_t; /** Kernel Calls supported */

#endif // K_DEFINITIONS_H
