#ifndef _CMOS_H_
#define _CMOS_H_

#include "../lib.h"
#include "../i8259.h"
#include "rtc.h"

#define UPDATE_INTERRUPT_FLAG_BITMASK 0x10
#define RTC_HOURS 0x04
#define RTC_MINUTES 0x02
#define RTC_SECONDS 0x00
#define UTC_TO_EST_OFFSET 20
#define HOURS_IN_DAY 24

void cmos_init(void);
void cmos_handler(void);

#endif
