
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

void init_term(terminal_t* term)
{
    term->mode = COMMAND_HANDLER;

    term->input_entry = 0;
    circular_buffer_init(&term->buf);

    term->box = bind(IO_BOX);

    ClearBitRange(term->active_pid, 0, PID_MAX);
    ResetInputCapture(&term->capture);

    // Places in IDLE in high priority so user processes do not run
    ChangeProcessPriority(IDLE_ID, 1);

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

inline void send_home(char* home)
{
    UART0_puts(CURSOR_SAVE);
    UART0_puts(CURSOR_HOME);
    UART0_puts(CLEAR_LINE);
    UART0_puts(HOME_COLOURS);

    UART0_puts(home);

    UART0_puts(CURSOR_RESTORE);
}

inline void ResetScreen()
{
    UART0_puts(CLEAR_SCREEN);
    UART0_puts(CURSOR_HOME);
    UART0_puts(TERM_COLOURS);
    UART0_puts("\n\n");
    UART0_puts("> ");
}

inline void ResetTerminal(terminal_t* term)
{
    circular_buffer_init(&term->buf);
    term->input_entry = 0;

    ResetScreen();
    send_home(term->home_line);

    term->mode = COMMAND_HANDLER;

    // Place idle process in high priority so user processes do not run
    ChangeProcessPriority(IDLE_ID, 1);
}

inline void ConfigureInputCapture(input_capture_t* cap, IO_metadata_t* meta)
{
    cap->en  = true;
    cap->dst = meta->box_id;
    cap->max = meta->size;
    cap->pid = meta->proc_id;
}

inline void ResetInputCapture(input_capture_t* cap)
{
    cap->en  = false;
    cap->dst = 0;
    cap->max = 0;
    cap->pid = 0;
}

void terminal()
{
    terminal_t term;

    init_term(&term);

    ResetScreen();
    send_home(term.home_line);

    uint8_t rx_buf[MSG_MAX_SIZE];

    ChangeProcessPriority(IDLE_ID, 1);

    // Alias pointers.
    // Depending on the src_box value, rx_buf has two different data structures
    IO_metadata_t* IO_meta = (IO_metadata_t*)rx_buf;
    uart_msgdata_t* uart = (uart_msgdata_t*)rx_buf;
    pmbox_t* src_box = (pmbox_t*)&rx_buf; // The first four bytes in array should be the source box

    while (1) {
        recv(term.box, ANY_BOX, rx_buf, MSG_MAX_SIZE);

        if (*src_box == IO_BOX) {
            // Process UART input
            if (uart->c == TERM_ESC) {
                ResetTerminal(&term);
            }
            else if (term.mode == COMMAND_HANDLER || term.capture.en) {
                ProcessInput(uart->c, &term);
            }
        }
        else if (GetBit(term.active_pid, IO_meta->proc_id) && !term.capture.en) {
            if (IO_meta->is_send) {
                UART0_puts((char*)IO_meta->send_data);
                // Sends data back just so the sender's recv gets the size sent to uart.
                send(*src_box, term.box, IO_meta->send_data, IO_meta->size);
            }
            else {
                ConfigureInputCapture(&term.capture, IO_meta);
            }
        }
        else {
            send(*src_box, term.box, "", 0);
        }
    }
}

void ProcessInput(char c, terminal_t* term)
{
    UART0_put(&c, 1);

    switch (c) {
        case '\b':
        case 0x7F: {
            if (term->buf.wr_ptr > 0) {
                term->buf.wr_ptr--;
                term->input_entry--;
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

            term->input_entry = 0;
            term->buf.wr_ptr = 0;
        } break;

//        case 0x1B: {
//            CursorCodeCheck(rx_buf);
//        } break;

        default: {
            // todo: User input size constraints would be applied here.
            if (term->mode == COMMAND_HANDLER)  c = toupper(c);

            if (!enqueuec_s(&term->buf, c, false)) {
                UART0_puts("\b");
            }

            if (term->input_entry < term->buf.wr_ptr) {
                term->input_entry = term->buf.wr_ptr;
            }
        } break;
    }
}

void SendUserInput(terminal_t* term)
{
    size_t size;
    if (term->buf.wr_ptr > term->capture.max) {
        size = term->capture.max;
        term->buf.data[size-1] = '\0';
    }
    else {
        size = term->buf.wr_ptr+1;
    }

    send(term->capture.dst, term->box, (uint8_t*)term->buf.data, size);

    ResetInputCapture(&term->capture);
}

bool CommandCheck(terminal_t* term)
{
    bool valid_command = false;

    char* comm = term->buf.data;
    uint32_t size = term->buf.wr_ptr;

    char* keyword;
    char* attr_data;

    if (!enqueuec_s(&term->buf, '\0', false)) {
        term->buf.data[term->buf.wr_ptr] = '\0';
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

    if (!valid_command) UART0_puts("?\n> ");

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
        SetBitRange(term->active_pid, 0, PID_MAX);
    }
    else {
        end = strlen(attr);

        while (start < end) {
            while (i < end && attr[i] != ' ')  i++;
            attr[i++] = '\0';

            pid = strtoull(attr+start, NULL, 10);

            if (pid == 0 && attr[start] != '0') return false;
            else {
                SetBit(term->active_pid, pid);
            }

            start = i;
        }
    }

    term->mode = PROCESS_HANDLER;

    ChangeProcessPriority(IDLE_ID, IDLE_LEVEL);
    UART0_puts(CLEAR_SCREEN);
    UART0_puts(CURSOR_HOME);
    return true;
}


