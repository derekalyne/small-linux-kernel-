.globl divide_error_wrapper
.globl debug_error_wrapper
.globl nmi_interrupt_error_wrapper
.globl breakpoint_error_wrapper
.globl overflow_error_wrapper
.globl bound_range_exceeded_error_wrapper
.globl invalid_opcode_error_wrapper
.globl device_not_available_error_wrapper
.globl dbl_fault_error_wrapper
.globl coprocess_segment_overrun_error_wrapper
.globl invalid_tss_error_wrapper
.globl segment_not_present_error_wrapper
.globl stack_segment_fault_error_wrapper 
.globl gen_protection_error_wrapper
.globl page_fault_error_wrapper
.globl float_point_error_wrapper
.globl align_check_error_wrapper
.globl machine_check_error_wrapper
.globl simd_floating_point_error_wrapper

divide_error_wrapper:
    pushal 
    call divide_error
    popal 
    iret 

debug_error_wrapper:
    pushal
    call debug_error
    popal 
    iret 

nmi_interrupt_error_wrapper:
    pushal 
    call nmi_interrupt_error
    popal 
    iret 

breakpoint_error_wrapper:
    pushal 
    call breakpoint_error
    popal 
    iret 

overflow_error_wrapper:
    pushal 
    call overflow_error
    popal 
    iret 

bound_range_exceeded_error_wrapper:
    pushal 
    call bound_range_exceeded_error
    popal 
    iret 

invalid_opcode_error_wrapper:
    pushal 
    call invalid_opcode_error
    popal 
    iret 

device_not_available_error_wrapper:
    pushal 
    call device_not_available_error
    popal 
    iret 

dbl_fault_error_wrapper:
    pushal 
    call dbl_fault_error
    popal 
    iret 

coprocess_segment_overrun_error_wrapper:
    pushal 
    call coprocess_segment_overrun_error
    popal 
    iret 

invalid_tss_error_wrapper:
    pushal 
    call invalid_tss_error
    popal 
    iret 

segment_not_present_error_wrapper:
    pushal 
    call segment_not_present_error
    popal 
    iret 

stack_segment_fault_error_wrapper:
    pushal 
    call stack_segment_fault_error
    popal 
    iret 

gen_protection_error_wrapper:
    pushal 
    call gen_protection_error
    popal
    iret 

page_fault_error_wrapper:
    pushal 
    call page_fault_error
    popal 
    iret 

float_point_error_wrapper:
    pushal 
    call float_point_error
    popal 
    iret 

align_check_error_wrapper:
    pushal
    call align_check_error
    popal 
    iret 

machine_check_error_wrapper:
    pushal 
    call machine_check_error
    popal 
    iret 

simd_floating_point_error_wrapper:
    pushal 
    call simd_floating_point_error
    popal 
    iret 
    
