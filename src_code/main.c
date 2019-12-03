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

void inout_test()
{
    pid_t id = getpid();
    pmbox_t box = bind(ANY_BOX);

    char text[128], name[64];

    sprintf(text, "Hello from process %u!\nWhat is your name? ", id);

    while (send_user(text) == 0);

    while (recv_user(name, 64) == 0);

    send(10, box, (uint8_t*)name, strlen(name)+1);
}

void ipc_test()
{
    pmbox_t box = bind(10);
    char text[128], name[64];

    int i;
    for (i = 0; i < 2; i++) {
        recv(box, ANY_BOX, (uint8_t*)name, 64);
        sprintf(text, "Name %d: %s\n", i, name);

        while (send_user(text) == 0);
    }
}

void name_test()
{
    char name[64], name_new[] = {"name tester"};

    get_name(name);
    set_name(name_new);

    send_user("Old name was: ");
    send_user(name);

    get_name(name);
    send_user("\nNew name is: ");
    send_user(name);

    while(1) {}
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
    pcreate(NULL, &name_test);
//    pcreate(NULL, &inout_test);
//    pcreate(NULL, &inout_test);
//    pcreate(NULL, &ipc_test);

    kernel_start();

	return 0;
}



