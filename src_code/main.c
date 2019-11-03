/**
 * @file    main.c
 * @brief   Entry point of the embedded kernel.
 * @details Main is technically kernel code, but the user will place here
 *          its process creation requests so they get registered with the kernel.
 * @author  Manuel Burnay
 * @date    2019.10.22  (Created)
 * @date    2019.11.03  (Last Modified)
 */

#include "k_handlers.h"

/**
 * main.c
 */
int main(void)
{
    kernel_init();

    /* Place Process Create requests here */


    kernel_start();

	return 0;
}
