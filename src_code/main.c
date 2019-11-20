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
    pmbox_t box = bind(0);

    uint32_t tx = 100, rx = 0;
    send(box, box, (uint8_t*)&tx, sizeof(uint32_t));
    recv(box, box, (uint8_t*)&rx, sizeof(uint32_t));

    while(1) {}
}

/**
 * main.c
 */
int main(void)
{
    kernel_init();

    /* Place Process Create requests here */
    pcreate(0, 0, &proc_test);
    pcreate(0, 0, &proc_test);

    kernel_start();

	return 0;
}
