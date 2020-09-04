#include "../types.h"

#ifndef _PAGE_STRUCTS_H
#define _PAGE_STRUCTS_H

// 4 kB and 4 MB page table sizes
#define PG_BASE_SIZE 4096  
#define PG_LRG_SIZE 4096000

#define PG_ENTRIES 1024

// Bits that mark the named flags as enabled
#define RW_SUPERVISOR_PRESENT_MASK 0x00000003
#define RW_SUPERVISOR_ABSENT_MASK  0x00000002
#define RW_USER_PRESENT_4MB_MASK   0x00000087
//vidmap magic number
#define USER_READ_WRITE_PRESENT_ENABLE 7 //represents 0b111, enables user, read/write, and present bits

#define FIVE_MSB 0xFFFFF000
#define EMPTY_ENTRY 0x00000000
#define THREE_BYTE_SIZE 12

// address for where video mem starts
#define VIDEO_MEM_FULL_ADDR 0x000B8000

// hardcoded entry for 4mb kernel page
#define FOUR_MB_PAGE_DIR_ENTRY 0x00400193

#define KERNEL_MEM_START 0x400000
#define KERNEL_MEM_END   0x800000

#ifndef ASM

uint32_t page_directory[PG_ENTRIES] __attribute__ ((aligned (PG_BASE_SIZE)));
uint32_t page_table[PG_ENTRIES] __attribute__ ((aligned (PG_BASE_SIZE)));
uint32_t page_table_vid[PG_ENTRIES]  __attribute__((aligned(PG_BASE_SIZE)));
void init_page_structs(void);

#endif /* ASM */

#endif /* _PAGE_STRUCTS_H */
