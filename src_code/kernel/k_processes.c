
#include <stdint.h>
#include "k_processes.h"

bool k_CreatePCB(pcb_t* pcb, pid_t id);
{
    pcb_t* newPCB = (pcb_t*)malloc(sizeof(pcb_t));
    if (newPCB == NULL) return false;

    newPCB->sp_top = (uint32_t*)malloc(STACKSIZE);  // If malloc fails it returns 0
    if (newPCB->sp_top == NULL) return false;

    newPCB->id = pid;

    newPCB->sp = newPCB->sp_top+STACKSIZE;

    newPCB->next = NULL;
    newPCB->prev = NULL;
}

void k_DeletePCB(pcb_t* pcb)
{
    free(pcb->sp_top);
    free(pcb);
}
