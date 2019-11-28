
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "k_terminal.h"
#include "calls.h"
#include "uart.h"
//#include "k_messaging.h"
#include "k_processes.h"
#include "k_scheduler.h"
#include "bitmap.h"
#include "cstr_utils.h"

enum TERMINAL_COMMANDS {SYSTEM_VIEW, SET_PROCESS_FOCUS, T_COMMAND_SIZE};

const char* const COMMAND[] =
{
    "SYS", "SETP"
};

bool (* const CommHandler[])(char*, terminal_t*) =
{
    SystemView,
    SetProcessFocus
};

extern active_IO_t IO;

void init_term(terminal_t* term)
{
    term->mode = COMMAND_HANDLER;

    ClearBitRange(IO.active_pid, 0, PID_MAX);

    term->buf_entry = 0;
    circular_buffer_init(&term->in_buf);

    create_home_line(term->home_line, 64);
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

                ClearBitRange(IO.active_pid, 0, PID_MAX);

                term.mode = COMMAND_HANDLER;
            }
            else if (term.mode == COMMAND_HANDLER || IO.in_proc != 0){
                TerminalInputHandler(in_char, &term);
            }
        }
        else {
//            UART0_puts(CURSOR_SAVE);
//            UART0_puts(BLINK_TEXT);
//            UART0_puts("_");
//            UART0_puts(CURSOR_RESTORE);
            if (term.mode != COMMAND_HANDLER){
                nice(LOWEST_PRIORITY);
            }
        }
    }
}

void TerminalInputHandler(char c, terminal_t* term)
{
    UART0_put(&c, 1);

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

        case '\0':
            UART0_puts("\n");
        case '\r':
        case '\n': {
            if (term->mode == COMMAND_HANDLER) {
                CommandCheck(term);
            }
            else {
                SendUserInput(term);
            }

            term->buf_entry = 0;
            term->in_buf.wr_ptr = 0;
        } break;

//        case 0x1B: {
//            CursorCodeCheck(rx_buf);
//        } break;

        default: {
            // todo: User input size constraints would be applied here.
            if (term->mode == COMMAND_HANDLER) {
                if (!enqueuec_s(&term->in_buf, toupper(c), false)) {
                    UART0_puts("\b");
                }
            }
            else {
                if (!enqueuec_s(&term->in_buf, c, false)) {
                    UART0_puts("\b");
                }
            }

            if (term->buf_entry < term->in_buf.wr_ptr) {
                term->buf_entry = term->in_buf.wr_ptr;
            }
        } break;
    }
}

void SendUserInput(terminal_t* term)
{
    if (term->in_buf.wr_ptr > IO.inbuf_max) {
        *IO.ret_size = IO.inbuf_max-1;
    }
    else {
        *IO.ret_size = term->in_buf.wr_ptr;
    }

    memcpy(IO.proc_inbuf, term->in_buf.data, (*IO.ret_size));
    IO.proc_inbuf[(*IO.ret_size)] = '\0';

    pcb_t* proc = GetPCB(IO.in_proc);
    LinkPCB(proc, proc->priority);

    IO.in_proc = 0;
    IO.output_off = false;
}

bool CommandCheck(terminal_t* term)
{
    bool valid_command = false;

    char* comm = term->in_buf.data;
    uint32_t size = term->in_buf.wr_ptr;

    char* keyword;
    char* attr_data;

    if (!enqueuec_s(&term->in_buf, '\0', false)) {
        term->in_buf.data[term->in_buf.wr_ptr] = '\0';
    }

    int i = 0;
    // Find the begin of they query entry
    while (i < size && comm[i] == ' ') i++;
    keyword = comm + i;

    // Find the end of the query keyword
    while (i < size && comm[i] != ' ') i++;
    comm[i++] = '\0';    // null-terminate the keyword to make decoding easier.

    // Find the begin of the query set data (if it exists)
    while (i < size && comm[i] == ' ') i++;
    attr_data = (i < size) ? (comm + i) : NULL;

    for (i = 0; i < T_COMMAND_SIZE; i++) {
        if (strcmp(keyword, COMMAND[i]) == 0) {
            valid_command = CommHandler[i](attr_data, term);
        }
    }

    return valid_command;
}

bool SystemView(char* attr, terminal_t* term)
{
    pcb_t* pcb;

    char num_buf[INT_BUF];

    int i;
    for(i = 0; i < PID_MAX; i++) {
        pcb = GetPCB((pid_t)i);

        if (pcb->state != UNASSIGNED) {
            UART0_puts("PID: ");
            UART0_puts(itoa((int)pcb->id, num_buf));
            UART0_puts("\n---- ");
            UART0_puts("Name:  ");
            UART0_puts(pcb->name);
            UART0_puts("\n---- ");
            UART0_puts("State: ");

            switch (pcb->state) {
                case WAITING_TO_RUN: UART0_puts("Waiting to run");
                    break;
                case RUNNING: UART0_puts("Running");
                    break;
                case BLOCKED: UART0_puts("Blocked");
                    break;
                case TERMINATED: UART0_puts("Terminated");
                    break;
            }

            UART0_puts("\n---- ");
            UART0_puts("Priority: ");
            UART0_puts(itoa((int)pcb->priority, num_buf));

            UART0_puts("\n");
        }
    }

    UART0_puts("> ");
    return true;
}

bool SetProcessFocus(char* attr, terminal_t* term)
{
    uint32_t start = 0, i = 0, end;
    pid_t pid;

    if (attr == NULL) {
        SetBitRange(IO.active_pid, 0, PID_MAX);
    }
    else {
        end = strlen(attr);

        while (start < end) {
            while (i < end && attr[i] != ' ')  i++;
            attr[i++] = '\0';

            pid = strtoull(attr+start, NULL, 10);

            if (pid == 0 && attr[start] != '0') return false;
            else {
                SetBit(IO.active_pid, pid);
            }

            start = i;
        }
    }

    term->mode = PROCESS_INPUT_CAPTURE;
    UART0_puts(CLEAR_SCREEN);
    UART0_puts(CURSOR_HOME);
    return true;
}


