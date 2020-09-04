#include "../lib.h"
#include "../i8259.h"
#include "../interrupts/syscall_structs.h"
#include "../scheduler/scheduler.h"
#include "cmos.h"

#define RTC_IRQ 8
#define SELECT_PORT 0x70
#define WRITE_PORT 0x71
#define SELECT_A 0x0A
#define SELECT_B 0x0B
#define SELECT_C 0x0C

#define MASK_UPPER_BYTE 0xF0
#define MASK_LOWER_BYTE 0x0F
#define MASK_SIXTH_BIT 0x40

#define MASK_NMI 0x80
#define UNMASK_NMI 0x7F

// should be 3, but in reality the maximum rate is actually 512Hz, which corresponds to rate 7
#define RTC_MAX_RATE 7
#define RTC_MAX_FREQ (32768 >> (RTC_MAX_RATE - 1))

#define DEFAULT_RTC_FREQ 2

#ifndef _RTC_H_
#define _RTC_H_

/* Initialize RTC */
void rtc_init(void);

/* entry for irq 8 handler */
void rtc_handler(void);

/* enables NMIs */
void NMI_enable(void);

/* disables NMIs */
void NMI_disable(void);

/* opens RTC with no side effects */
int32_t rtc_open(const uint8_t* filename);

/* writes frequency from buf */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);

/* spins until interrupt is received */
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);

/* closes RTC */
int32_t rtc_close(int32_t fd);

#endif
