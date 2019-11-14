/**
 * @file    k_cpu.h
 * @brief   Contains all definitions and entities regarding the
 *          cpu-specific implementation of the embedded kernel.
 * @details This is the module that opens the kernel to the cpu it runs on.
 *          Ideally this is the only module (.c & .h files) that needs to change in order to transfer
 *          the kernel from one cpu to another.
 * @author  Manuel Burnay
 * @date    2019.11.02  (Created)
 * @date    2019.11.03  (Last Modified)
 */

#ifndef K_CPU_H
#define K_CPU_H

#include <stdint.h>
#include "SysTick.h"

#define NVIC_INT_CTRL_R (*((volatile uint32_t*) 0xE000ED04))
#define TRIGGER_PENDSV 0x10000000
#define NVIC_SYS_PRI3_R (*((volatile uint32_t*) 0xE000ED20))
#define PENDSV_LOWEST_PRIORITY 0x00E00000

inline void PendSV_init();

/** @brief  Triggers the PendSV trap to be called. */
#define PendSV()    (NVIC_INT_CTRL_R |= TRIGGER_PENDSV)

/** @brief  SysTick Initialize alias function */
#define SystemTick_init(rate)  SysTick_Init(rate)

/** @brief  SysTick Reset alias function */
#define SystemTick_reset()  SysTick_Reset()

/** @brief  SysTick Start alias function */
#define SystemTick_resume() SysTick_Start()

/** @brief  SysTick Stop alias function */
#define SystemTick_pause()  SysTick_Stop()

/** @brief   Enables Interrupt Requests. */
#define ENABLE_IRQ() __asm(" cpsie i")

/** @brief   Disables Interrupt Requests. */
#define DISABLE_IRQ() __asm(" cpsid i")

/** @brief   Triggers the Supervisor (kernel) Trap. */
#define SVC()	__asm(" SVC #0")

/** @brief   Saves the Trap Return address to the kernel's stack. */
#define SaveTrapReturn()    __asm("     PUSH    {LR}")

/** @brief  Restores the Trap return address to the CPU's exception return link register */
#define RestoreTrapReturn()   __asm("     POP     {LR}")

#define PSR_INIT_VAL    0x01000000  /// CPU's Status Register initial value

/**
 * @brief   Process' CPU context structure.
 * @details This structure is laid out in a way that the
 *          context is used throughout the kernel/cpu.
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

typedef enum TRAP_SOURCES {KERNEL, USER} trap_sources_t;    /// Possible Sources of the trap call

inline trap_sources_t TrapSource();

inline void SaveContext();
inline void RestoreContext();

inline void SaveProcessContext();
inline void RestoreProcessContext();

inline void SetCallReg(volatile void* call);
inline void* GetCallReg();
inline void* GetProcessCall();

inline void InitProcessContext(uint32_t** sp, void (*proc_program)(), void (*exit_program)());

inline void SetPSP(volatile uint32_t ProcessStack);
inline uint32_t GetPSP();

inline void StartProcess();

#endif	// k_CPU_H
