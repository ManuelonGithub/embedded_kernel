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


/**
 * main.c
 */
int main(void)
{
    kernel_init();

    /* Place Process Create requests here */
    pcreate(10, 0, &rx_test);
    pcreate(11, 0, &tx_test);

    kernel_start();

	return 0;
}
