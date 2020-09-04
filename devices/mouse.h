#ifndef _MOUSE_H_
#define _MOUSE_H_

#include "../lib.h"
#include "../i8259.h"
#include "keyboard.h"
#include "../scheduler/scheduler.h"

#define MOUSE_DATA_PORT 0x60
#define MOUSE_COMMAND_PORT 0x64
#define MOUSE_IRQ 12

#define ENABLE_AUX_MOUSE_DEVICE 0xA8
#define GET_COMPAQ_STATUS 0x20
#define SET_COMPAQ_STATUS 0x60 
#define DATA_PORT_SIGNAL 0xD4
#define SET_MOUSE_DEFAULT_SETTINGS 0xF6
#define ENABLE_MOUSE_STREAMING 0xF4
#define MOUSE_CMD_SAMPLE_RATE 0xF3

#define INVERT_SIGN 0xFFFFFF00

#define FAKE_MOUSE_RES_X 320
#define FAKE_MOUSE_RES_Y 200

#define TIMEOUT 100000

typedef struct mouse {
    uint32_t left_btn   : 1;
    uint32_t right_btn  : 1;
    uint32_t middle_btn : 1;
    uint32_t val        : 1;
    uint32_t x_sign_bit : 1;
    uint32_t y_sign_bit : 1;
    uint32_t x_overflow : 1;
    uint32_t y_overflow : 1;
    uint32_t x_movement : 8;
    uint32_t y_movement : 8;
    uint32_t padding    : 8;
} __attribute__ ((packed)) mouse_t;

void mouse_init(void);
void mouse_wait(uint8_t flag);
void mouse_handler(void);
int32_t mouse_open(const uint8_t* filename);
int32_t mouse_read(int fd, void* char_buffer, int bytes);
int32_t mouse_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t mouse_close(int32_t fd);

#endif
