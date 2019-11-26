
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "k_terminal.h"
#include "k_types.h"
#include "calls.h"
#include "uart.h"
#include "k_defs.h"
#include "k_messaging.h"
#include "bitmap.h"

uint8_t term_state;
uint8_t term_size;
uint8_t active_msgbox[SYS_MSGBOXES/8];

void init_term(home_data_t* home)
{
    term_state = USER;
    term_size = 40;

    strcpy(home->idle, "=");
    strcpy(home->middle, "M'uh Kernel v0.1");

    uint8_t middle_offset =
            term_size/2 - strlen(home->middle)/2 - strlen(home->idle);

    int i = 0;
    for (i = 0; i < middle_offset; i++) {
        home->mid_buf[i] = ' ';
    }
    home->mid_buf[i] = '\0';

    uint8_t end_offset =
            term_size - (strlen(home->idle) + strlen(home->middle) + middle_offset);

    for (i = 0; i < end_offset; i++) {
        home->end_buf[i] = ' ';
    }
    memcpy(home->end_buf+i-strlen(home->idle), home->idle, strlen(home->idle));

    home->end_buf[i] = '\n';
    home->end_buf[i+1] = '\0';

    UART0_puts(CLEAR_SCREEN);
    UART0_puts(CURSOR_HOME);
    UART0_puts(TERM_COLOURS);
    UART0_puts("\n\n");

    for (i = 0; i < SYS_MSGBOXES/8; i++) {
        active_msgbox[i] = 0xFF;
    }
}

void send_home(home_data_t* home)
{
    UART0_puts(CURSOR_SAVE);
    UART0_puts(CURSOR_HOME);
    UART0_puts(CLEAR_LINE);
    UART0_puts(HOME_COLOURS);

    UART0_puts(home->idle);

    UART0_puts(home->mid_buf);

    UART0_puts(home->middle);

    UART0_puts(home->end_buf);

    UART0_puts(CURSOR_RESTORE);
}

void output_manager()
{
    pmbox_t box = bind(OUT_BOX);

    circular_buffer_t buffer;
    circular_buffer_init(&buffer);

    pmsg_t proc_rx = {
        .dst = box,
        .src = 0,
        .data = (uint8_t*)buffer.data,
        .size = CIRCULAR_BUFFER_SIZE
    };

    home_data_t home;

    init_term(&home);

    send_home(&home);

    while(1) {
        buffer.wr_ptr = recv_msg(&proc_rx);
        proc_rx.size = CIRCULAR_BUFFER_SIZE;

        if (GetBit(active_msgbox, proc_rx.src)) {
            while (buffer.rd_ptr < buffer.wr_ptr) {
                buffer.rd_ptr += UART0_put (
                        (buffer.data+buffer.rd_ptr), buffer_size(&buffer)
                    );
            }
        }

        buffer.wr_ptr = 0;
        buffer.rd_ptr = 0;
    }
}

void terminal()
{
    pmbox_t box = bind(IN_BOX);

    circular_buffer_t buffer;
    circular_buffer_init(&buffer);

    term_state = USER;

//    pmsg_t proc_tx = {
//        .dst = box,
//        .src = 0,
//        .data = (uint8_t*)buffer.data,
//        .size = CIRCULAR_BUFFER_SIZE
//    };

    home_data_t home;

    init_term(&home);

    int i = 0;

    for (i = 0; i < SYS_MSGBOXES/8; i++) {
        active_msgbox[i] = 0xFF;
    }

    char in_char;
//
//    bool proc_input_en = false;

    while (1) {
        if (UART_getc(&in_char)) {
            UART0_put(&in_char, 1);
        }
    }
}





