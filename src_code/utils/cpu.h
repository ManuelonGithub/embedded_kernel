/**
 *	@file	cpu.h
 *	@brief	Has some general functionality/information about the C-M4 cpu.
 *	@author	Manuel Burnay
 *	@date 	2019.09.24	(Created)
 *	@date	2019.09.30	(Last Modified)
 */

#ifndef CPU_H
	#define CPU_H


	#define ENABLE_IRQ() __asm(" cpsie i")
	#define DISABLE_IRQ() __asm(" cpsid i")

	#define SVC()	__asm(" SVC #0")

	#define F_CPU_CLK	16000000

#endif // CPU_H
