#include "syscall_structs.h"

// initialize global variables
uint8_t current_process_pid = 0;
PCB_BLOCK_t* PCB[MAX_PROCESSES];

/*
 * get_current_ebp
 * Description: Gets the current base pointer
 * Inputs: None
 * Return Value: ebp address
 * Side Effects: Calls inline asm.
 */
uint32_t get_current_ebp() {
	uint32_t address;
	asm (
		"movl %%ebp, %0;\n"
        : "=g"(address)
		:
    );
	return address;
}
