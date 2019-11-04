/**
 * @file    k_cpu.c
 * @brief   Contains the implementations of all CPU specific functionality
 *          that the kernel requires to operate.
 * @details This is the module that opens the kernel to the CPU it runs on.
 *          Ideally this is the only module (.c & .h files) that needs to change in order to transfer
 *          the kernel from one CPU to another (while still maintaining the same function names).
 * @author  Manuel Burnay
 * @date    2019.11.02  (Created)
 * @date    2019.11.03  (Last Modified)
 */

#include <stdio.h>
#include "k_cpu.h"

/**
 * @brief   Initializes the CPU to support the Pending Supervisor trap.
 * @details A Pending Supervisor Trap is configured to be the lowest-priority trap.
 */
inline void PendSV_init()
{
    NVIC_SYS_PRI3_R |= PENDSV_LOWEST_PRIORITY;
}

/**
 * @brief   Determines the source of a trap call.
 * @retval  A code value related to the the source.
 *          See trap_souces_t for more information.
 * @details This function relies on the trap return address to be saved in the kernel stack.
 * @todo    See about cleaning up the assembly code: http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.kui0097a/armcc_cihccdja.htm
 */
inline trap_sources_t TrapSource() {
    __asm("     LDR r0,[sp]");
	__asm(" 	ANDS r0,r0,#4");
	__asm(" bx  lr");
    return KERNEL; // function will return with the line above, this is just to remove compiler warning
}

/** @brief  Saves the current CPU context onto the running stack. */
inline void SaveContext() {
    __asm("     PUSH    {r4-r11}");
}

/** @brief  Restores the current CPU context from the running stack. */
inline void RestoreContext() {
    __asm("     POP {r4-r11}");
}

/**
 * @brief   Saves the CPU context of the process that was running before.
 */
inline void SaveProcessContext() {
    /* Save r4..r11 on process stack */
    __asm(" mrs     r0,psp");
    /* Store multiple, decrement before; '!' - update R0 after each store */
    __asm(" stmdb   r0!,{r4-r11}");
    __asm(" msr psp,r0");
}

/**
 * @brief   Restores the CPU context of the process that was running before.
 */
inline void RestoreProcessContext() {
    /* Restore r4..r11 from stack to CPU */
    __asm(" mrs r0,psp");
    /* Load multiple, increment after; '!' - update R0 */
    __asm(" ldmia   r0!,{r4-r11}");
    __asm(" msr psp,r0");
}

/**
 * @brief   Sets the designated kernel call register with a pointer to a kernel call structure.
 */
inline void SetCallReg(volatile void* call) {
    __asm("     mov r7,r0");
}

/**
 * @brief   Gets the pointer to a kernel call structure out of the designated kernel call register.
 */
inline void* GetCallReg() {
    __asm("     mov r0,r7");
	__asm(" bx  lr");
	return NULL;
}

/**
 * @brief   Initializes the CPU context of a process.
 */
inline void InitProcessContext(uint32_t** sp, void (*proc_program)(), void (*exit_program)()) {

    *sp -= sizeof(cpu_context_t);

    cpu_context_t* cpu = (cpu_context_t*)(*sp);

	cpu->psr = PSR_INIT_VAL;
    cpu->lr = (uint32_t)exit_program;
    cpu->pc = (uint32_t)proc_program;

}

/**
 * @brief   Sets the current process stack pointer value.
 * @param   [in] ProcessStack: Stack pointer value to set the current process stack to.
 */
inline void SetPSP(volatile uint32_t ProcessStack)
{
    /* set PSP to ProcessStack */
    __asm(" msr psp, r0");
}

/**
 * @brief   Retrieves the current process' stack pointer value.
 * @return  The current stack pointer's value.
 */
inline uint32_t GetPSP()
{
    /* Returns contents of PSP (current process stack */
    __asm(" mrs     r0, psp");
    __asm(" bx  lr");
    return 0;   /* Not executed -- removes compiler warning */
}

/**
 * @brief   Forces the machine to switch to a process' context when in handler mode.
 * @details This does require the process stack to already be set up
 *          And with the proper register values in it.
 */
inline void StartProcess()
{
    __asm(" movw    LR,#0xFFFD");  /* Lower 16 [and clear top 16] */
    __asm(" movt    LR,#0xFFFF");  /* Upper 16 only */
    __asm(" bx  LR");          /* Force return to PSP */
}



