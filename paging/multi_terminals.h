#ifndef _MULTI_TERMINALS_H
#define _MULTI_TERMINALS_H

#include "page_structs.h"
#include "../devices/terminal_structs.h"
#include "../devices/terminal.h"
#include "../interrupts/syscalls.h"

#define VISUAL_VIRTUAL_ADDR 0xB8000
#define TERM_1_PHYSICAL_BACKUP 0xB9000
#define TERM_2_PHYSICAL_BACKUP 0xBA000
#define TERM_3_PHYSICAL_BACKUP 0xBB000
#define TERM_1_VIRTUAL_BACKUP 0xB9000
#define TERM_2_VIRTUAL_BACKUP 0xBA000
#define TERM_3_VIRTUAL_BACKUP 0xBB000

extern uint32_t virtual_terminal_addresses[3];
extern uint32_t physical_terminal_addresses[3];

void three_term_init(); 
#endif 
