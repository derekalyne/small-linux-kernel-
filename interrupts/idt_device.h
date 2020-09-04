#ifndef _IDT_DEVICE_H_
#define _IDT_DEVICE_H_

#include "../devices/keyboard.h" 
#include "../devices/rtc.h" 
#include "../devices/pit.h"
#include "../devices/mouse.h"

extern void irq_keyboard();

extern void irq_rtc();

extern void irq_pit();

extern void irq_mouse();

#endif
