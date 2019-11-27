/**
 * @file    k_processes.c
 * @brief   Contains the process allocation management and all
 *          supporting functionality related to the kernel processes.
 * @author  Manuel Burnay
 * @date    2019.11.13  (Created)
 * @date    2019.11.21  (Last Modified)
 */


#include <stdint.h>
#include <stdlib.h>
#include "k_processes.h"
#include "bitmap.h"

uint8_t pid_bitmap[PID_MAX/8];

#ifdef RTS_PROCESSES

pcb_t   proc_table[PID_MAX];

#else

pcb_t*  proc_table[PID_MAX];

#endif

void process_init()
{
    int i;

    for (i = 0; i < PID_MAX; i++) {
        k_CreatePCB(i);
    }

    for(i = 0; i < PID_MAX/8; i++) {
        pid_bitmap[i] = 0;
    }
}

/**
 * @brief   Allocates and initializes a new PCB.
 * @param   [in] id: ID of the PCB. (*)
 * @return  Pointer to allocated PCB.
 *          NULL if PCB allocation failed.
 * @details (*) If the ID is 0, then the next available ID is used
 *          to allocate the PCB.
 */
pcb_t* k_AllocatePCB(pid_t id)
{
    pcb_t* pcb = NULL;

    if (id == 0) id = FindClear(pid_bitmap, 0, PID_MAX);

    if (id < PID_MAX && ~GetBit(pid_bitmap, id)) {
        SetBit(pid_bitmap, id);
        pcb = &proc_table[id];
        pcb->state = WAITING_TO_RUN;
    }

    return pcb;
}

/**
 * @brief   De-allocates a PCB.
 * @param   [in] id: Process ID to be de-allocated.
 */
inline void k_DeallocatePCB(pid_t id)
{
    ClearBit(pid_bitmap, id);
    proc_table[id].state = TERMINATED;
}

/**
 * @brief   Creates a Process Control block in Heap Memory.
 * @param   [in] id: Process ID to set PCB to.
 * @return  NULL if a process was unable to be created.
 *          pointer to created PCB if creation was successful.
 */
pcb_t* k_CreatePCB(pid_t id)
{
    pcb_t* pcb = NULL;

#ifdef  RTS_PROCESSES

    pcb = &proc_table[id];

#else

    pcb = (pcb_t*)malloc(sizeof(pcb_t));
    if (pcb == NULL) return NULL;

    pcb->sp_top = (uint32_t*)malloc(STACKSIZE);
    if (pcb->sp_top == NULL) {
        free(pcb);
        return NULL;
    }

    proc_table[id] = pcb;

#endif

    pcb->id = id;

    pcb->sp = pcb->sp_top+STACKSIZE;

    pcb->next = NULL;
    pcb->prev = NULL;

    pcb->state = UNALLOCATED;

    int i;
    for (i = 0; i < SYS_MSGBOXES/8; i++) {
        pcb->box_bitmap[i] = 0;
    }

    return pcb;
}

pcb_t* GetPCB(pid_t id)
{
    if (id < PID_MAX) {
        return &proc_table[id];
    }
    return NULL;
}






