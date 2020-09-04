#include "keyboard.h"

// global variables
uint32_t flags;
uint8_t shift_pressed;
uint8_t ctrl_pressed;
uint8_t caps_pressed;
uint8_t key_pressed;
uint8_t alt_pressed;
uint8_t enter_pressed;
uint8_t current_key;
uint8_t handled_enter;
uint8_t spam_enters_received;
unsigned char parse_buffer[4]; //used for parseing keyboard

/* http://www.scs.stanford.edu/10wi-cs140/pintos/specs/kbd/scancodes-1.html
 * https://ideone.com/y1zoJq
 */
int8_t printable_scan_codes[] = {
    '\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\0', '\0', 'q', 'w', 'e', 'r',
    't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\0', '\0', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
    '\'', '`', '\0', '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', '\0', '\0', '\0', ' ', '\0',
    '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\0', '\0', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U',
    'I', 'O', 'P', '{', '}', '\0', '\0', 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', '\0',
    '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', '\0', '\0', '\0', ' '
};

/* void keyboard_init(void);
 * Inputs: None
 * Return Value: None
 * Function: Initialize keyboard */
void keyboard_init(void) {

    // enable IRQ1
    enable_irq(KEYBOARD_IRQ);
    enable_cursor();

    // init global vars
    shift_pressed = 0;
    ctrl_pressed = 0;
    caps_pressed = 0;
    key_pressed = 0;
    current_key = 0;
    enter_pressed = 0;
    spam_enters_received = 0;
    handled_enter = 1;
}

/* void keyboard_handler(void);
 * Inputs: None
 * Return Value: None
 * Function: entry for keyboard handler */
void keyboard_handler(void) {

    uint8_t status;
    uint8_t keycode;
    uint8_t printable_char;
    uint8_t idx_offset;

    printable_char = '\0';
    idx_offset = 0;

    // read status of keyboard
    status = inb(KEYBOARD_STATUS_PORT);
    if (status & 0x01) {
        // get keycode value
        keycode = inb(KEYBOARD_DATA_PORT);

        // set conditions for special key combos
        if (keycode == LCTRL) {
            ctrl_pressed = 1;
        } else if (keycode == LCTRL + RELEASE_OFFSET) {
            ctrl_pressed = 0;
        } else if (keycode == LSHIFT || keycode == RSHIFT) {
            shift_pressed = 1;
        } else if (keycode == (LSHIFT + RELEASE_OFFSET) || keycode == (RSHIFT + RELEASE_OFFSET)) {
            shift_pressed = 0;
        } else if (keycode == CAPS_LOCK) {
            // toggle value instead of setting it
            caps_pressed = 1 - caps_pressed;
        } else if (keycode == ALT) {
            alt_pressed = 1;
        } else if (keycode == ALT + RELEASE_OFFSET) {
            alt_pressed = 0;
        } else if (keycode == ENTER) { //enter has been pressed
            // when enter is held, we take every other interrupt
            if (!enter_pressed || (enter_pressed && spam_enters_received > ENTER_SPAM_RATE)) {
                // setup condition codes
                parse_buffer[0] = '\n';
                parse_buffer[1] = '0';
                parse_buffer[2] = '1';
                parse_buffer[3] = '0';
                enter_pressed = 1;
                //[1] and [2] are zero;
                read_parser(parse_buffer);
                spam_enters_received = 0;
            }
            if (enter_pressed)
                spam_enters_received++;
            enter_pressed = 1;
        } else if (keycode == ENTER + RELEASE_OFFSET) {
            enter_pressed = 0;
        } else if (keycode == BACKSPACE) { //backspace key has been pressed
            // setup condition codes
            parse_buffer[0] = '0';
            parse_buffer[1] = '0';
            parse_buffer[2] = '1';
            parse_buffer[3] = '0';
            read_parser(parse_buffer);
        } else if (keycode == TAB) { //tab character has been pressed
            // setup condition codes
            parse_buffer[0] = ' ';
            parse_buffer[1] = '0';
            parse_buffer[2] = '0';
            parse_buffer[3] = '1';
            read_parser(parse_buffer);
        } else if (keycode == KEY_UP) {
            prev_command();
        } else if(keycode == KEY_DOWN) {
            next_command();
        } else if(keycode == KEY_LEFT) {
            cursor_left();
        } else if (keycode == KEY_RIGHT) {
            cursor_right();
        } else {
            if (keycode < MAX_ASCII) {
                key_pressed = 1;
                // set offset for printing alt keys
                if ((!shift_pressed && caps_pressed) || (shift_pressed && !caps_pressed))
                    idx_offset = ALT_OFFSET;

                // only enable caps lock for letters
                if ((printable_scan_codes[keycode] < MIN_ALPHABET || printable_scan_codes[keycode] > MAX_ALPHABET) && caps_pressed) {
                    if (shift_pressed)
                        idx_offset = ALT_OFFSET;
                    else
                        idx_offset = 0;
                }

                // get the printable char
                printable_char = printable_scan_codes[keycode + idx_offset];
            } else {
                // let go of key
                key_pressed = 0;
            }
        }
        if (printable_char == '!' && shift_pressed != 1 && alt_pressed) {
            // switch to term 1 on F1, other flag is for all terminals inited
            terminal_switch(TERMINAL_1, 1);
        } else if (printable_char == '@' && shift_pressed != 1 && alt_pressed) {
            // switch to term 2 on F2, other flag is for all terminals inited
            terminal_switch(TERMINAL_2, 1);
        } else if (printable_char == '#' && shift_pressed != 1 && alt_pressed) {
            // switch to term 3 on F3, other flag is for all terminals inited
            terminal_switch(TERMINAL_3, 1);
        }
        // if char is valid, print it
        else if (printable_char > 0 && !ctrl_pressed) { //normal character has been pressed
            // setup condition codes
            parse_buffer[0] = printable_char;
            parse_buffer[1] = '0';
            parse_buffer[2] = '0';
            parse_buffer[3] = '0';
            read_parser(parse_buffer);
        } else if (ctrl_pressed && (printable_char == 'l' || printable_char == 'L')) { //CTRL-L has been pressed
            // setup condition codes
            parse_buffer[0] = printable_char;
            parse_buffer[1] = '1';
            parse_buffer[2] = '0';
            parse_buffer[3] = '0';
            read_parser(parse_buffer);
        }

    }
    // send eoi
    send_eoi(KEYBOARD_IRQ);
}
/* void move_cursor(int cursor_loc);
 * Inputs: cursor_loc which is the location of the cursor
 * Return Value: None
 * Function: Updates and displays the location of the cursor */
void move_cursor(int cursor_loc) {
    int cursor_row, cursor_col;

    //Making sure cursor is not negative
    if (three_terminals[current_term].cursor_loc <= 0) {
        three_terminals[current_term].cursor_loc = 0;
    }
    //Getting row and col to display cursor
    cursor_row = (three_terminals[current_term].cursor_loc / NUM_COLS);
    cursor_col = (three_terminals[current_term].cursor_loc % NUM_COLS);
    //Ensuring cursor is not more than max rows
    if (cursor_row >= NUM_ROWS - STATUS_BAR_HEIGHT) {
        cursor_row = NUM_ROWS - 1 - STATUS_BAR_HEIGHT;
    }
    //Updating position of cursor
    uint32_t pos = (cursor_row * NUM_COLS) + cursor_col;
    //Displaying cursor
    outb(0x0F, 0x3D4);
    outb((uint8_t)(pos & 0xFF), 0x3D5);
    outb(0x0E, 0x3D4);
    outb((uint8_t)((pos >> 8) & 0xFF), 0x3D5);
}
/* void enable_cursor();
 * Inputs: None
 * Return Value: None
 * Function: Initializes the display cursor */
void enable_cursor() {

    //Displaying cursor
    outb(0x0A, 0x3D4);
    outb((inb(0x3D5) & 0xC0) | 0, 0x3D5);
    outb(0x3D4, 0x0B);
    outb((inb(0x3D5) & 0xE0) | 15, 0x3D5);

    //Initializing location
    three_terminals[current_term].cursor_loc = 0;
    move_cursor(three_terminals[current_term].cursor_loc);
}
/*
 * read_parser()
 * Description: Helper function for reading from the screen
 * Input: None
 * Output: None
 * Side effects: Updates the history buffer and live buffer
 * Return: None
 */
//parser_buffer[0] to character
//parser_buffer[1] to ctrl
//parser_buffer[2] backspace
//parser_buffer[3] tab
int read_parser(unsigned char * parser_buffer) {

    // only accept keypress if the terminal is doing a terminal read, otherwise block
    if (!three_terminals[current_term].read_in_progress)
        return 0;

    if (parser_buffer == NULL) { //if parser buffer is empty
        return 0;
    } else if (parser_buffer[0] == '\n') { //check if keyboard has pressed an enter key
        new_line();
    } else if (parser_buffer[1] == '1' && (parser_buffer[0] == 'l' || parser_buffer[0] == 'L')) { //cltl -L has been recieved from keyboard handler
        buffer_clear(); //clear the screen and clear history buffer
    } else if (parser_buffer[2] == '1') { //backspace key has been pressed from keyboard handler
        backspace();
    } else if (parser_buffer[3] == '1') { //tab key press has been recieved from keyboard handler
        tab(parser_buffer);
    } else {
        output_char(parser_buffer);
    }
    return 0;
}
/*
 * cursor_left
 * Description: Moves cursor to left
 * Input: None
 * Output: None
 * Side effects: Updates buffer
 * Return: None
 */
void cursor_left(){
    int y_loc;
    int x_loc;
    int row_idx;
    int bytes_written;
    int live_buffer_location;

    y_loc = three_terminals[current_term].screen_y;
    x_loc = three_terminals[current_term].screen_x;
    row_idx = three_terminals[current_term].row_index;
    bytes_written = three_terminals[current_term].bytes_written;
    live_buffer_location = three_terminals[current_term].live_buffer_location;


    if ((live_buffer_location > bytes_written)){
            three_terminals[current_term].cursor_loc--;
            three_terminals[current_term].screen_x = three_terminals[current_term].cursor_loc % NUM_COLS;
            three_terminals[current_term].screen_y = three_terminals[current_term].cursor_loc / NUM_COLS;
            move_cursor(three_terminals[current_term].cursor_loc);
            three_terminals[current_term].live_buffer_location--;
    }
}
/*
 * cursor_right
 * Description: Moves cursor to right
 * Input: None
 * Output: None
 * Side effects: Updates buffer
 * Return: None
 */
void cursor_right(){
    int y_loc;
    int x_loc;
    int row_idx;
    int bytes_written;
    int live_buffer_location;

    y_loc = three_terminals[current_term].screen_y;
    x_loc = three_terminals[current_term].screen_x;
    row_idx = three_terminals[current_term].row_index;
    bytes_written = three_terminals[current_term].bytes_written;
    bytes_read = three_terminals[current_term].bytes_read;
    live_buffer_location = three_terminals[current_term].live_buffer_location;

    if (!(y_loc == NUM_ROWS - 1 && x_loc == NUM_COLS - 1) && (live_buffer_location < bytes_read)){
            three_terminals[current_term].cursor_loc++;
            three_terminals[current_term].screen_x = three_terminals[current_term].cursor_loc % NUM_COLS;
            three_terminals[current_term].screen_y = three_terminals[current_term].cursor_loc / NUM_COLS;
            move_cursor(three_terminals[current_term].cursor_loc);
            three_terminals[current_term].live_buffer_location++;
    }
}
/*
 * command_buffer_scroll
 * Description: Scrolls the command buffer up when necessary
 * Input: None
 * Output: None
 * Side effects: Updates command buffer
 * Return: None
 */
void command_buffer_scroll(int terminal_idx){
    //Initializing variables
    int idx;
    int row;

    for (row = 0; row < MAX_COMMAND_STORE-1; row++){
        for (idx = 0; idx < CHAR_BUFFER_SIZE; idx++){
            three_terminals[terminal_idx].command_buffer[row][idx] = three_terminals[terminal_idx].command_buffer[row+1][idx];
        }
    }

    for (idx = 0; idx < CHAR_BUFFER_SIZE; idx++){
        three_terminals[terminal_idx].command_buffer[MAX_COMMAND_STORE-1][idx] = '\0';
    }

    three_terminals[terminal_idx].write_command_row = MAX_COMMAND_STORE-1;

}
/*
 * prev_command()
 * Description: Scrolls the command buffer up when necessary
 * Input: None
 * Output: None
 * Side effects: Updates command buffer
 * Return: None
 */
void prev_command(){
    int idx;
    int buf_idx;
    int view_row;
    int write_row;
    char parse_buffer[4];
    view_row = three_terminals[current_term].view_command_row;
    write_row = three_terminals[current_term].write_command_row;
    bytes_written = three_terminals[current_term].bytes_written;
    bytes_read_store = three_terminals[current_term].bytes_read_store;

    if (view_row == write_row){
        buf_idx = bytes_written;
        for (idx = 0; idx < SCREEN_SIZE; idx++){
            if (idx < bytes_read_store)
                three_terminals[current_term].temp_commands[idx] = three_terminals[current_term].live_buffer[buf_idx];
            else
            {
                three_terminals[current_term].temp_commands[idx] = '\0';
            }
            buf_idx++;  
        }
        three_terminals[current_term].temp_bytes_read = three_terminals[current_term].bytes_read;
    }

    if (view_row > 0){
        three_terminals[current_term].view_command_row--;
        view_row = three_terminals[current_term].view_command_row;

        for (idx = 0; idx < CHAR_BUFFER_SIZE; idx++) {
            backspace();
        }

        for (idx = 0; idx < CHAR_BUFFER_SIZE; idx++) { //copy data from keyboard buffer into char_buffer
            if (three_terminals[current_term].command_buffer[view_row][idx] == '\0'){
                break;
            }
                parse_buffer[0] = three_terminals[current_term].command_buffer[view_row][idx];
                parse_buffer[1] = '0';
                parse_buffer[2] = '0';
                parse_buffer[3] = '0';
                output_char(parse_buffer);
        }
    } 
}
/*
 * next_command()
 * Description: Scrolls the command buffer up when necessary
 * Input: None
 * Output: None
 * Side effects: Updates command buffer
 * Return: None
 */
void next_command(){
    int idx;
    int buf_idx;
    int view_row;
    int write_row;
    char parse_buffer[4];
    view_row = three_terminals[current_term].view_command_row;
    write_row = three_terminals[current_term].write_command_row;
    bytes_written = three_terminals[current_term].bytes_written;
    bytes_read_store = three_terminals[current_term].bytes_read_store;

    if (view_row == write_row){
        buf_idx = bytes_written;
        for (idx = 0; idx < SCREEN_SIZE; idx++){
            if (idx < bytes_read_store)
                three_terminals[current_term].temp_commands[idx] = three_terminals[current_term].live_buffer[buf_idx];
            else
            {
                three_terminals[current_term].temp_commands[idx] = '\0';
            }
            buf_idx++;  
        }
        three_terminals[current_term].temp_bytes_read = three_terminals[current_term].bytes_read;
    }

    if (view_row < write_row){
        three_terminals[current_term].view_command_row++;
        view_row = three_terminals[current_term].view_command_row;

        for (idx = 0; idx < CHAR_BUFFER_SIZE; idx++) {
            backspace();
        }

        for (idx = 0; idx < CHAR_BUFFER_SIZE; idx++) { //copy data from keyboard buffer into char_buffer
            if (three_terminals[current_term].command_buffer[view_row][idx] == '\0'){
                break;
            }
            if (view_row <= write_row-1)
                parse_buffer[0] = three_terminals[current_term].command_buffer[view_row][idx];
            else
                parse_buffer[0] = three_terminals[current_term].temp_commands[idx];
            parse_buffer[1] = '0';
            parse_buffer[2] = '0';
            parse_buffer[3] = '0';
            output_char(parse_buffer);
        }
    } 
}
/*
 * terminal_scroll
 * Description: Scrolls the terminal window up when necessary
 * Input: None
 * Output: None
 * Side effects: Updates the history buffer to scroll up
 * Return: None
 */
void terminal_scroll() {
    //Initializing variables
    int idx;
    int row;
    uint32_t addr;
    uint32_t terminal_idx;

    //Getting the right address to scroll
    if (current_term == all_process[current_process_pid].active_terminal_idx) {
        addr = VISUAL_VIRTUAL_ADDR;
        terminal_idx = current_term;
    } else {
        terminal_idx = all_process[current_process_pid].active_terminal_idx;
        addr = virtual_terminal_addresses[terminal_idx];
    }

    char * video_mem_ptr = (char * ) addr;

    // copy everything a row up
    memcpy(video_mem_ptr, video_mem_ptr + (NUM_COLS * 2), (NUM_COLS * 2) * (NUM_ROWS - STATUS_BAR_HEIGHT - 1));

    // clear last row
    for (idx = 0; idx < NUM_COLS; idx++) {
        *(uint8_t * )(video_mem_ptr + ((NUM_COLS * (NUM_ROWS - STATUS_BAR_HEIGHT - 1) + idx) << 1)) = '\0';
    }

    // update history buffer
    for (row = 0; row < NUM_ROWS - STATUS_BAR_HEIGHT; row++) {
        for (idx = 0; idx < NUM_COLS; idx++) {
            three_terminals[terminal_idx].history_buffer[row][idx] = * (uint8_t * )(video_mem_ptr + ((NUM_COLS * row + idx) << 1));
        }
    }

    // adjust bookkeeping info
    three_terminals[terminal_idx].row_index--;
    three_terminals[terminal_idx].screen_x = 0;
    three_terminals[terminal_idx].screen_y = (NUM_ROWS - STATUS_BAR_HEIGHT - 1);
    three_terminals[terminal_idx].cursor_loc = (NUM_COLS * three_terminals[terminal_idx].row_index);
}

/*
 * terminal_switch
 * Description: Switches to a different terminal to display
 * Input: new_term - terminal we want to switch to
          all_terms_initialized - have we initialized all the terminals yet
 * Output: none
 * Side effects: displays a different terminal
 * Return: none
 */
void terminal_switch(int new_term, uint8_t all_terms_initialized) {

    if (new_term == current_term)
        return;

    // memcpy whats in 0xb8000 to the old term's buffer
    memcpy((void * ) virtual_terminal_addresses[current_term], (void * ) VISUAL_VIRTUAL_ADDR, 0x1000);
    // memcpy whats in the new term's buffer to 0xb8000
    memcpy((void * ) VISUAL_VIRTUAL_ADDR, (void * ) virtual_terminal_addresses[new_term], 0x1000);

    //Updating the cursor
    uint8_t old_term = current_term;
    current_term = new_term;
    move_cursor(three_terminals[new_term].cursor_loc);

    draw_status_bar();

    //Switching over
    if (all_terms_initialized) {
        current_multiprocess = new_term;
        process_context_switch(old_term, new_term);
    }
}

/*
 * buffer_clear
 * Description: clears terminal buffer
 * Input: none
 * Output: none
 * Side effects: clears terminal buffer
 * Return: none
 */
void buffer_clear() {
    //Initialize variables
    int idx;
    int row;
    three_terminals[current_term].cursor_loc = 0;

    //Move Cursor and updating last row
    move_cursor(three_terminals[current_term].cursor_loc);
    for (row = 0; row < NUM_ROWS - STATUS_BAR_HEIGHT; row++)
        for (idx = 0; idx < NUM_COLS; idx++)
            three_terminals[current_term].history_buffer[row][idx] = '\0'; //clear the history buffer

    clear_multi((uint8_t * ) VISUAL_VIRTUAL_ADDR, current_term);
    three_terminals[current_term].row_index = 0;

    //Reprinting the last row
    for (idx = 0; idx < three_terminals[current_term].bytes_read; idx++) {
        three_terminals[current_term].history_buffer[three_terminals[current_term].row_index][idx] = three_terminals[current_term].live_buffer[idx]; //store the keyboard buffer into history buffer
        putc_multi(three_terminals[current_term].history_buffer[three_terminals[current_term].row_index][idx], (uint8_t * ) VISUAL_VIRTUAL_ADDR, current_term); //output the line keyboard buffer
        three_terminals[current_term].cursor_loc += 1;
        three_terminals[current_term].screen_x = three_terminals[current_term].cursor_loc % NUM_COLS;
        three_terminals[current_term].screen_y = three_terminals[current_term].cursor_loc / NUM_COLS;
        move_cursor(three_terminals[current_term].cursor_loc);
    }

    three_terminals[current_term].live_buffer_location = three_terminals[current_term].bytes_read;

}

/*
 * new_line()
 * Description: Makes the changes on screen for new line
 * Input: none
 * Output: none
 * Side effects: Updates history buffer with new line
 * Return: none
 */
void new_line() {
    int idx;
    for (idx = 0; idx < three_terminals[current_term].bytes_read; idx++) {
        if (idx >= NUM_COLS && (three_terminals[current_term].row_index + 1 < NUM_ROWS - STATUS_BAR_HEIGHT)) {
            three_terminals[current_term].history_buffer[three_terminals[current_term].row_index + 1][idx - NUM_COLS] = three_terminals[current_term].live_buffer[idx];
        } else
            three_terminals[current_term].history_buffer[three_terminals[current_term].row_index][idx] = three_terminals[current_term].live_buffer[idx];
    }
    //update row_indexes, two if more than one line is types
    if (three_terminals[current_term].bytes_read > NUM_COLS)
        three_terminals[current_term].row_index += 2;
    else
        three_terminals[current_term].row_index += 1;
    //store a copy of live_buffer into live_buffer_storage for terminal_read usage
    for (idx = 0; idx < three_terminals[current_term].bytes_read; idx++)
        three_terminals[current_term].live_buffer_store[idx] = three_terminals[current_term].live_buffer[idx]; //store current live buffer into storage for terminal_read usage
    //store a new line character into last character for live_buffer_storage indicates new line character has been pressed
    three_terminals[current_term].live_buffer_store[three_terminals[current_term].bytes_read] = '\n';
    //clear live_buffer
    for (idx = 0; idx < three_terminals[current_term].bytes_read; idx++)
        three_terminals[current_term].live_buffer[idx] = '\0';

    three_terminals[current_term].bytes_read_store = three_terminals[current_term].bytes_read; //store bytes_read into storage

    three_terminals[current_term].bytes_read = 0;
    three_terminals[current_term].live_buffer_location = three_terminals[current_term].bytes_read;
    putc_multi('\n', (uint8_t * ) VISUAL_VIRTUAL_ADDR, current_term);

    //scroll the screen upward
    if (row_index >= NUM_ROWS - STATUS_BAR_HEIGHT) {
        terminal_scroll();
    }

    //reset bytes written
    if (three_terminals[current_term].bytes_written > 0) {
        three_terminals[current_term].bytes_written = 0;
    }

    //update cursor position
    three_terminals[current_term].cursor_loc = ((three_terminals[current_term].cursor_loc / NUM_COLS) + 1) * NUM_COLS;
    move_cursor(three_terminals[current_term].cursor_loc);
}

/*
 * backspace()
 * Description: clears bit in live buffer appropriately
 * Input: none
 * Output: none
 * Side effects: Updates the local live buffer
 * Return: none
 */
void backspace() {
    int idx;
    int cursor_flag;

    if (three_terminals[current_term].live_buffer_location > three_terminals[current_term].bytes_written) {
        three_terminals[current_term].bytes_read--; //decrement amount of bytes read
        three_terminals[current_term].live_buffer_location--;
        cursor_flag = 1;
        if (three_terminals[current_term].bytes_read < 0) { //check if line is already clear, if so set to zero to prevent errors
            three_terminals[current_term].bytes_read = 0; //
            cursor_flag = 0;
        }

        three_terminals[current_term].live_buffer[three_terminals[current_term].live_buffer_location] = '\0'; //set last pressed entry in keyboard buffer to null

        if (three_terminals[current_term].screen_x == 0) {
            three_terminals[current_term].screen_x = NUM_COLS - 1;
            three_terminals[current_term].screen_y -= 1;
        } else {
            three_terminals[current_term].screen_x -= 1;
        }

        if (three_terminals[current_term].live_buffer_location < three_terminals[current_term].bytes_read){
            for (idx = three_terminals[current_term].live_buffer_location; idx < three_terminals[current_term].bytes_read+1; idx++){
                three_terminals[current_term].live_buffer[idx] = three_terminals[current_term].live_buffer[idx+1];
                setc_multi(idx, three_terminals[current_term].row_index, getc_multi(idx+1, three_terminals[current_term].row_index, (uint8_t * ) VISUAL_VIRTUAL_ADDR), (uint8_t * ) VISUAL_VIRTUAL_ADDR);
            }
            three_terminals[current_term].live_buffer[idx] = '\0';
            setc_multi(idx, three_terminals[current_term].row_index, '\0', (uint8_t * ) VISUAL_VIRTUAL_ADDR);
        } else {
            setc_multi(three_terminals[current_term].screen_x, three_terminals[current_term].screen_y, '\0', (uint8_t * ) VISUAL_VIRTUAL_ADDR);
        }

        //Updating cursor if necessary
        if (three_terminals[current_term].cursor_loc > 0 && cursor_flag == 1) {
            three_terminals[current_term].cursor_loc -= 1;
            move_cursor(three_terminals[current_term].cursor_loc);
        }
    }
}

/*
 * tab()
 * Description: adds tab to the live buffer
 * Input: none
 * Output: none
 * Side effects: Updates the live buffer
 * Return: none
 */
void tab(unsigned char * parser_buffer) {
    int idx;
    int line_estimate;
    int tab_counter = 0;
    //The number is 3 because we are added three more spaces after the first for tab key
    if ((three_terminals[current_term].bytes_read + 3) < (CHAR_BUFFER_SIZE - 1 + three_terminals[current_term].bytes_written)) { //check if tab will exceed maxinum char_buffer_size
        for (tab_counter = 0; tab_counter < 4; tab_counter++) { //place 4 tabs into buffer
            if (three_terminals[current_term].bytes_read >= NUM_COLS) { //check if we will got to a second line
                line_estimate = 1; //buffer is over NUM_COL size with the new tab pressed
            } else {
                line_estimate = 0;
            }
            //Scrolling if necessary
            if ((line_estimate + three_terminals[current_term].row_index) >= NUM_ROWS - STATUS_BAR_HEIGHT) {
                terminal_scroll();
                three_terminals[current_term].cursor_loc += NUM_COLS;
            }

            if (three_terminals[current_term].live_buffer_location < three_terminals[current_term].bytes_read)
            for (idx = three_terminals[current_term].bytes_read - 1; idx >= three_terminals[current_term].live_buffer_location; idx--){
                three_terminals[current_term].live_buffer[idx+1] = three_terminals[current_term].live_buffer[idx];
                setc_multi(idx+1, three_terminals[current_term].row_index, getc_multi(idx, three_terminals[current_term].row_index, (uint8_t * ) VISUAL_VIRTUAL_ADDR), (uint8_t * ) VISUAL_VIRTUAL_ADDR);
            }

            three_terminals[current_term].live_buffer[three_terminals[current_term].live_buffer_location] = parser_buffer[0]; //set new character into the keyboard buffer

            putc_multi(parser_buffer[0], (uint8_t * ) VISUAL_VIRTUAL_ADDR, current_term); //print space

            three_terminals[current_term].bytes_read++; //increment the amount of bytes read
            three_terminals[current_term].live_buffer_location++;
        }
        //4 since a tab is four spaces
        three_terminals[current_term].cursor_loc += 4;
        move_cursor(three_terminals[current_term].cursor_loc);
    }
}

/*
 * output_char
 * Description: Outputs regular char to terminal
 * Input: none
 * Output: none
 * Side effects: Updates termainal buffer
 * Return: none
 */
void output_char(unsigned char * parser_buffer) {
    int idx;

    int line_estimate = 0;

    if (three_terminals[current_term].bytes_read < (CHAR_BUFFER_SIZE - 1 + three_terminals[current_term].bytes_written)) { //normal pressing of the keyboard for the rest of the characters
        if (three_terminals[current_term].bytes_read >= NUM_COLS) { //check if amount of character have exceeded NUM_COLS
            line_estimate = 1;
        } else {
            line_estimate = 0;
        }
        if ((line_estimate + three_terminals[current_term].row_index) >= NUM_ROWS - STATUS_BAR_HEIGHT) { //check if we are exceeeding maxinum number of rows
            terminal_scroll();
            three_terminals[current_term].cursor_loc += NUM_COLS;
        }

        if (three_terminals[current_term].live_buffer_location < three_terminals[current_term].bytes_read)
            for (idx = three_terminals[current_term].bytes_read - 1; idx >= three_terminals[current_term].live_buffer_location; idx--){
                three_terminals[current_term].live_buffer[idx+1] = three_terminals[current_term].live_buffer[idx];
                setc_multi(idx+1, three_terminals[current_term].row_index, getc_multi(idx, three_terminals[current_term].row_index, (uint8_t * ) VISUAL_VIRTUAL_ADDR), (uint8_t * ) VISUAL_VIRTUAL_ADDR);
            }

        three_terminals[current_term].live_buffer[three_terminals[current_term].live_buffer_location] = parser_buffer[0]; //set new character into the keyboard buffer

        putc_multi(parser_buffer[0], (uint8_t * ) VISUAL_VIRTUAL_ADDR, current_term); //print the character that was pressed to the screen

        //Updates bytes read and cursor location
        three_terminals[current_term].bytes_read++;
        three_terminals[current_term].live_buffer_location++;
        three_terminals[current_term].cursor_loc += 1;
        move_cursor(three_terminals[current_term].cursor_loc);
    }
}
