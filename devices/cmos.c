#include "cmos.h"

void cmos_init() {
    // enable update interrupts
    uint8_t regB_val;
    outb(SELECT_B, SELECT_PORT);
    regB_val = inb(WRITE_PORT);
    outb(SELECT_B, SELECT_PORT);
    // turn on bit 4 of register B, enables update interrupts
    outb(regB_val | UPDATE_INTERRUPT_FLAG_BITMASK, WRITE_PORT);
}

void cmos_handler() {
    uint8_t cmos_hours;
    uint8_t cmos_minutes;
    uint8_t cmos_sec;
    
    // called from rtc_handler
    outb(RTC_HOURS, SELECT_PORT);
    cmos_hours = inb(WRITE_PORT);
    // convert from BCD to binary
    cmos_hours = ((cmos_hours & 0x0F) + (((cmos_hours & 0x70) / 16) * 10) ) | (cmos_hours & 0x80);
    cmos_hours = (cmos_hours + UTC_TO_EST_OFFSET) % HOURS_IN_DAY;

    outb(RTC_MINUTES, SELECT_PORT);
    cmos_minutes = inb(WRITE_PORT);
    // convert from BCD to binary
    cmos_minutes = (cmos_minutes & 0x0F) + ((cmos_minutes / 16) * 10);

    outb(RTC_SECONDS, SELECT_PORT);
    cmos_sec = inb(WRITE_PORT);
    // convert from BCD to binary
    cmos_sec = (cmos_sec & 0x0F) + ((cmos_sec / 16) * 10);

    // printf("cmos int\n");

    // things to call on update to datetime
    update_status_bar_time(cmos_hours, cmos_minutes, cmos_sec);
}
