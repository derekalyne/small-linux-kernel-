#include "mouse.h"
#include "keyboard.h"

mouse_t mouse_data;

uint16_t fake_mouse_pos_x = FAKE_MOUSE_RES_X / 2;
uint16_t fake_mouse_pos_y = FAKE_MOUSE_RES_Y / 2;
uint16_t x_pos = NUM_COLS / 2;
uint16_t y_pos = NUM_ROWS / 2;

/* int32_t mouse_open(const uint8_t* filename);
 * Inputs: filename -- ignored
 * Return Value: 0
 * Function: sets mouse position in middle of screen */
int32_t mouse_open(const uint8_t* filename) {
    x_pos = NUM_COLS / 2;
    y_pos = NUM_ROWS / 2;
    fake_mouse_pos_x = FAKE_MOUSE_RES_X / 2;
    fake_mouse_pos_y = FAKE_MOUSE_RES_Y / 2;
    return 0;
}

/* int32_t mouse_read(int fd, void* char_buffer, int bytes);
 * Inputs: fd -- ignored, char_buffer -- buffer for storing mouse data, bytes -- ignored
 * Return Value: 0
 * Function: copies x_pos into two bytes, y_pos into two bytes, left btn, middle btn, right btn
 *           into 7 bytes in char_buffer. */
int32_t mouse_read(int fd, void* char_buffer, int bytes) {
    char* ptr = (char *) char_buffer;
    *(ptr) = x_pos;
    *(ptr + 2) = y_pos;
    *(ptr + 4) = mouse_data.left_btn;
    *(ptr + 5) = mouse_data.middle_btn;
    *(ptr + 6) = mouse_data.right_btn;
    return 0;
}

/* int32_t mouse_write(int32_t fd, const void* buf, int32_t nbytes);
 * Inputs: fd -- ignored, buf -- buffer reading mouse data from, nbytes -- ignored
 * Return Value: 0
 * Function: sets x_pos (2 bytes), y_pos (2 bytes) */
int32_t mouse_write(int32_t fd, const void* buf, int32_t nbytes) {
    uint16_t temp_x;
    uint16_t temp_y;
    char* ptr = (char *) buf;
    temp_x = *(ptr);
    temp_y = *(ptr + 2);
    if(temp_x >= NUM_COLS || temp_y >= NUM_ROWS)
        return -1;
    x_pos = temp_x;
    y_pos = temp_y;
    return 0;
}

/* int32_t mouse_close(int32_t fd);
 * Inputs: fd -- ignored
 * Return Value: 0
 * Function: does nothing */
int32_t mouse_close(int32_t fd) {
    return 0;
}

// https://forum.osdev.org/viewtopic.php?t=10247
/* void mouse_wait(uint8_t flag)
 * Inputs: flag -- waiting on read/write
 * Return Value: 0
 * Function: waits for data to send */
void mouse_wait(uint8_t flag) {
    int timeout = TIMEOUT;
    if(flag) {
        while(timeout--) {
            if(inb(MOUSE_COMMAND_PORT) & 1)
                return;
        }
    } else {
        while(timeout--) {
            if((inb(MOUSE_COMMAND_PORT) & 2) == 0)
                return;
        }
    }
    return;
}

/* void mouse_write_data(uint8_t val)
 * Inputs: val -- value to write to mouse data port
 * Return Value: None
 * Function: Preps mouse to send to data port, then sends data */
void mouse_write_data(uint8_t val) {
    mouse_wait(1);
    outb(DATA_PORT_SIGNAL, MOUSE_COMMAND_PORT);
    mouse_wait(1);
    outb(val, MOUSE_DATA_PORT);
}

/* uint8_t mouse_read_data()
 * Inputs: None
 * Return Value: byte read from mouse data port
 * Function: Reads data from mouse data port */
uint8_t mouse_read_data() {
    mouse_wait(0);
    return inb(MOUSE_DATA_PORT);
}

/* void mouse_init()
 * Inputs: None
 * Return Value: None
 * Function: Does initialization for mouse */
void mouse_init() {
    uint8_t status;

    mouse_wait(1);
    outb(ENABLE_AUX_MOUSE_DEVICE, MOUSE_COMMAND_PORT);

    // enable mouse
    mouse_wait(1);
    outb(GET_COMPAQ_STATUS, MOUSE_COMMAND_PORT);
    mouse_wait(0);
    status = inb(MOUSE_DATA_PORT);
    // set bit 1
    status |= 0x02;
    // clear bit 5
    status &= 0xDF;
    mouse_wait(1);
    outb(SET_COMPAQ_STATUS, MOUSE_COMMAND_PORT);
    mouse_wait(1);
    outb(status, MOUSE_DATA_PORT);
    mouse_wait(1);

    // use default settings
    mouse_write_data(SET_MOUSE_DEFAULT_SETTINGS);
    mouse_read_data(); // ack

    // enable packet streaming
    mouse_write_data(ENABLE_MOUSE_STREAMING);
    mouse_read_data(); // ack

    enable_irq(MOUSE_IRQ);
}

/* void mouse_handler()
 * Inputs: None
 * Return Value: None
 * Function: interrupt handler for mouse */
void mouse_handler() {
    int x_vel;
    int y_vel;
    uint32_t data = (inb(MOUSE_DATA_PORT)) | (inb(MOUSE_DATA_PORT) << 8) | (inb(MOUSE_DATA_PORT) << 16);
    mouse_t temp_mouse;
    memcpy(&temp_mouse, &data, 4);
    // if these conditions aren't true, we didn't receive a valid packet
    if(temp_mouse.val == 0 || temp_mouse.y_overflow == 1 || temp_mouse.x_overflow == 1) {
        send_eoi(MOUSE_IRQ);
        return;
    }
    mouse_data = temp_mouse;

    x_vel = (!mouse_data.x_sign_bit) ? mouse_data.x_movement : -(0xFF - mouse_data.x_movement);
    y_vel = (!mouse_data.y_sign_bit) ? mouse_data.y_movement : -(0xFF - mouse_data.y_movement);

    x_vel /= 2;
    y_vel /= 2;
    
    fake_mouse_pos_x = min(max(fake_mouse_pos_x + x_vel, 0), FAKE_MOUSE_RES_X - 1);
    fake_mouse_pos_y = min(max(fake_mouse_pos_y - y_vel, 0), FAKE_MOUSE_RES_Y - 1);

    // do a proportion between fake to real
    x_pos = (fake_mouse_pos_x * NUM_COLS) / FAKE_MOUSE_RES_X;
    y_pos = (fake_mouse_pos_y * NUM_ROWS) / FAKE_MOUSE_RES_Y;

    send_eoi(MOUSE_IRQ);
}
