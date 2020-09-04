.globl irq_syscall

irq_syscall:
	pushl %ecx			
	pushl %edx
	pushl %ebx
	pushl %esi
	pushl %edi
	pushfl
	pushl %es
	pushl %ds

	cmpl $1, %eax	#checks if %eax is less than 1 no negative locations in disbatch 
	jl error				
	cmpl $10, %eax  #checks if %eax is exceeding the size of the sys_batch table 
	jg error	

	pushl %edx						#arg 2
	pushl %ecx						#arg 1 
	pushl %ebx						#arg 0 
	subl  $1, %eax						

	call *sys_disbatch(, %eax, 4)

	popl %ebx						#pop arg0
	popl %ecx						#pop arg1
	popl %edx						#pop arg2 
pop_args: 
	popl %ds					
	popl %es
	popfl
	popl %edi
	popl %esi
	popl %ebx
	popl %edx
	popl %ecx
	iret
error:
	popl %ds					
	popl %es
	popfl
	popl %edi
	popl %esi
	popl %ebx
	popl %edx
	popl %ecx
	movl $-1, %eax
	iret

sys_disbatch:
.long sys_halt_wrapper, sys_execute, sys_read, sys_write, sys_open, sys_close, sys_getargs, sys_vidmap, sys_set_handler, sys_sigreturn
.end
