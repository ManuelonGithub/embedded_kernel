
#ifndef K_TERMINAL_H
#define K_TERMINAL_H


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
} term_modes_t;

#define IDLE_SIZE   16
#define MID_SIZE    32
#define BUF_SIZE    40

typedef struct home_data_ {
    char idle[IDLE_SIZE];
    char middle[MID_SIZE];
    char mid_buf[BUF_SIZE];
    char end_buf[BUF_SIZE];
} home_data_t;

void init_term(home_data_t* home);

void send_home(home_data_t* home);

void output_manager();

void terminal();

#endif // K_TERMINAL_H



