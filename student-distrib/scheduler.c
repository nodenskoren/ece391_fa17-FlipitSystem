#include "scheduler.h"


//2->0 (1)

//0->1 (2)

//1->2 (0)

//= terminal1,terminal2,terminal3;

int32_t terminal_num = -1;
int32_t current_visible = 0;
uint32_t esp;
uint32_t ebp;

void initalize_terminals();

void initalize_terminals(){
	int i;
	for(i=0;i<3;i++){
    terminal[i].current_pcb=NULL;
	terminal[i].active_process=NULL;
	terminal[i].previous_process=NULL;
	terminal[i].esp=0;
	terminal[i].ebp=0;
	terminal[i].previous_buf_length=0;
	terminal[i].buf_position=0;
	terminal[i].screen_x=0;
	terminal[i].screen_y=0;
	terminal[i].command_ready_flag = -1;
	terminal[i].RTC_flag = 0;
	}
}
	

void scheduler() {
	
	uint32_t flags;
	cli_and_save(flags);
	
	
	asm volatile(
		"movl %%esp, %0     \n \
		 movl %%ebp, %1"
		: "=r"(esp),"=r"(ebp)
		:
		: "memory"
	);

	if(terminal_num==-1){
			initalize_terminals();
		}
	terminal_num++;
	terminal_num = terminal_num % 3;
	
	if(terminal[terminal_num].current_pcb == NULL) {
		
		terminal[terminal_num].esp = esp;
		terminal[terminal_num].ebp = ebp;
		term_page_switch();
		restore_flags(flags);
		send_eoi(0);
		execute((uint8_t *)"shell");
	}

	switch(terminal_num) {
		case 0:
		    terminal[0].esp = esp;
			terminal[0].ebp = ebp;
			/* need init function */
			user_page_init(terminal[0].active_process->pid);
			term_page_switch();
			/* restore in assembly later */
			esp = terminal[1].esp;
			ebp = terminal[1].ebp;
			tss.esp0 = terminal[0].active_process->esp0;
			tss.ss0 = KERNEL_DS;
            break;			
			
		case 1:
			terminal[1].esp = esp;
			terminal[1].ebp = ebp;
			/* need init function */			
			user_page_init(terminal[1].active_process->pid);
			term_page_switch();
			esp = terminal[2].esp;
			ebp = terminal[2].ebp;
			tss.esp0 = terminal[1].active_process->esp0;
			tss.ss0 = KERNEL_DS;
			//execute((uint8_t*)"cat frame0.txt");
            break;			
			
		case 2:
			terminal[2].esp = esp;
			terminal[2].ebp = ebp;
			/* need init function */			
			user_page_init(terminal[2].active_process->pid);
			term_page_switch();
			esp = terminal[0].esp;
			ebp = terminal[0].ebp;
			tss.esp0 = terminal[2].active_process->esp0;
			tss.ss0 = KERNEL_DS;
			//execute((uint8_t*)"ls");			
            break;			
	}
	
	
	
	asm volatile(
		"movl %0, %%esp     \n \
		movl %1, %%ebp"
		:
		: "r" (esp),"r" (ebp)
		: "memory"
	);
	restore_flags(flags);
	send_eoi(0);
	
}

	

