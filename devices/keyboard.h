#include "../lib.h"
#include "../i8259.h"
#include "terminal_structs.h"
#include "../paging/multi_terminals.h"


#define REPEAT_INTERVAL 30
#define REPEAT_DELAY 250

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define KEYBOARD_IRQ 1

#define LCTRL 29
#define LSHIFT 42
#define RSHIFT 54
#define ALT 56
#define CAPS_LOCK 58
#define BACKSPACE 14
#define ENTER 28
#define TAB 15
#define ALT_F1 0x68
#define ALT_F2 0x69
#define ALT_F3 0x6A
#define KEY_UP 0x48
#define KEY_DOWN 0x50
#define KEY_RIGHT 0x4D
#define KEY_LEFT 0x4B

#define ENTER_SPAM_RATE 1

#define MIN_ALPHABET 97
#define MAX_ALPHABET 122
#define MAX_ASCII 128

#define RELEASE_OFFSET 0x80
#define ALT_OFFSET 57

#define CHAR_BUFFER_SIZE 128

char live_buffer[CHAR_BUFFER_SIZE];

char live_buffer_store[CHAR_BUFFER_SIZE];

int bytes_read;
int bytes_read_store;
int bytes_written;
char history_buffer[NUM_ROWS][NUM_COLS];
int row_index;
int cursor_loc;

#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

/* Initialize keyboard */
void keyboard_init(void);

/* entry for irq handler */
void keyboard_handler(void);

int read_parser(unsigned char *parser_buffer);

void move_cursor(int cursor_loc);

void enable_cursor();

//switch the terminal
void terminal_switch(int new_term, uint8_t all_terms_initialized);
//clear the buffer
void buffer_clear();
//scrolls the terminal
void terminal_scroll();
void terminal_scrollv2();
void command_buffer_scroll(int terminal_idx);
void prev_command();
void next_command();
void cursor_right();
void cursor_left();
//redraws the terminal
void terminal_redraw(int scrolled);
//Helper Function for the read function
int read_parser(unsigned char *parser_buffer);
//Helper function for the write function
int write_parser(unsigned char *parser_buffer);
//a function that creates a new line on screen when enter key is pressed
void new_line();
//creates backspace functionality
void backspace();
//creates a tab
void tab(unsigned char *parser_buffer);
//outputs character to screen
void output_char(unsigned char* parser_buffer);

#endif
