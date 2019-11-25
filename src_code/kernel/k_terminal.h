
#ifndef K_TERMINAL_H
#define K_TERMINAL_H

typedef enum TERMINAL_MODES {
    USER,
    PROCESS_FOCUS,
    PROCESS_STREAM,
    BACKGROUND
} term_modes_t;

// struct home_line {
// 	char 	cursor_save[sizeof(CURSOR_SAVE)];
// 	char 	cursor_home[sizeof(CURSOR_HOME)];
// 	char* 	idle_data;
// 	char 	cursor_middle[sizeof(CURSOR_MIDDLE)];
// 	char* 	middle_data;
// 	char 	cursor_restore[sizeof(CURSOR_RESTORE)];
// };

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

#endif // K_TERMINAL_H



