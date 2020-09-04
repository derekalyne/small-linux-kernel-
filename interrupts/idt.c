#include "idt.h"
#include "syscall_structs.h"

// The following functions are function pointers that are exceptions handlers 


void divide_error(){
	// halt for kernel exceptions
	uint32_t address = get_saved_eip_addr();
	if(address >= KERNEL_MEM_START && address <= KERNEL_MEM_END) {
        cli();
		printf("0: Divide By Zero Error\n");
		while(1);
	}
	sys_write(1, (void*)"0: Divide By Zero Error\n", 24);
	sys_halt_wrapper(HALT_EXCEPTION);
}

void debug_error(){
	 
	// halt for kernel exceptions
	uint32_t address = get_saved_eip_addr();
	if(address >= KERNEL_MEM_START && address <= KERNEL_MEM_END) {
        cli();
		printf("1: Debug Error\n");
		while(1);
	}
	sys_write(1, (void*)"1: Debug Error\n", 15);
	sys_halt_wrapper(HALT_EXCEPTION); 
}

void nmi_interrupt_error(){
	
	// halt for kernel exceptions
	uint32_t address = get_saved_eip_addr();
	if(address >= KERNEL_MEM_START && address <= KERNEL_MEM_END) {
        cli();
		printf("2: Non-maskable Interrupt Error\n");
		while(1);
	}
	sys_write(1, (void*)"2: Non-maskable Interrupt Error\n", 32);
	sys_halt_wrapper(HALT_EXCEPTION);
}

void breakpoint_error(){
	
	// halt for kernel exceptions
	uint32_t address = get_saved_eip_addr();
	if(address >= KERNEL_MEM_START && address <= KERNEL_MEM_END) {
        cli();
		printf("3: Breakpoint Error\n");
		while(1);
	}
	sys_write(1, (void*)"3: Breakpoint Error\n", 20);
	sys_halt_wrapper(HALT_EXCEPTION);
}

void overflow_error(){
	
	// halt for kernel exceptions
	uint32_t address = get_saved_eip_addr();
	if(address >= KERNEL_MEM_START && address <= KERNEL_MEM_END) {
        cli();
		printf("4: Overflow Error\n");
		while(1);
	}
	sys_write(1, (void*)"4: Overflow Error\n", 18);
	sys_halt_wrapper(HALT_EXCEPTION); 
}

void bound_range_exceeded_error(){
	
	// halt for kernel exceptions
	uint32_t address = get_saved_eip_addr();
	if(address >= KERNEL_MEM_START && address <= KERNEL_MEM_END) {
        cli();
		printf("5: Bound Range Exceeded Error\n");
		while(1);
	}
	sys_write(1, (void*)"5: Bound Range Exceeded Error\n", 30);
	sys_halt_wrapper(HALT_EXCEPTION);
}

void invalid_opcode_error(){
	
	// halt for kernel exceptions
	uint32_t address = get_saved_eip_addr();
	if(address >= KERNEL_MEM_START && address <= KERNEL_MEM_END) {
        cli();
		printf("6: Invalid Opcode Error\n");
		while(1);
	}
	sys_write(1, (void*)"6: Invalid Opcode Error\n", 24);
	sys_halt_wrapper(HALT_EXCEPTION);
}

void device_not_available_error(){
	
	// halt for kernel exceptions
	uint32_t address = get_saved_eip_addr();
	if(address >= KERNEL_MEM_START && address <= KERNEL_MEM_END) {
        cli();
		printf("7: Device Not Available Error");
		while(1);
	}
	sys_write(1, (void*)"7: Device Not Available Error", 28);
	sys_halt_wrapper(HALT_EXCEPTION);
}

void dbl_fault_error(){
	
	// halt for kernel exceptions
	uint32_t address = get_saved_eip_addr();
	if(address >= KERNEL_MEM_START && address <= KERNEL_MEM_END) {
        cli();
		printf("8: Double Fault Error\n");
		while(1);
	}
	sys_write(1, (void*)"8: Double Fault Error\n", 22);
	sys_halt_wrapper(HALT_EXCEPTION); 
}

void coprocess_segment_overrun_error(){
	
	// halt for kernel exceptions
	uint32_t address = get_saved_eip_addr();
	if(address >= KERNEL_MEM_START && address <= KERNEL_MEM_END) {
        cli();
		printf("2: Non-maskable Interrupt Error\n");
		while(1);
	}
	sys_write(1, (void*)"2: Non-maskable Interrupt Error\n", 38);
	sys_halt_wrapper(HALT_EXCEPTION); 
}

void invalid_tss_error(){
	
	// halt for kernel exceptions
	uint32_t address = get_saved_eip_addr();
	if(address >= KERNEL_MEM_START && address <= KERNEL_MEM_END) {
        cli();
		printf("10: Invalid TSS Error\n");
		while(1);
	}
	sys_write(1, (void*)"10: Invalid TSS Error\n", 22);
	sys_halt_wrapper(HALT_EXCEPTION); 
}

void segment_not_present_error(){
	
	uint32_t address = get_saved_eip_addr();
	if(address >= KERNEL_MEM_START && address <= KERNEL_MEM_END) {
        cli();
		while(1);
		printf("11: Segment Not Present Error\n");
	}
	sys_write(1, (void*)"11: Segment Not Present Error\n", 30);
	sys_halt_wrapper(HALT_EXCEPTION); 
}

void stack_segment_fault_error(){
	
	// halt for kernel exceptions
	uint32_t address = get_saved_eip_addr();
	if(address >= KERNEL_MEM_START && address <= KERNEL_MEM_END) {
        cli();
		printf("12: Stack Segment Fault Error\n");
		while(1);
	}
	sys_write(1, (void*)"12: Stack Segment Fault Error\n", 30);
	sys_halt_wrapper(HALT_EXCEPTION);
}

void gen_protection_error(){
	
	// halt for kernel exceptions
	uint32_t address = get_saved_eip_addr();
	if(address >= KERNEL_MEM_START && address <= KERNEL_MEM_END) {
        cli();
		printf("13: General Protection Error\n");
		while(1);
	}
	sys_write(1, (void*)"13: General Protection Error\n", 29);
	sys_halt_wrapper(HALT_EXCEPTION); 
}

void page_fault_error(){
	
	// halt for kernel exceptions
	uint32_t address = get_saved_eip_addr();
	if(address >= KERNEL_MEM_START && address <= KERNEL_MEM_END) {
        cli();
		printf("14: Page Fault Error\n");
		uint32_t addr = get_page_fault_addr();
		printf("at address: %x", addr);
		while(1);
	}
	sys_write(1, (void*)"14: Page Fault Error\n", 21);
	sys_halt_wrapper(HALT_EXCEPTION); 
}

void float_point_error(){
	
	// halt for kernel exceptions
	uint32_t address = get_saved_eip_addr();
	if(address >= KERNEL_MEM_START && address <= KERNEL_MEM_END) {
        cli();
		printf("16: Floating Point Error\n");
		while(1);
	}
	sys_write(1, (void*)"16: Floating Point Error\n", 25);
	sys_halt_wrapper(HALT_EXCEPTION); 
}

void align_check_error(){
	
	// halt for kernel exceptions
	uint32_t address = get_saved_eip_addr();
	if(address >= KERNEL_MEM_START && address <= KERNEL_MEM_END) {
        cli();
		printf("17: Align Check Error\n");
		while(1);
	}
	sys_write(1, (void*)"17: Align Check Error\n", 22);
	sys_halt_wrapper(HALT_EXCEPTION); 
}

void machine_check_error(){
	
	// halt for kernel exceptions
	uint32_t address = get_saved_eip_addr();
	if(address >= KERNEL_MEM_START && address <= KERNEL_MEM_END) {
        cli();
		printf("18: Machine Check Error\n");
		while(1);
	}
	sys_write(1, (void*)"18: Machine Check Error\n", 26);
	sys_halt_wrapper(HALT_EXCEPTION); 
}

void simd_floating_point_error(){
	
	// halt for kernel exceptions
	uint32_t address = get_saved_eip_addr();
	if(address >= KERNEL_MEM_START && address <= KERNEL_MEM_END) {
        cli();
		printf("19: Simd Floating Point Error\n");
		while(1);
	}
	sys_write(1, (void*)"19: Simd Floating Point Error\n", 30);
	sys_halt_wrapper(HALT_EXCEPTION); 
}

/* init_idt
 * 
 * initializes IDT, which is required for handling exceptions hardware/device interrupts 
 * Inputs: None
 * Outputs: NONE
 * Side Effects: initializes IDT 
 * Files: idt.c/idt.h 
 */
void init_idt() { //for IDT look at "x86_desc.h"
    int idx;
    for(idx = 0; idx < NUM_VEC; idx++) {
		// look at figure 5-2 IDT Gate Descriptors on 5.12 on IA-32 Intel Manuel vol 3 
		// look at idt structure documentation in x86_desc.h 
        idt[idx].reserved0 = 0;
		idt[idx].reserved1 = 1;
		idt[idx].reserved2 = 1;
		if(idx >= PIC_INTERRUPT_START && idx <= PIC_INTERRUPT_END) 
			idt[idx].reserved3 = 0; 
		else 
			idt[idx].reserved3 = 1;

		if(idx == SYSCALL) 
			idt[idx].reserved3 = 1;

		idt[idx].reserved4 = 0;
		
		idt[idx].size = 1;
		idt[idx].present = 1;
		
		if(idx == SYSCALL) //change the priority for hardware/exception to device level priority 
			idt[idx].dpl = 3;
		else 
			idt[idx].dpl = 0; 
		idt[idx].seg_selector = KERNEL_CS;
	}
	//look at table 5-1. on 5.3.1 on IA-32Intel Manuel vol 3 
	SET_IDT_ENTRY(idt[DIVIDE_ZERO], divide_error_wrapper); //divide by zero error
	SET_IDT_ENTRY(idt[DEBUG],  debug_error_wrapper); //debug error 
	SET_IDT_ENTRY(idt[NMI], nmi_interrupt_error_wrapper); //NVM interrupt 
	SET_IDT_ENTRY(idt[BREAK], breakpoint_error_wrapper); //breakpoint error 
	SET_IDT_ENTRY(idt[OVER], overflow_error_wrapper); //overflow error 
	SET_IDT_ENTRY(idt[BOUNDRANGE], bound_range_exceeded_error_wrapper); //bound range exceeded error 
	SET_IDT_ENTRY(idt[INVALID_OP], invalid_opcode_error_wrapper); //invalid opcode error 
	SET_IDT_ENTRY(idt[DEVICE], device_not_available_error_wrapper); //invalid device not available error 
	SET_IDT_ENTRY(idt[DBL_FAULT], dbl_fault_error_wrapper); //double fault error 
	SET_IDT_ENTRY(idt[CO_SEGMENT],  coprocess_segment_overrun_error_wrapper); //co-processor error 
	SET_IDT_ENTRY(idt[INVALID_TSS], invalid_tss_error_wrapper); //invalid tss error 
	SET_IDT_ENTRY(idt[SEGMENT], segment_not_present_error_wrapper); //invalid segment not present error 
	SET_IDT_ENTRY(idt[STACK], stack_segment_fault_error_wrapper); //stack-segment fault 
	SET_IDT_ENTRY(idt[GEN_PROT], gen_protection_error_wrapper); //general protection error 
	SET_IDT_ENTRY(idt[PAGE_FAULT], page_fault_error_wrapper); //page-fault error 
	//15, Intel reserved exception 
	SET_IDT_ENTRY(idt[FLOAT_POINT], float_point_error_wrapper); //FPU floating point error 
	SET_IDT_ENTRY(idt[ALIGN], align_check_error_wrapper); //align check error 
	SET_IDT_ENTRY(idt[MACHINE],  machine_check_error_wrapper); //machine check error 
	SET_IDT_ENTRY(idt[SIMD_FLOAT],  simd_floating_point_error_wrapper); //smid floating point error 
	
	SET_IDT_ENTRY(idt[PIT_DEVICE], irq_pit);
	SET_IDT_ENTRY(idt[KEYBOARD], irq_keyboard);
	SET_IDT_ENTRY(idt[RTC], irq_rtc); 
	SET_IDT_ENTRY(idt[MOUSE], irq_mouse); 
	SET_IDT_ENTRY(idt[SYSCALL], irq_syscall); 
    lidt(idt_desc_ptr); //load IDT table into description pointer 
}

/*
 * get_page_fault_addr
 * Description: Gets the address we page faulted at from CR2 register
 * Inputs: None
 * Return Value: page fault address
 * Side Effects: Calls inline asm.
 */
uint32_t get_page_fault_addr() {
	uint32_t addr;
	asm (
		"movl %%cr2, %0;\n"
        : "=r"(addr)
    );

	return addr;
}

/*
 * get_saved_eip_addr
 * Description: Gets the saved eip from iret structure. The saved eip is at an offset of 48 bytes from ebp,
 *				it used to be 28 bytes from esp, but then we allocated a local var, 
 *				then called this func, and allocated another local var
 * Inputs: None
 * Return Value: eip address
 * Side Effects: Calls inline asm.
 */
uint32_t get_saved_eip_addr() {
	uint32_t address;
	asm (
		"movl 48(%%ebp), %%ebx;\n"
		"movl %%ebx, %0;\n"
        : "=g"(address)
		:
		: "ebx"
    );
	return address;
}
