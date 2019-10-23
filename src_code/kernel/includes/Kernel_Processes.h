/**
 * @file    Kernel_Processes.h
 * @brief   Contains all functions and entities related to
 *          processes and process control blocks.
 * @details This module should not be exposed to user programs.
 * @author  Manuel Burnay
 * @date    2019.10.23 (Created)
 * @date    2019.10.23 (Last Modified)
 */

#ifndef     KERNELL_PROCESSES_H
#define     KERNELL_PROCESSES_H

#include <stdint.h>

#define STACKSIZE	1024

typedef enum PROCESS_STATES_ {
    WAITING_TO_RUN = -1, RUNNING, SEND_WAIT, RECV_WAIT, DELAY_WAIT, TERMINATED
} process_state_t;

typedef enum PCB_HANDLE_CODES_ {
    INVALID_PID = -2, INVALID_PRIORITY, HANDLE_SUCCESS
} pcb_handle_code_t;

/** Process control block structure */
typedef struct pcb_ {
    uint32_t        id;
    uint32_t        priority;
    uint32_t        sp;
    process_state_t state;
    struct _pcb*    next;
    struct _pcb*    prev;
} pcb_t;

#endif	//  KERNELL_PROCESSES_H
