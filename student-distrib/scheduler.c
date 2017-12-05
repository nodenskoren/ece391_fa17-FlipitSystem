#include "scheduler.h"


//2->0 (1)

//0->1 (2)

//1->2 (0)




int32_t terminal_num = -1;

uint32_t esp = 0;
//= terminal1,terminal2,terminal3;

void scheduler() {
	
	//inline save %esp here
	asm volatile(
		"movl %%esp, %0"
		: "=r"(esp)
		:
		: "memory"
	);
	
	terminal_num++;
	terminal_num = terminal_num % 3;
	
	if(terminal[terminal_num].current_pcb == NULL) {
		terminal[terminal_num].esp = esp;
		execute((uint8_t *)"shell");
	}

	switch(terminal_num) {
		case 0:
		    terminal[0].esp = esp;
			/* need init function */
			user_page_init(terminal[0].current_pcb->pid);
			/* restore in assembly later */
			esp = terminal[1].esp;
			tss.esp0 = terminal[0].current_pcb->esp0;
			tss.ss0 = KERNEL_DS;	
			
		case 1:
			terminal[1].esp = esp;
			/* need init function */			
			user_page_init(terminal[1].current_pcb->pid);
			esp = terminal[2].esp;
			tss.esp0 = terminal[1].current_pcb->esp0;
			tss.ss0 = KERNEL_DS;	
			
		case 2:
			terminal[2].esp = esp;
			/* need init function */			
			user_page_init(terminal[2].current_pcb->pid);
			esp = terminal[0].esp;
			tss.esp0 = terminal[2].current_pcb->esp0;
			tss.ss0 = KERNEL_DS;				
	}
	
	asm volatile(
		"movl %%eax, %%esp"
		:
		: "a" (esp)
		: "memory"
	);
	
	asm volatile(
		"iret"
	);
}

