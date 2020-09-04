#include "terminal.h"

/*
 * terminal_init
 * Description: Initializes the terminal
 * Input: none
 * Output: none
 * Side effects: Initializes the terminal and creates a new one
 * Return: none
 */
void terminal_init() {
    int idx;
    int row;
    //clear the history_buffer
    for (row = 0; row < NUM_ROWS - STATUS_BAR_HEIGHT; row++)
        for (idx = 0; idx < NUM_COLS; idx++)
            three_terminals[current_term].history_buffer[row][idx] = '\0';
    //clear the live_buffer
    for (idx = 0; idx < SCREEN_SIZE; idx++) {
        three_terminals[current_term].live_buffer[idx] = '\0';
        three_terminals[current_term].live_buffer_store[idx] = '\0';
    }
    //set shared variables to zero
    three_terminals[current_term].bytes_read = 0;
    three_terminals[current_term].bytes_read_store = 0;
    three_terminals[current_term].cursor_loc = 0;
    three_terminals[current_term].screen_x = 0;
    three_terminals[current_term].screen_y = 0;
    three_terminals[current_term].row_index = 0;
    three_terminals[current_term].bytes_written = 0;
    three_terminals[current_term].read_in_progress = 0;

    // clear screen
    clear_multi((uint8_t * ) virtual_terminal_addresses[current_term], current_term);
}

/*
 * terminal_open
 * Description: Open the virtual console file
 * Input: file
 * Output: None
 * Side effects: clears screen
 * Return: 0 if terminal opens
 */
int32_t terminal_open(const unsigned char * file) {
    terminal_init();
    return 0;
}

/*
 * terminal_close
 * Description: closes the terminal
 * Input: None
 * Output: None
 * Side effects: closes the terminal
 * Return: 0 on success
 */
int32_t terminal_close(int fd) {
    int idx;
    int row;
    //clear history buffer
    for (row = 0; row < NUM_ROWS - STATUS_BAR_HEIGHT; row++)
        for (idx = 0; idx < NUM_COLS; idx++)
            three_terminals[current_term].history_buffer[row][idx] = '\0';
    //clear the screen
    clear_multi((uint8_t * ) virtual_terminal_addresses[current_term], current_term);
    //clear all shared vaiables
    three_terminals[current_term].row_index = 0;
    three_terminals[current_term].bytes_written = 0;
    three_terminals[current_term].bytes_read = 0;
    three_terminals[current_term].bytes_read_store = 0;
    for (idx = 0; idx < CHAR_BUFFER_SIZE; idx++) {
        three_terminals[current_term].live_buffer[idx] = '\0';
        three_terminals[current_term].live_buffer_store[idx] = '\0';
    }
    return 0;
}

/*
 * terminal_read
 * Description: Reads the characters from keyboard and sends to write
 * Input: index, offset, buffer, and amount of characters to write to screen
 * Output: writes buffer globally saved buffer
 * Side effects: Updates the buffer to be written
 * Return: -1 on failure, 0 on success
 */
int32_t terminal_read(int fd, void * char_buffer, int bytes) {

    int idx;
    int buf_idx;
    int local_bytes_written;
    int command_loc;
    int length = 0;
    char * ptr;
    local_bytes_written = 0;

    uint32_t addr;
    uint32_t terminal_idx;

    if (current_term == all_process[current_process_pid].active_terminal_idx) {
        addr = VISUAL_VIRTUAL_ADDR;
        terminal_idx = current_term;
    } else {
        terminal_idx = all_process[current_process_pid].active_terminal_idx;
        addr = virtual_terminal_addresses[terminal_idx];
    }

    three_terminals[terminal_idx].read_in_progress = 1;

    ptr = (char * ) char_buffer;
    if (ptr == NULL || bytes < 0)
        return -1;
    cli();
    if (three_terminals[terminal_idx].bytes_written != 0) {
        local_bytes_written = three_terminals[terminal_idx].bytes_written;
    }
    sti();
    // while(three_terminals[current_term].live_buffer_store[three_terminals[current_term].bytes_read_store] != '\n' || current_term != current_multiprocess) {
    while (three_terminals[terminal_idx].live_buffer_store[three_terminals[terminal_idx].bytes_read_store] != '\n') {
        //wait until a newline character has been pressed
    }
    cli();
    buf_idx = 0;

    if (three_terminals[terminal_idx].write_command_row >= MAX_COMMAND_STORE){
        command_buffer_scroll(terminal_idx);
    }
    command_loc = three_terminals[terminal_idx].write_command_row;

    for (idx = local_bytes_written; idx < three_terminals[terminal_idx].bytes_read_store; idx++) { //copy data from keyboard buffer into char_buffer
        if (buf_idx < three_terminals[terminal_idx].bytes_read_store) {
            ptr[buf_idx] = three_terminals[terminal_idx].live_buffer_store[idx];
            if (three_terminals[terminal_idx].bytes_read_store > local_bytes_written){
                three_terminals[terminal_idx].command_buffer[command_loc][buf_idx] = ptr[buf_idx];
            }
            length += 1;
        }
        buf_idx++;
    }
    if (three_terminals[terminal_idx].bytes_read_store > local_bytes_written){
        three_terminals[terminal_idx].write_command_row++;
        three_terminals[terminal_idx].view_command_row = three_terminals[terminal_idx].write_command_row;
    }
    ptr[buf_idx] = '\n';
    length += 1;
    for (idx = 0; idx < three_terminals[terminal_idx].bytes_read_store; idx++) { //set live_buffer_storage to null
        three_terminals[terminal_idx].live_buffer_store[idx] = '\0';
    }
    three_terminals[terminal_idx].bytes_read_store = 0; //set keyboard buffer index to null

    three_terminals[terminal_idx].read_in_progress = 0;

    sti();
    return length; //return the length
}

/*
 * write_parser()
 * Description: Helper function for writing to the screen
 * Input: None
 * Output: None
 * Side effects: Updates the history buffer and writes to screen
 * Return: None
 */
//parser_buffer[0] to character
//parser_buffer[1] to ctrl
//parser_buffer[2] backspace
//parser_buffer[3] tab
int write_parser(unsigned char * parser_buffer) {

    //Checking if input is null
    if (parser_buffer == NULL) {
        return 0;
    }

    uint32_t addr;
    uint32_t terminal_idx;
    if (current_term == all_process[current_process_pid].active_terminal_idx) {
        addr = VISUAL_VIRTUAL_ADDR;
        terminal_idx = current_term;
    } else {
        terminal_idx = all_process[current_process_pid].active_terminal_idx;
        addr = virtual_terminal_addresses[terminal_idx];
    }

    //If column overflow then goes to next line
    if (three_terminals[terminal_idx].bytes_written >= NUM_COLS) {
        three_terminals[terminal_idx].row_index += 1;
        three_terminals[terminal_idx].bytes_written = 0;
    }
    //If row overflow then scrolls up
    if (three_terminals[terminal_idx].row_index >= NUM_ROWS - STATUS_BAR_HEIGHT) {
        terminal_scroll();
    }

    //Updates history buffer with new written text
    three_terminals[terminal_idx].history_buffer[three_terminals[terminal_idx].row_index][three_terminals[terminal_idx].bytes_written] = parser_buffer[0];
    //Prints the text
    putc_multi(parser_buffer[0], (uint8_t * ) addr, terminal_idx);

    return 0;
}

/*
 * terminal_write
 * Description: Writes characters from buffer to terminal
 * Input: buffer and amount of characters to write to screen
 * Output: writes buffer to video memory
 * Side effects: cursor updates and outputs characters to terminal
 * Return: -1 on failure, 0 on success
 */
int32_t terminal_write(int fd,
    const void * char_buffer, int n) {

    int idx;
    int buf_idx;
    uint8_t terminal_idx = all_process[current_process_pid].active_terminal_idx;
    unsigned char buffer_to_parse[4];
    char * ptr = (char * ) char_buffer;

    //Validating input parameters
    if (ptr == NULL || n < 0)
        return -1;

    //Setting up buffer to send to write helper
    buffer_to_parse[0] = '\0'; //Default set to null character
    buffer_to_parse[1] = '0'; //Ctrl is not pressed so default to 0
    buffer_to_parse[2] = '0'; //Backspace is not pressed so default to 0
    buffer_to_parse[3] = '0'; //Tab is not pressed so default to 0

    cli();
    //Updating writing position
    for (idx = 0; idx < n; idx++) {
        //Writing char
        buffer_to_parse[0] = ptr[idx];
        write_parser(buffer_to_parse);
        //0 Because checking if first char on new line
        if (three_terminals[terminal_idx].bytes_written % NUM_COLS == 0) {
            for (buf_idx = 0; buf_idx < CHAR_BUFFER_SIZE; buf_idx++) {
                three_terminals[terminal_idx].live_buffer[buf_idx] = '\0';
            }
            three_terminals[terminal_idx].live_buffer[0] = ptr[idx];
        } else {
            three_terminals[terminal_idx].live_buffer[three_terminals[terminal_idx].bytes_written] = ptr[idx];
        }
        //If enter is pressed then we want to skip to new line
        if (buffer_to_parse[0] == '\n') {
            three_terminals[terminal_idx].bytes_written = 0;
            //1 is Added because we want to go to new line
            three_terminals[terminal_idx].cursor_loc = ((three_terminals[terminal_idx].cursor_loc / NUM_COLS) + 1) * NUM_COLS;
            three_terminals[terminal_idx].row_index++;
            for (buf_idx = 0; buf_idx < CHAR_BUFFER_SIZE; buf_idx++) {
                three_terminals[terminal_idx].live_buffer[buf_idx] = '\0';
            }
        } else {
            three_terminals[terminal_idx].bytes_written++;
            three_terminals[terminal_idx].cursor_loc += 1;
        }
    }

    //Adding new line character at the end and updating variables
    three_terminals[terminal_idx].live_buffer_store[0] = '\0';
    three_terminals[terminal_idx].bytes_read_store = 0;
    three_terminals[terminal_idx].bytes_read = (three_terminals[terminal_idx].bytes_written % NUM_COLS);
    three_terminals[terminal_idx].bytes_written = (three_terminals[terminal_idx].bytes_written % NUM_COLS);
    three_terminals[terminal_idx].live_buffer_location = three_terminals[terminal_idx].bytes_read;

    // only move cursor if we are looking at the right screen
    if (current_term == terminal_idx) {
        move_cursor(three_terminals[terminal_idx].cursor_loc);
    }
    sti();
    return n;
}
