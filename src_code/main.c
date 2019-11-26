/**
 * @file    main.c
 * @brief   Entry point of the embedded kernel.
 * @details Main is technically kernel code, but the user will place here
 *          its process creation requests so they get registered with the kernel.
 * @author  Manuel Burnay
 * @date    2019.10.22  (Created)
 * @date    2019.11.21  (Last Modified)
 */

#include "k_handlers.h"
#include "calls.h"
#include <string.h>

void rx_test(void)
{
    pid_t rx_id = getpid();
    pmbox_t rx_box = bind(rx_id);

    pid_t tx_id = (uint32_t)(-1);

    recv(rx_box, 11, (uint8_t*)&tx_id, sizeof(pid_t));
    send(11, rx_box, (uint8_t*)&rx_id, sizeof(pid_t));

    while(1);
}

void tx_test(void)
{
    pid_t tx_id = getpid();
    pmbox_t tx_box = bind(tx_id);

    pid_t rx_id = (pid_t)(-1);

    send(10, tx_box, (uint8_t*)&tx_id, sizeof(pid_t));
    recv(tx_box, 10, (uint8_t*)&rx_id, sizeof(pid_t));

    while(1);
}

void output_test()
{
    pid_t id = getpid();
    pmbox_t box = bind(0);

    char text[64];

    sprintf(text, "Hello from process %u!\n", id);

    send(OUT_BOX, box, (uint8_t*)text, strlen(text));
}


/**
 * @brief   main.c
 * @details Initializes the kernel and all processes to run in the system.
 *          It also start's kernel's "run-mode".
 *
 */
int main(void)
{
    kernel_init();

    /* Place Process Create requests here */
    int i = 0;

//    for (i = 0; i < 11; i++) {
//        pcreate(0, 1, &output_test);
//    }

    kernel_start();

	return 0;
}
