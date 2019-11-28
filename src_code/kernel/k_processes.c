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
#include <string.h>
#include "k_processes.h"
#include "k_scheduler.h"
#include "k_cpu.h"
#include "bitmap.h"

bitmap_t available_pid[PID_BITMAP_SIZE];

#ifdef RTS_PROCESSES

pcb_t   proc_table[PID_MAX];

#else

pcb_t*  proc_table[PID_MAX];

#endif

void process_init()
{
    int i;
    for (i = 0; i < PID_MAX; i++) {
#ifdef RTS_PROCESSES

        proc_table[i].id = i;

        proc_table[i].sp = proc_table[i].sp_top + (STACKSIZE/sizeof(uint32_t)) - 1;

        proc_table[i].next = NULL;
        proc_table[i].prev = NULL;

        proc_table[i].state = UNASSIGNED;

        ClearBitRange(proc_table[i].owned_box, 0, BOXID_MAX);

#else
        proc_table[i] = NULL;
#endif
    }

    ClearBitRange(available_pid, 0, PID_MAX);
}

/**
 * @brief   Creates a process and registers it in kernel space.
 * @param   [in] pid: Unique process identifier value.
 * @param   [in] priortity: Priority level that the process will run in.
 * @param   [in] proc_program:
 *              Pointer to start of the program the process will execute.
 * @retval  Returns the process ID that was created.
 *          0 if a process wasn't able to be created.
 */
pid_t k_pcreate(process_attr_t* attr, void (*program)(), void (*terminate)())
{
    pcb_t* pcb = NULL;

    pid_t id = (attr == NULL || attr->id == 0) ?
            FindClear(available_pid, 0, PID_MAX) : attr->id;

    priority_t priority = (attr == NULL || attr->priority == 0) ?
            DEF_PRIORITY : attr->priority;

    bool err = (id > PID_MAX || GetBit(available_pid, id) || priority > PRIORITY_LEVELS);

    if (!err) {
        pcb = k_AllocatePCB(id);
        pcb->state = WAITING_TO_RUN;

        if (attr != NULL && strlen(attr->name) != 0) {
            strcpy(pcb->name, attr->name);
        }
        else {
            strcpy(pcb->name, "no name");
        }

        InitProcessContext(&pcb->sp, program, terminate);
        LinkPCB(pcb, priority);
    }
    else {
        id = (pid_t)PROC_ERR;
    }

    return id;
}

/**
 * @brief   Allocates a new PCB.
 * @param   [in] id: ID of the PCB.
 * @return  Pointer to allocated PCB.
 *          NULL if PCB allocation failed.
 */
pcb_t* k_AllocatePCB(pid_t id)
{
#ifdef RTS_PROCESSES

    SetBit(available_pid, id);
    return &proc_table[id];

#else

    pcb_t* pcb = (pcb_t*)malloc(sizeof(pcb_t));
    if (pcb == NULL) return NULL;

    pcb->sp_top = (uint32_t*)malloc(STACKSIZE);
    if (pcb->sp_top == NULL) {
        free(pcb);
        return NULL;
    }

    proc_table[id] = pcb;

    pcb->id = id;

    pcb->sp = pcb->sp_top + (STACKSIZE/sizeof(uint32_t)) - 1;

    pcb->next = NULL;
    pcb->prev = NULL;

    pcb->state = UNASSIGNED;

    ClearBitRange(pcb->owned_box, 0, BOXID_MAX);

    SetBit(available_pid, id);

    return pcb;
#endif
}

/**
 * @brief   De-allocates a PCB.
 * @param   [in] id: Process ID to be de-allocated.
 */
inline void k_DeallocatePCB(pid_t id)
{
    ClearBit(available_pid, id);
    proc_table[id].state = TERMINATED;
}

pcb_t* GetPCB(pid_t id)
{
    if (id < PID_MAX) {
        return &proc_table[id];
    }
    return NULL;
}






