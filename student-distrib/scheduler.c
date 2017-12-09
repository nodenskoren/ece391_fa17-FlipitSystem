#include "scheduler.h"

int32_t terminal_num = INITIAL_VALUE;
int32_t current_visible = 0;
uint32_t esp;
uint32_t ebp;
	

/*
 * scheduler()
 *		DESCRIPTION: 
 *					- Swaps between different active process in each terminals in each pit interval (quantum)
 *                    in a round-robin manner (0->1->2->0)
 *                  - Start one shell in each terminal during booting initialization
 *                  - Initialize the terminal_t structure in the first run
 *		INPUTS: none
 *		OUTPUTS: none
 *		SIDE EFFECT:
 *                  - All 3 terminal structures will be initialized
 *                  - terminal_num will be incremented in each quantum
 *                  - interrupts would be temporarily disabled when scheduler() is running
 *                  - %esp and %ebp values of the last terminal will be updated inside the esp and ebp
 *                    inside the current terminal structure.
 *
 */
void scheduler() {
	
	uint32_t flags;
	
	/* Disable the interrupts and set the flags */
	cli_and_save(flags);
	
	/* Retreive the esp and ebp register values and store them into local variables */
	asm volatile(
		"movl %%esp, %0     \n \
		 movl %%ebp, %1"
		: "=r"(esp),"=r"(ebp)
		:
		: "memory"
	);

	/* Initialize terminal structure if first time being called */
	if(terminal_num == INITIAL_VALUE){
		int i;
		for(i = MINIMUM_TERMINAL_INDEX; i <= MAXIMIM_TERMINAL_INDEX; i++){
			terminal[i].current_pcb = NULL;
			terminal[i].active_process = NULL;
			terminal[i].previous_process = NULL;
			terminal[i].esp = 0;
			terminal[i].ebp = 0;
			terminal[i].previous_buf_length = 0;
			terminal[i].buf_position = 0;
			terminal[i].screen_x = 0;
			terminal[i].screen_y = 0;
			terminal[i].command_ready_flag = -1;
			terminal[i].RTC_flag = 0;
		}
	}
	
	/* Increment the terminal number and truncate it down to 0 to 2 */
	terminal_num++;
	terminal_num = terminal_num % NUM_OF_TERMINAL;
	
	/* Launch one shell in each terminal during initialization */
	if(terminal[terminal_num].active_process == NULL) {
		
		/* Store the esp and ebp for each terminal */
		terminal[terminal_num].esp = esp;
		terminal[terminal_num].ebp = ebp;
		term_page_switch();
		restore_flags(flags);
		/* send end of interrupt signal to PIT port */
		send_eoi(PIT_IRQ_NUM);
		/* Launch shell */
		execute((uint8_t *)"shell");
	}

	switch(terminal_num) {
		
		case 0:
			/* Load the esp and ebp stack address for the last terminal */
		    terminal[0].esp = esp;
			terminal[0].ebp = ebp;
			/* Set the active process and setup its paging */
			user_page_init(terminal[0].active_process->pid);
			term_page_switch();
			/* Load the esp and ebp values for the current terminal into temporary variable */
			esp = terminal[1].esp;
			ebp = terminal[1].ebp;
			/* Setup the tss for the current terminal */						
			tss.esp0 = terminal[0].active_process->esp0;
			tss.ss0 = KERNEL_DS;
            break;			
			
		case 1:
			/* Save the esp and ebp stack address for the last terminal */
			terminal[1].esp = esp;
			terminal[1].ebp = ebp;
			/* Set the active process and setup its paging */
			user_page_init(terminal[1].active_process->pid);
			term_page_switch();
			/* Load the esp and ebp values for the current terminal into temporary variables */
			esp = terminal[2].esp;
			ebp = terminal[2].ebp;
			/* Setup the tss for the current terminal */						
			tss.esp0 = terminal[1].active_process->esp0;
			tss.ss0 = KERNEL_DS;
            break;			
			
		case 2:
			/* Save the esp and ebp stack address for last terminal */		
			terminal[2].esp = esp;
			terminal[2].ebp = ebp;
			/* Set the active process and setup its paging */
			user_page_init(terminal[2].active_process->pid);
			term_page_switch();
			/* Load the esp and ebp values for the current terminal into temporary variables */
			esp = terminal[0].esp;
			ebp = terminal[0].ebp;
			/* Setup the tss for the current terminal */			
			tss.esp0 = terminal[2].active_process->esp0;
			tss.ss0 = KERNEL_DS;
            break;			
	}
	
	
	/* Setup the esp and ebp for the current terminal */	
	asm volatile(
		"movl %0, %%esp     \n \
		movl %1, %%ebp"
		:
		: "r" (esp),"r" (ebp)
		: "memory"
	);
	
	/* Restore the flag and enable interrupts again */
	restore_flags(flags);
	/* Send end of interrupt signal to pit port */
	send_eoi(PIT_IRQ_NUM);
	
}

	

