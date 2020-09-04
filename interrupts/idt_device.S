.globl irq_keyboard
.globl irq_rtc
.globl irq_pit
.globl irq_exit 
.globl irq_mouse

irq_keyboard:

  pushal
  pushfl
  pushl %eax
  pushl %ecx
  pushl %edx
  
  call keyboard_handler
  
  popl %edx
  popl %ecx
  popl %eax
  popfl
  popal
  
  iret

irq_rtc:

  pushal
  pushfl
  pushl %eax
  pushl %ecx
  pushl %edx
  
  call rtc_handler
  
  popl %edx
  popl %ecx
  popl %eax
  popfl
  popal

  iret

irq_pit:

  pushal
  pushfl
  pushl %eax
  pushl %ecx
  pushl %edx
  
  call pit_handler
  
  popl %edx
  popl %ecx
  popl %eax
  popfl
  popal

  iret

irq_mouse:

  pushal
  pushfl
  pushl %eax
  pushl %ecx
  pushl %edx
  
  call mouse_handler
  
  popl %edx
  popl %ecx
  popl %eax
  popfl
  popal

  iret
