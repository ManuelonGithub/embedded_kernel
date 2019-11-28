
#ifndef K_TERMINAL_H
#define K_TERMINAL_H

#include <stdbool.h>
#include "cirbuffer.h"
#include "k_types.h"

#define CLEAR_SCREEN    "\x1b[2J"
#define CURSOR_SAVE     "\x1b""7"
#define CURSOR_HOME     "\x1b[H"
#define CLEAR_LINE      "\x1b[2K"
#define HOME_COLOURS    "\x1b[0;30;47m"
#define CURSOR_MIDDLE   "\x1b[20C"
#define TERM_COLOURS    "\x1b[0;0;0m"
#define CURSOR_RESTORE  "\x1b""8"

#define CURSOR_LEFT     "\x1b[D"
#define CURSOR_RIGHT    "\x1b[C"
#define CURSOR_UP       "\x1b[A"
#define CURSOR_DOWN     "\x1b[B"

#define BLINK_TEXT      "\x1b[5m"
#define TERM_ESC        '\x03'

typedef enum TERMINAL_MODES {
    COMMAND_HANDLER,
    PROCESS_INPUT_CAPTURE
} term_mode_t;

#define HOME_HEADER "==="
#define HOME_TEXT   "M'uh Kernel v0.3"

typedef struct active_IO_ {
    bitmap_t    active_pid[PID_BITMAP_SIZE];
    bool        output_off;
    char*       proc_inbuf;
    uint32_t    inbuf_max;
    pid_t       in_proc;
    size_t*     ret_size;
} active_IO_t;

typedef struct terminal_ {
    circular_buffer_t   in_buf;
    uint32_t            buf_entry;
    char        home_line[128];
    term_mode_t mode;
} terminal_t;

void output_manager();
void terminal();

void init_term(terminal_t* term);

void create_home_line(char* home, uint32_t term_size);
void send_home(char* home);

void TerminalInputHandler(char c, terminal_t* term);

void SendUserInput(terminal_t* term);

bool CommandCheck(terminal_t* term);

bool SystemView(char* attr_str, terminal_t* term);
bool SetProcessFocus(char* attr_str, terminal_t* term);

#endif // K_TERMINAL_H



