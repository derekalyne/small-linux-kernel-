#include "../lib.h"

#include "../i8259.h"

#ifndef __TERMINAL_STRUCT_H
#define __TERMINAL_STRUCT_H

#define CHAR_BUFFER_SIZE 128

#define NUM_COLS 80
#define NUM_ROWS 25
#define SCREEN_SIZE 2000
#define MAX_COMMAND_STORE 20

#define TERMINAL_1 0
#define TERMINAL_2 1
#define TERMINAL_3 2
#define NUM_TERMINALS 3

// var for current displayed terminal
int current_term;
// struct for terminal
typedef struct terminal {
    char live_buffer[SCREEN_SIZE];
    char live_buffer_store[SCREEN_SIZE];
    char temp_commands[SCREEN_SIZE];
    char command_buffer[MAX_COMMAND_STORE][CHAR_BUFFER_SIZE];

    int bytes_read;
    int bytes_read_store;
    int bytes_written;
    int temp_bytes_read;
    int live_buffer_location;

    char history_buffer[NUM_ROWS - STATUS_BAR_HEIGHT][NUM_COLS];
    int row_index;
    int write_command_row;
    int view_command_row;
    int cursor_loc;

    int screen_x;
    int screen_y;

    uint8_t read_in_progress;
} terminal_t;

// array containing the 3 terminal structs
terminal_t three_terminals[NUM_TERMINALS];


#endif
