
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "k_terminal.h"
#include "calls.h"
#include "uart.h"
#include "k_messaging.h"
#include "k_processes.h"
#include "bitmap.h"
#include "cstr_utils.h"

enum TERMINAL_COMMANDS {SYSTEM_VIEW, SET_PROCESS_FOCUS, T_COMMAND_SIZE};

const char* const COMMAND[] =
{
    "SYS", "SETP"
};

bool (* const CommHandler[])(char*) =
{
    SystemView,
    SetProcessFocus
};

uint8_t active_msgbox[SYS_MSGBOXES/8];

void init_term(terminal_t* term)
{
    term->mode = USER;
    term->line_size = 40;

    int i;
    for (i = 0; i < SYS_MSGBOXES/8; i++) {
        active_msgbox[i] = 0;
    }

    term->buf_entry = 0;
    circular_buffer_init(&term->in_buf);

    term->proc_in = false;
    term->proc_box = bind(IN_BOX);

    create_home_line(term->home_line, term->line_size);
}

void create_home_line(char* home, uint32_t term_size)
{
    uint8_t mid_off = term_size/2 - strlen(HOME_TEXT)/2 - strlen(HOME_HEADER);
    uint8_t end_offset =
            term_size - (strlen(HOME_HEADER) + strlen(HOME_TEXT) + mid_off);

    strcat(home, HOME_HEADER);

    int i;
    for (i = 0; i < mid_off; i++) {
        strcat(home, " ");
    }

    strcat(home, HOME_TEXT);

    for (i = 0; i < end_offset; i++) {
        strcat(home, " ");
    }

    strcat(home, HOME_HEADER);
}

void send_home(char* home)
{
    UART0_puts(CURSOR_SAVE);
    UART0_puts(CURSOR_HOME);
    UART0_puts(CLEAR_LINE);
    UART0_puts(HOME_COLOURS);

    UART0_puts(home);

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

void ResetTerminalScreen()
{
    UART0_puts(CLEAR_SCREEN);
    UART0_puts(CURSOR_HOME);
    UART0_puts(TERM_COLOURS);
    UART0_puts("\n\n");
    UART0_puts("> ");
}

void terminal()
{
    terminal_t term;

    init_term(&term);

    int i = 0;

    for (i = 0; i < SYS_MSGBOXES/8; i++) {
        active_msgbox[i] = 0xFF;
    }

    term.mode = PROCESS_STREAM;

    char in_char;

    ResetTerminalScreen();
    send_home(term.home_line);

    while (1) {
        if (UART0_getc(&in_char)) {
            if (in_char == TERM_ESC) {
                circular_buffer_init(&term.in_buf);
                term.buf_entry = 0;
                ResetTerminalScreen();
                send_home(term.home_line);
            }
            else if (term.mode == USER) {
                UART0_put(&in_char, 1);
                user_command_analysis(in_char, &term);
            }

        }
        else if (term.mode != USER){
            nice(LOWEST_PRIORITY);
        }
    }
}

void user_command_analysis(char c, terminal_t* term)
{
    switch (c) {
        case '\b':
        case 0x7F: {
            if (term->in_buf.wr_ptr > 0) {
                term->in_buf.wr_ptr--;
                term->buf_entry--;
            }
            else {
                UART0_puts(" ");
            }
        } break;

        case '\r':
        case '\n': {
            enqueuec_s(&term->in_buf, '\0', false);
            if (!CommandCheck(term->in_buf.data, term->in_buf.wr_ptr)) {
                UART0_puts("? \n");
            }

            term->buf_entry = 0;
            term->in_buf.wr_ptr = 0;

            UART0_puts("> ");
        } break;

//        case 0x1B: {
//            CursorCodeCheck(rx_buf);
//        } break;

        default: {
            // todo: User input size constraints would be applied here.
            if (!enqueuec_s(&term->in_buf, toupper(c), false)) UART0_puts("\b");
            if (term->buf_entry < term->in_buf.wr_ptr) {
                term->buf_entry = term->in_buf.wr_ptr;
            }
        } break;
    }
}

bool CommandCheck(char* comm, uint32_t size)
{
    bool valid_command = false;

    char* keyword;
    char* attr_data;

    int i = 0;
    // Find the begin of they query entry
    while (i <= size && comm[i] == ' ') i++;
    keyword = comm + i;

    // Find the end of the query keyword
    while (i < size && comm[i] != ' ') i++;
    comm[i++] = '\0';    // null-terminate the keyword to make decoding easier.

    // Find the begin of the query set data (if it exists)
    while (i < size && comm[i] == ' ') i++;
    attr_data = (i < size) ? (comm + i) : NULL;

    for (i = 0; i < T_COMMAND_SIZE; i++) {
        if (strcmp(keyword, COMMAND[i]) == 0) {
            valid_command = CommHandler[i](attr_data);
        }
    }

    return valid_command;
}

bool SystemView(char* attr)
{
    int i = 0;
    pcb_t* pcb;

    char num_buf[INT_BUF];

    for(i = 0; i < PID_MAX; i++) {
        pcb = GetPCB(i);

        if (pcb->state != UNALLOCATED) {
            UART0_puts("PID: ");
            UART0_puts(itoa((int)pcb->id, num_buf));
            UART0_puts(" |\t state: ");
            UART0_puts(itoa((int)pcb->state, num_buf));
            UART0_puts("\n");
        }
    }

    return true;
}

bool SetProcessFocus(char* attr)
{
    return true;
}


