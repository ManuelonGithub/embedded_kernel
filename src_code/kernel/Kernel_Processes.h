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

#define PSR_INIT_VAL    0x01000000

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
    uint32_t*       sp;
    process_state_t state;
    struct pcb_*    next;
    struct pcb_*    prev;
} pcb_t;

void set_LR(volatile uint32_t);
uint32_t get_PSP();
void set_PSP(volatile uint32_t);
uint32_t get_MSP(void);
void set_MSP(volatile uint32_t);
uint32_t get_SP();

void volatile save_registers();
void volatile restore_registers();

/**
 * @brief   CPU context structure.
 * @details This structure is aligned with how registers are placed onto the stack
 *          when a context switch occurs.
 *          By casting the process' stack pointer to a pointer to this structure,
 *          You are able to explicitly analyze the process' CPU context.
 */
typedef struct cpu_context_ {
    /* Registers saved by s/w (explicit) */
    /* There is no actual need to reserve space for R4-R11, other than
     * for initialization purposes.  Note that r0 is the h/w top-of-stack.
     */
    uint32_t r4;
    uint32_t r5;
    uint32_t r6;
    uint32_t r7;
    uint32_t r8;
    uint32_t r9;
    uint32_t r10;
    uint32_t r11;
    /* Stacked by hardware (implicit)*/
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;
    uint32_t pc;
    uint32_t psr;
} cpu_context_t;

#endif	//  KERNELL_PROCESSES_H
