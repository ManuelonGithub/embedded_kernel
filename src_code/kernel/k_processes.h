/**
 * @file    k_processes.h
 * @brief   Contains all functions and entities related to
 *          processes and process control blocks.
 * @details This module should not be exposed to user programs.
 * @author  Manuel Burnay
 * @date    2019.10.23 (Created)
 * @date    2019.10.23 (Last Modified)
 */

#ifndef     K_PROCESSES_H
#define     K_PROCESSES_H

#include <stdint.h>

#define STACKSIZE	1024

#define PSR_INIT_VAL    0x01000000

typedef enum PROCESS_STATES_ {
    WAITING_TO_RUN = -1, RUNNING, SEND_WAIT, RECV_WAIT, DELAY_WAIT, TERMINATED
} process_state_t;

typedef enum PCB_HANDLE_CODES_ {
    INVALID_PID = -2, INVALID_PRIORITY, HANDLE_SUCCESS
} pcb_handle_code_t;

/** Process control block structure */
typedef struct pcb_ {
    struct pcb_*    next;
    struct pcb_*    prev;
    uint32_t        id;
    uint32_t        priority;
    uint32_t*       sp_bottom;
    uint32_t*       sp;
    process_state_t state;
} pcb_t;

void set_LR(volatile uint32_t);
uint32_t get_PSP();
void set_PSP(volatile uint32_t);
uint32_t get_MSP(void);
void set_MSP(volatile uint32_t);
uint32_t get_SP();

void volatile save_registers();
void volatile restore_registers();

#endif	//  K_PROCESSES_H
