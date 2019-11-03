
#include <stdint.h>
#include "k_processes.h"

uint32_t get_PSP(void)
{
    /* Returns contents of PSP (current process stack */
    __asm(" mrs     r0, psp");
    __asm(" bx  lr");
    return 0;   /***** Not executed -- shuts compiler up */
            /***** If used, will clobber 'r0' */
}

uint32_t get_MSP(void)
{
    /* Returns contents of MSP (main stack) */
    __asm(" mrs     r0, msp");
    __asm(" bx  lr");
    return 0;
}

void set_PSP(volatile uint32_t ProcessStack)
{
    /* set PSP to ProcessStack */
    __asm(" msr psp, r0");
}

void set_MSP(volatile uint32_t MainStack)
{
    /* Set MSP to MainStack */
    __asm(" msr msp, r0");
}

void volatile save_registers()
{
    /* Save r4..r11 on process stack */
    __asm(" mrs     r0,psp");
    /* Store multiple, decrement before; '!' - update R0 after each store */
    __asm(" stmdb   r0!,{r4-r11}");
    __asm(" msr psp,r0");
}

void volatile restore_registers()
{
    /* Restore r4..r11 from stack to CPU */
    __asm(" mrs r0,psp");
    /* Load multiple, increment after; '!' - update R0 */
    __asm(" ldmia   r0!,{r4-r11}");
    __asm(" msr psp,r0");
}

uint32_t get_SP()
{
    /**** Leading space required -- for label ****/
    __asm("     mov     r0,SP");
    __asm(" bx  lr");
    return 0;
}
