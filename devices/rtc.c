#include "rtc.h"

uint32_t global_count;
uint32_t prev_count;

/* void rtc_init(void);
 * Inputs: none
 * Return Value: none
 * Function: Initialize RTC */
void rtc_init(void) {
    int8_t prev_A_reg;
    int8_t prev_B_reg;

    NMI_disable();

    enable_irq(RTC_IRQ);
    
    // select register B
    outb(SELECT_B, SELECT_PORT);
    prev_B_reg = inb(WRITE_PORT);
    outb(SELECT_B, SELECT_PORT);
    // turn on bit 6 of register B, sets default frequency to 1024Hz
    outb(prev_B_reg | MASK_SIXTH_BIT, WRITE_PORT);

    // virtualize by setting RTC to max frequency
    outb(SELECT_A, SELECT_PORT);
    prev_A_reg = inb(WRITE_PORT);
    outb(SELECT_A, SELECT_PORT);
    // clear lower 4 bits and overwrite with our rate
    outb((prev_A_reg & MASK_UPPER_BYTE) | RTC_MAX_RATE, WRITE_PORT);

    // init global variables
    global_count = 0;
    prev_count = 0;

    NMI_enable();
}

/* void rtc_handler(void);
 * Inputs: none
 * Return Value: none
 * Function: Entry for RTC Handler */
void rtc_handler(void) {
    
    global_count += 1;
    
    // select register C
    outb(SELECT_C, SELECT_PORT);
    // flush buffer
    uint8_t c_val = inb(WRITE_PORT);
    
    if((c_val & 0x10) >> 4) {
        cmos_handler();
    }

    // send EOI
    send_eoi(RTC_IRQ);
}

/* void NMI_enable(void);
 * Inputs: none
 * Return Value: none
 * Function: Enables non-maskable interrupts */
void NMI_enable() {
    outb(inb(SELECT_PORT) & UNMASK_NMI, SELECT_PORT);
}
 
/* void NMI_disable(void);
 * Inputs: none
 * Return Value: none
 * Function: Disables non-maskable interrupts */
void NMI_disable() {
    outb(inb(SELECT_PORT) | MASK_NMI, SELECT_PORT);
}

/* void rtc_open(const uint8_t* filename);
 * Inputs: ignored
 * Return Value: 0 on success
 * initializes rtc to 2hz */
int32_t rtc_open(const uint8_t* filename) {
    return 0;
}

/* void rtc_read(int32_t fd, void* buf, int32_t nbytes);
 * Inputs: ignored
 * Return Value: 0 on success
 * spins until interrupt is received */
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes) {
    uint32_t entered_count = global_count;
    // wait until division # of ticks have passed since we entered rtc_read
    while(entered_count + PCB[current_process_pid]->file[fd].file_position > global_count);
    return 0;
}

/* void rtc_read(int32_t fd, const void* buf, int32_t nbytes);
 * Inputs: all ignored except for buf -- ptr to integer refering to frequency
 * Return Value: -1 on fail, 0 on success
 * spins until interrupt is received */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes) {
    int32_t freq = *((int *) buf);
    if(freq != 0 && (freq & (freq-1)) == 0 && freq <= RTC_MAX_FREQ) {
        // frequency is a power of 2 and is less than the maximum allowed freq
        // save our division in the file position in fd index for the right process
        PCB[current_process_pid]->file[fd].file_position = RTC_MAX_FREQ / freq;
        return 0;
    }
    // one of the preconditions failed
    return -1;
}

/* void rtc_close(void);
 * Inputs: ignored
 * Return Value: 0 on success
 * closes the RTC */
int32_t rtc_close(int32_t fd) {
    PCB[current_process_pid]->file[fd].file_position = 0;
    return 0;
}


