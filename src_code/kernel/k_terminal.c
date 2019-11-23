
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

char CLEAR_SCREEN[]     = {"\x1b[2J"};
char CURSOR_SAVE[] 		= {"\x1b""7"};
char CURSOR_HOME[] 		= {"\x1b[H"};
char CLEAR_LINE[]       = {"\x1b[2K"};
char HOME_COLOURS[]     = {"\x1b[2;30;47m"};
char CURSOR_MIDDLE[] 	= {"\x1b[20C"};
char TERM_COLOURS[]     = {"\x1b[0;0;0m"};
char CURSOR_RESTORE[] 	= {"\x1b""8"};

uint8_t term_state;
pid_t	active_pid;
uint8_t term_size;


void init_term(home_data_t* home)
{
    term_state = PROCESS_STREAM;
    active_pid = 0;
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

void term_out()
{
	pmbox_t box = bind(OUT_BOX);

    circular_buffer_t buffer;
    circular_buffer_init(&buffer);

    home_data_t home;

    init_term(&home);

    pmsg_t proc_rx = {
        .dst = box,
        .src = 0,
        .data = (uint8_t*)buffer.data,
        .size = CIRCULAR_BUFFER_SIZE
    };

    while(1) {
        buffer.wr_ptr = recv_msg(&proc_rx);

        switch (proc_rx.src) {
        	case IDLE_BOX: {
//        		modify_home(idle, )
        	} break;

        	case IN_BOX: {
        		if (term_state == USER_TERMINAL) {
        			while (buffer.rd_ptr < buffer.wr_ptr) {
            			buffer.rd_ptr +=
                    		UART0_put((buffer.data+buffer.rd_ptr),
                    		          buffer_size(&buffer));
        			}
        		}
        	} break;

        	default: {
                switch (term_state) {
                    case (PROCESS_FOCUS): {
                        // Don't output if message source isn't from
                        // the currently active PID
                        if (active_pid != OwnerID(proc_rx.src)) break;
                    }
                    case (PROCESS_STREAM): {
                        while (buffer.rd_ptr < buffer.wr_ptr) {
                            buffer.rd_ptr += UART0_put((buffer.data+buffer.rd_ptr),
                                    buffer_size(&buffer));
                        }
                    } break;
                    default:
                        break;
                }
        	} break;
        }

        buffer.wr_ptr = 0;
        buffer.rd_ptr = 0;

        send_home(&home);
    }
}

void term_in()
{

}







