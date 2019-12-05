
/**
 * @file    k_terminal.c
 * @brief   Contains the terminal process and all its
 *          supporting functionality.
 * @author  Manuel Burnay
 * @date    2019.11.22 (Created)
 * @date    2019.11.29 (Last Modified)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "k_terminal.h"
#include "calls.h"
#include "uart.h"
#include "k_processes.h"
#include "k_scheduler.h"
#include "cstr_utils.h"

enum SUPPORTED_COMMANDS {PS, IO_ON, IO_OFF, RUN, COMMANDS_SIZE};

/**
 * @brief   Supported command keywords. Commands are case insensitive.
 * @details SYSVIEW: displays information about the current processes.
 *          IO: IO permissions command.
 *              Let's user configure or display IO permissions.
 *          RUN: places terminal process on the background
 *               and lets user processes run.
 */
const char* const COMMAND[] = {
    "PS", "IO_ON", "IO_OFF", "RUN"
};

bool (* const CommHandler[])(char*, terminal_t*) = {
    ProcessStatus,
    EnableIO,
    DisableIO,
    run
};

/**
 * @brief   Initializes the terminal settings.
 * @param   [out] term: pointer to terminal structure to initialize.
 */
void init_term(terminal_t* term)
{
    term->mode = COMMAND_HANDLER;

    term->input_entry = 0;
    circular_buffer_init(&term->buf);

    term->box = bind(IO_BOX);

    SetBitRange(term->active_pid, 0, PID_MAX);
    ResetInputCapture(&term->capture);

    // Places in IDLE in high priority so user processes do not run
    ChangeProcessPriority(IDLE_ID, 1);

    GenerateHeader(term->header, 64);
}

/**
 * @brief   Generates the header text displayed when terminal
 *          is running in Command-mode.
 * @param   [out] home:
 *              pointer to string where
 *              generated output will be placed in.
 * @param   [in] width: Width of the header line.
 */
void GenerateHeader(char* home, uint32_t width)
{
    // Find middle and end offsets for the frame and text
    uint8_t mid_off =
            width/2 - strlen(HEADER_TEXT)/2 - strlen(HEADER_FRAME);

    uint8_t end_offset =
            width - (strlen(HEADER_FRAME) +
                    strlen(HEADER_TEXT) + mid_off);

    strcat(home, HEADER_FRAME);

    int i;
    for (i = 0; i < mid_off; i++) {
        strcat(home, " ");
    }

    strcat(home, HEADER_TEXT);

    for (i = 0; i < end_offset; i++) {
        strcat(home, " ");
    }

    strcat(home, HEADER_FRAME);
}

/**
 * @brief   Sends the header line to computer terminal
 */
inline void SendHeader(char* header)
{
    UART0_puts(CURSOR_SAVE);
    UART0_puts(CURSOR_HOME);
    UART0_puts(CLEAR_LINE);
    UART0_puts(HOME_COLOURS);

    UART0_puts(header);

    UART0_puts(CURSOR_RESTORE);
}

/**
 * @brief   Resets the computer terminal settings
 *          and cursor position.
 */
inline void ResetScreen()
{
    UART0_puts(CLEAR_SCREEN);
    UART0_puts(CURSOR_HOME);
    UART0_puts(TERM_COLOURS);
    UART0_puts("\n\n");
    UART0_puts("> ");
}

/**
 * @brief   Resets the terminal settings
 * @param   [out] term:
 *              pointer to terminal structure to reset its contents
 */
inline void ResetTerminal(terminal_t* term)
{
    circular_buffer_init(&term->buf);
    term->input_entry = 0;

    ResetScreen();
    SendHeader(term->header);

    term->mode = COMMAND_HANDLER;

    // Place idle process in high priority
    // so user processes do not run
    ChangeProcessPriority(IDLE_ID, 1);
}

/**
 * @brief   Configures the terminal's input capture settings based
 *          on supplied metadata.
 * @param   [out] cap: pointer to capture settings of the terminal.
 * @param   [in] in:
 *              pointer to metadata structure containing the
 *              input configuration of a process request.
 */
inline void ConfigureInputCapture(input_capture_t* cap, IO_metadata_t* meta, pmbox_t box)
{
    cap->en  = true;
    cap->dst = box;
    cap->max = meta->size;
    cap->pid = meta->proc_id;
}

/**
 * @brief   Resets the terminal's input capture settings.
 * @param   [out] cap: pointer to capture settings of the terminal.
 */
inline void ResetInputCapture(input_capture_t* cap)
{
    cap->en  = false;
    cap->dst = 0;
    cap->max = 0;
    cap->pid = 0;
}

/**
 * @brief   Terminal process.
 * @details Serves as a user command processor
 *          and as the IO server for processes.
 */
void terminal()
{
    terminal_t term;

    init_term(&term);

    ResetScreen();
    SendHeader(term.header);

    uint8_t rx_buf[MSG_MAX_SIZE];

    ChangeProcessPriority(IDLE_ID, 1);

    // Alias pointers.
    // Depending on the src_box value,
    // rx_buf has two different data structures
    IO_metadata_t* IO_meta = (IO_metadata_t*)rx_buf;

    uint8_t* uart_char = (uint8_t*)rx_buf;
    pmbox_t src_box;

    while (1) {
        recv(term.box, ANY_BOX, rx_buf, MSG_MAX_SIZE, &src_box);

        if (src_box == IO_BOX) {
            // Process UART input
            if (*uart_char == TERM_ESC) {
                ResetTerminal(&term);
            }
            else if (term.mode == COMMAND_HANDLER || term.capture.en) {
                ProcessInput(*uart_char, &term);
            }
        }
        else if (GetBit(term.active_pid, IO_meta->proc_id) && !term.capture.en) {
            // Process has IO permissions
            if (IO_meta->is_send) {
                UART0_puts((char*)IO_meta->send_data);
                // Sends data back just so the sender's recv gets the size sent to uart.
                send(src_box, term.box, IO_meta->send_data, IO_meta->size);
            }
            else {
                ConfigureInputCapture(&term.capture, IO_meta, src_box);
            }
        }
        else {
            send(src_box, term.box, "", 0);
        }
    }
}

/**
 * @brief   Processes an input character sent from UART.
 * @param   [in] c: input character.
 * @param   [in,out] term: pointer to active terminal structure.
 * @details Function is used when either in command handler mode or process handler mode.
 *
 */
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

        default: {
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

/**
 * @brief   Sends captured input to process that requested it.
 * @param   [in,out] term: pointer to active terminal structure.
 */
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

/**
 * @brief   Checks terminal's buffer for valid commands
 *          and calls their respective handler functions.
 * @param   [in,out] term: pointer to active terminal structure.
 * @return  True if a valid command was found in the buffer
 *          False if not.
 */
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

    i = 0;
    while(i < COMMANDS_SIZE && strcmp(keyword, COMMAND[i]) != 0)    i++;

    if (i < COMMANDS_SIZE)  valid_command = CommHandler[i](attr_data, term);
    if (!valid_command)     UART0_puts("?\n> ");

    return valid_command;
}

/**
 * @brief   Displays information about the system and allocated processes.
 * @param   [in] attr:
 *              pointer to attribute string associated
 *              with command (not used in this command).
 * @param   [in] term: pointer to active terminal (not used in this command).
 * @returns true.
 */
bool ProcessStatus(char* attr, terminal_t* term)
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
            UART0_puts("Name:       ");
            UART0_puts(pcb->name);
            UART0_puts("\n---- ");
            UART0_puts("State:      ");

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
            UART0_puts("Priority:   ");
            UART0_puts(itoa((int)pcb->priority, num_buf));

            UART0_puts("\n---- ");
            UART0_puts("allowed IO: ");

            if (GetBit(term->active_pid, pcb->id))  UART0_puts("Y");
            else                                    UART0_puts("N");

            UART0_puts("\n");
        }
    }

    UART0_puts("> ");
    return true;
}

/**
 * @brief
 */
bool EnableIO(char* attr, terminal_t* term)
{
    uint32_t start = 0, i = 0, end;
    pid_t pid;
    bitmap_t temp[PID_BITMAP_SIZE];

    ClearBitRange(temp, 0, PID_MAX);

    if (attr == NULL) {
        SetBitRange(temp, 0, PID_MAX);
    }
    else {
        end = strlen(attr);

        while (start < end) {
            while (i < end && attr[i] != ' ')  i++;
            attr[i++] = '\0';

            pid = strtoull(attr+start, NULL, 10);

            if (pid == 0 && attr[start] != '0') return false;
            else {
                SetBit(temp, pid);
            }

            start = i;
        }
    }

    for(i = 0; i < PID_BITMAP_SIZE; i++) {
        term->active_pid[i] |= temp[i];
    }

    UART0_puts("> ");
    return true;
}

bool DisableIO(char* attr, terminal_t* term)
{
    uint32_t start = 0, i = 0, end;
    pid_t pid;
    bitmap_t temp[PID_BITMAP_SIZE];

    ClearBitRange(temp, 0, PID_MAX);

    if (attr == NULL) {
        SetBitRange(temp, 0, PID_MAX);
    }
    else {
        end = strlen(attr);

        while (start < end) {
            while (i < end && attr[i] != ' ')  i++;
            attr[i++] = '\0';

            pid = strtoull(attr+start, NULL, 10);

            if (pid == 0 && attr[start] != '0') return false;
            else {
                SetBit(temp, pid);
            }

            start = i;
        }
    }

    for(i = 0; i < PID_BITMAP_SIZE; i++) {
        term->active_pid[i] &= ~temp[i];
    }

    UART0_puts("> ");
    return true;
}

bool run(char* attr, terminal_t* term)
{
    term->mode = PROCESS_HANDLER;
    ChangeProcessPriority(IDLE_ID, IDLE_LEVEL);
    UART0_puts(CLEAR_SCREEN);
    UART0_puts(CURSOR_HOME);

    return true;
}


