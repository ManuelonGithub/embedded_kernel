
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

#define TERM_ESC        '\x03'

typedef enum TERMINAL_MODES {
    USER,
    PROCESS_FOCUS,
    PROCESS_STREAM,
    BACKGROUND
} term_mode_t;

#define HOME_HEADER "==="
#define HOME_TEXT   "M'uh Kernel v0.2"

/**
 * @brief   User Command buffer structure.
 * @details It is simply a circular buffer with an extra variable that is used
            to keep track of the length of the entry
            as characters are inputted to the monitor.
            (the write pointer of the circular buffer is the "cursor",
            so it can be moved while there's vald ata in front of it)
 */
typedef struct command_buffer_ {
    circular_buffer_t buffer;
    uint32_t entry_ptr;
} commbuffer_t;

typedef struct terminal_ {
    circular_buffer_t   in_buf;
    uint32_t            buf_entry;

    char        home_line[128];
    char        idle_line[32];
    uint32_t    line_size;

    term_mode_t mode;

    pmbox_t proc_box;
    pmsg_t  proc_msg;
    bool    proc_in;
} terminal_t;

void output_manager();
void terminal();

void init_term(terminal_t* term);

void create_home_line(char* home, uint32_t term_size);
void send_home(char* home);

void user_command_analysis(char c, terminal_t* term);

bool CommandCheck(char* comm, uint32_t size);

bool SystemView(char* attr_str);
bool SetProcessFocus(char* attr_str);

#endif // K_TERMINAL_H



