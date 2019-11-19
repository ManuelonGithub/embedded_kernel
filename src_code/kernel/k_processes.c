
#include <stdint.h>
#include <stdlib.h>
#include "k_processes.h"

bool k_CreatePCB(pcb_t** pcb, pid_t id)
{
    (*pcb) = (pcb_t*)malloc(sizeof(pcb_t));
    if (*pcb == NULL) return false;

    (*pcb)->sp_top = (uint32_t*)malloc(STACKSIZE);  // If malloc fails it returns 0
    if ((*pcb)->sp_top == NULL) {
        free((*pcb));
        return false;
    }

    (*pcb)->id = id;

    (*pcb)->sp = (*pcb)->sp_top+STACKSIZE;

    (*pcb)->next = NULL;
    (*pcb)->prev = NULL;
    (*pcb)->msgbox = NULL;

    return true;
}

inline void k_DeletePCB(pcb_t** pcb)
{
    free((*pcb)->sp_top);
    free((*pcb));
}
