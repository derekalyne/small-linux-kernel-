#ifndef _IDT_H_
#define _IDT_H_

#ifndef ASM

#include "idt_device.h" //used for setting up device interrupts like irq_keyboard and irq_rtc
#include "../x86_desc.h"
#include "../lib.h" 
#include "../paging/page_structs.h"
#include "syscalls_linkage.h"
#include "idt_exception_linkage.h"

extern void init_idt();

//creates a divide by zero exception 

void divide_error(); 

//creates a debug exception
 
void debug_error(); 

//non-maskable external interrupt 

void nmi_interrupt_error(); 

//Breakpoint exception 

void breakpoint_error(); 

//stack overflow, stack is completely full 

void overflow_error(); 

//accessing an element that exceeds allocated memory for a structure 

void bound_range_exceeded_error(); 

//invalid operation 

void invalid_opcode_error(); 

//a device is not properly connect, Example: hard-drive missing 

void device_not_available_error(); 

// double fault error 

void dbl_fault_error(); 

//detected a page of segmentation violation within a process
 
void coprocess_segment_overrun_error(); 

//task switching not properly set up 

void invalid_tss_error(); 

//Loading segment registers or accessing system segment issues
 
void segment_not_present_error(); 

//stack operations and ss register loads have issues 

void stack_segment_fault_error(); 

//any memory reference and other protection check issues
 
void gen_protection_error(); 

//any memory reference issue with accessing invalid page in memory  

void page_fault_error(); 

//x87 FPU floating point calculation issue 

void float_point_error(); 

//issue with stack alignment 

void align_check_error(); 

//error code and source are model dependent 

void machine_check_error(); 

//SSE floating point issue 

void simd_floating_point_error();

/* Get the address that caused a page fault. Will always be located in control register 3. */
uint32_t get_page_fault_addr();

/* Get the address that caused exception. Will always be located 7 addresses below esp */
uint32_t get_saved_eip_addr();

#define SYSCALL		0x80
#define RTC			0x28
#define KEYBOARD	0x21
#define PIT_DEVICE  0x20
#define MOUSE       0x2C

#define DIVIDE_ZERO 0 
#define DEBUG       1 
#define NMI         2 
#define BREAK       3 
#define OVER        4 
#define BOUNDRANGE  5 
#define INVALID_OP  6 
#define DEVICE      7 
#define DBL_FAULT   8 
#define CO_SEGMENT  9 
#define INVALID_TSS 10 
#define SEGMENT     11 
#define STACK       12 
#define GEN_PROT    13 
#define PAGE_FAULT  14 
#define FLOAT_POINT 16
#define ALIGN       17 
#define MACHINE     18 
#define SIMD_FLOAT  19 

#define PIC_MASTER_START 32
#define PIC_MASTER_END   39 

#define PIC_INTERRUPT_START 0x20
#define PIC_INTERRUPT_END 0x2F

#define HALT_EXCEPTION 256

#endif  // ASM

#endif  // _IDT_H_


