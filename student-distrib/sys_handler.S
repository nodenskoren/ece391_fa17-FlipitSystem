
.global sys_handler, interrupt_handler, rtc_wrapper, keyboard_wrapper, pit_wrapper
sys_handler:

	pushfl
	
	cmpl $1, %eax
	jb 	 num_invalid
	cmpl $8, %eax
	ja	 num_invalid
	
	addl $-1, %eax

	pushl %edx
	pushl %ecx
	pushl %ebx	
	
	call *jump(, %eax, 4)
	
	
	popl %ebx	
	popl %ecx		
	popl %edx
	
	popfl
			
	iret
	
	
num_invalid:
	
	popfl
	movl $-1, %eax
	iret
	
		
jump:
.long halt, execute, read, write, open, close, getargs, vidmap




# saves all registers and flags before rtc_handler, restores all registers and flags after
rtc_wrapper:
	pushal   # push all registers
	pushfl   # push all flags 
	call RTC_interrupt_handler   
	popfl    # pop all flags 
	popal    # pop all registers
	
	iret


# saves all registers and flags before terminal, restores all registers and flags after
keyboard_wrapper:
	cli
	pushal   # push all registers
	pushfl   # push all flags 
	call keyboard_interrupt_handler
	popfl    # pop all flags 
	popal    # pop all registers
	sti
	
	iret
	
# saves all registers and flags before terminal, restores all registers and flags after
pit_wrapper:
	cli
	
	pushal   # push all registers
	pushfl   # push all flags
	call scheduler
	popfl    # pop all flags 
	popal    # pop all registers
	
	sti
	
	iret

