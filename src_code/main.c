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
#include "calls.h"


void proc_test(void)
{
    pid_t id = getpid();
    pmbox_t box;

    if (bind(&box, 0) == 0) {
        unbind(&box);
    }

    while(1) {}
}

/**
 * main.c
 */
int main(void)
{
    kernel_init();

    /* Place Process Create requests here */
    pcreate(NULL, 0, 0, &proc_test);
    pcreate(NULL, 0, 0, &proc_test);

    kernel_start();

	return 0;
}
