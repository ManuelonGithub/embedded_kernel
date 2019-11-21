/**
 * @file    k_processes.c
 * @brief   Defines supporting functionality related to the kernel processes.
 * @author  Manuel Burnay
 * @date    2019.11.13  (Created)
 * @date    2019.11.21  (Last Modified)
 */


#include <stdint.h>
#include <stdlib.h>
#include "k_processes.h"

uint8_t pid_bitmap[PID_MAX/8];

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

    if (id == 0) id = FindFreePID();

    if (id < PID_MAX && AvailablePID(id)) {
        pcb = (pcb_t*)malloc(sizeof(pcb_t));
        if (pcb == NULL) return NULL;

        pcb->sp_top = (uint32_t*)malloc(STACKSIZE);
        if (pcb->sp_top == NULL) {
            free(pcb);
            return NULL;
        }

        pcb->id = id;

        pcb->sp = pcb->sp_top+STACKSIZE;

        pcb->next = NULL;
        pcb->prev = NULL;
        pcb->msgbox = NULL;

        SetPIDbit(id);
    }

    return pcb;
}

/**
 * @brief   De-allocates a PCB.
 */
inline void k_DeallocatePCB(pcb_t** pcb)
{
    ClearPIDbit((*pcb)->id);
    free((*pcb)->sp_top);
    free((*pcb));
}

/**
 * @brief   Finds an available PID in the bitmap to be allocated.
 */
pid_t FindFreePID()
{
    pid_t i = 0;
    bool pid_found = false;
    while (i < PID_MAX && !pid_found) {
        if (!AvailablePID(i))   i++;
        else                    pid_found = true;
    }

    return i;
}

/**
 * @brief   Sets a PID bit in the bitmap to indicate that it's taken.
 * @param   [in] PID value.
 */
inline void SetPIDbit(pid_t pid)
{
    /*
     * bitmap is comprised of an array of bytes
     * (smallest addressable element).
     * So it find a specific bit within the bitmap,
     * two values need to be derived:
     * (pid >> 3) derives the byte index of the
     * bitmap associated with the bit (pid).
     * (1 << (pid & 7)) derives the location of the bit
     * (pid) within the addressed byte.
     */
    pid_bitmap[(pid >> 3)] |= (1 << (pid & 7));
}

/**
 * @brief   Clears a PID bit from the bitmap to indicate that it's available.
 * @param   [in] PID value.
 */
inline void ClearPIDbit(pid_t pid)
{
    pid_bitmap[(pid >> 3)] &= ~(1 << (pid & 7));
}

/**
 * @brief   Checks if a PID value is available to be assigned.
 * @param   [in] PID value.
 * @return  True if PID value is available,
 *          False if not.
 */
inline bool AvailablePID(pid_t pid)
{
    return !(pid_bitmap[pid >> 3] & (1 << (pid & 7)));
}






