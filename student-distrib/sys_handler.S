
.global sys_handler
sys_handler:


	pushfl
	
	cmpl $1, %eax
	jb 	 num_invalid
	cmpl $6, %eax
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
	popal
	movl $-1, %eax
	iret
	
		
jump:
.long halt, execute, read, write, open, close	



