#ifndef _IDT_EXCEPTION_LINKAGE_H_
#define _IDT_EXCEPTION_LINKAGE_H_

extern void divide_error_wrapper(); 

extern void debug_error_wrapper(); 

extern void nmi_interrupt_error_wrapper(); 

extern void breakpoint_error_wrapper();

extern void overflow_error_wrapper(); 

extern void bound_range_exceeded_error_wrapper(); 

extern void invalid_opcode_error_wrapper(); 

extern void device_not_available_error_wrapper(); 

extern void dbl_fault_error_wrapper(); 

extern void coprocess_segment_overrun_error_wrapper(); 

extern void invalid_tss_error_wrapper(); 

extern void segment_not_present_error_wrapper(); 

extern void stack_segment_fault_error_wrapper(); 

extern void gen_protection_error_wrapper(); 

extern void page_fault_error_wrapper(); 

extern void float_point_error_wrapper(); 

extern void align_check_error_wrapper(); 

extern void machine_check_error_wrapper();

extern void simd_floating_point_error_wrapper(); 
#endif
