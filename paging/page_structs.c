#include "page_structs.h"

/*
 * Initialize the values of the page directory and page table we 
 * load in at boot time.
 */
void init_page_structs() {
    int i;
    
    /* Page Directory Initialization */

    // Initialize all PDEs to blank, non-present entries
    for(i = 0; i < PG_ENTRIES; i++) {
        page_directory[i] = EMPTY_ENTRY;
        page_directory[i] |= RW_SUPERVISOR_ABSENT_MASK;
    }

    // Populate PDE corresponding to video memory page table
    uint32_t page_table_entry = (FIVE_MSB & (uint32_t) page_table);  // Upper 20 bits contain page table address needed for PDE, clear the rest
    page_table_entry |= RW_SUPERVISOR_PRESENT_MASK;
    page_directory[0] = page_table_entry;

    // Kernel 4 MB page starting at 0x400000, r/w with supervisor privs, PCD enabled, global page enabled
    page_directory[1] = FOUR_MB_PAGE_DIR_ENTRY;

    /* End Page Directory Initialization */
    
    /* Page Table Initialization */
    
    // Initialize all PTEs to blank, non-present entries
    for(i = 0; i < PG_ENTRIES; i++) {
        page_table[i] = EMPTY_ENTRY;
        page_table[i] |= RW_SUPERVISOR_ABSENT_MASK;
    }

    // Get virtual address of video memory page
    uint32_t addr = VIDEO_MEM_FULL_ADDR >> THREE_BYTE_SIZE;

    // Create a page table entry for video memory (located from 0xB8000 - 0xB9000 in virtual memory)
    // 4 KB page, r/w with supervisor privs
    // rshift 12 bits b/c page base address is represented by top 20 bits
    page_table[addr] = (VIDEO_MEM_FULL_ADDR) | RW_SUPERVISOR_PRESENT_MASK;  // Default to video mem page corresponding to terminal 1

    // Create page table entries for video memory buffers (located from 0xB9000-0xBC000 in virtual memory)
    // Used to support 3 separate terminals
    for (i = 1; i <= 3; i++) {
        // Backups are contiguous 4 KB memory pages, use 4 KB offset based on current index
        // Physical memory is contiguous from 0xB9000 - 0xBC000
        page_table[addr + i] = (VIDEO_MEM_FULL_ADDR + i * 0x1000) | RW_SUPERVISOR_PRESENT_MASK;
    }

    // flush TLB
	asm volatile ("movl %cr3,%eax; movl %eax,%cr3");
    
    /* End Page Table Initialization */
}

