
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
    PROCESS_HANDLER
} term_mode_t;

#define HOME_HEADER "==="
#define HOME_TEXT   "M'uh Kernel v0.3"

typedef struct input_capture_ {
    bool                en;
    size_t              max;
    pid_t               pid;
    pmbox_t             dst;
} input_capture_t;

typedef struct terminal_ {
    circular_buffer_t   buf;
    uint32_t            input_entry;
    char                home_line[128];
    term_mode_t         mode;
    pmbox_t             box;
    bitmap_t            active_pid[PID_BITMAP_SIZE];
    input_capture_t     capture;
} terminal_t;

void output_manager();
void terminal();

void init_term(terminal_t* term);

void create_home_line(char* home, uint32_t term_size);
inline void send_home(char* home);

inline void ResetScreen();
inline void ResetTerminal(terminal_t* term);

inline void ConfigureInputCapture(input_capture_t* cap, IO_metadata_t* meta);
inline void ResetInputCapture(input_capture_t* cap);

void ProcessInput(char c, terminal_t* term);

void SendUserInput(terminal_t* term);

bool CommandCheck(terminal_t* term);

bool SystemView(char* attr_str, terminal_t* term);
bool SetProcessFocus(char* attr_str, terminal_t* term);

#endif // K_TERMINAL_H



