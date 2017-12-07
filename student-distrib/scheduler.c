#include "scheduler.h"


//2->0 (1)

//0->1 (2)

//1->2 (0)

//= terminal1,terminal2,terminal3;
uint32_t esp;
uint32_t ebp;

void scheduler() {
	
	asm volatile(
		"movl %%esp, %0     \n \
		 movl %%ebp, %1"
		: "=r"(esp),"=r"(ebp)
		:
		: "memory"
	);

	
	terminal_num++;
	terminal_num = terminal_num % 3;
	
	if(terminal[terminal_num].current_pcb == NULL) {
		terminal[0].current_videomem = 0xB8000;
        terminal[1].current_videomem = 0xBA000;
        terminal[2].current_videomem = 0xBB000;
		terminal[terminal_num].esp = esp;
		terminal[terminal_num].ebp = ebp;
		send_eoi(0);
		execute((uint8_t *)"shell");
	}

	switch(terminal_num) {
		case 0:
		    terminal[0].esp = esp;
			terminal[0].ebp = ebp;
			/* need init function */
			user_page_init(terminal[0].current_pcb->pid);
			/* restore in assembly later */
			esp = terminal[1].esp;
			ebp = terminal[1].ebp;
			tss.esp0 = terminal[0].current_pcb->esp0;
			tss.ss0 = KERNEL_DS;
            break;			
			
		case 1:
			terminal[1].esp = esp;
			terminal[1].ebp = ebp;
			/* need init function */			
			user_page_init(terminal[1].current_pcb->pid);
			esp = terminal[2].esp;
			ebp = terminal[2].ebp;
			tss.esp0 = terminal[1].current_pcb->esp0;
			tss.ss0 = KERNEL_DS;
            break;			
			
		case 2:
			terminal[2].esp = esp;
			terminal[2].ebp = ebp;
			/* need init function */			
			user_page_init(terminal[2].current_pcb->pid);
			esp = terminal[0].esp;
			ebp = terminal[0].ebp;
			tss.esp0 = terminal[2].current_pcb->esp0;
			tss.ss0 = KERNEL_DS;
            break;			
	}
	
	
	
	asm volatile(
		"movl %0, %%esp     \n \
		movl %1, %%ebp"
		:
		: "r" (esp),"r" (ebp)
		: "memory"
	);
	send_eoi(0);
	
}

