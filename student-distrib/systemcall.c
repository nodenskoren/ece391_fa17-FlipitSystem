#include "systemcall.h"
#include "types.h"
#include "filesystem.h"
#include "paging.h"
#include "terminal_driver.h"
#include "RTC_driver.h"
#include "keyboard_handler.h"
#include "x86_desc.h"
#include "lib.h"
#include "scheduler.h"

#define Stdin_FD 0
#define Stdout_FD 1
#define file_length 32
#define cmd_length  128
#define file_offset 0x00048000
#define file_start 0x08048000
#define eightmeg_page 0x800000
#define eightkilo_page 0x2000
#define entry_point_length 4
#define executable_check_length 32
#define bitmap_size 8
#define pid_size 8
#define entry_point_start 24
#define _128MB 0x08000000
#define _4MB 0x00400000
#define	USER_VID 0x07CB8000
#define four_bytes 4

static int ret_val = 0;
uint32_t pid_array[PROCESS_MAX];

/* jump tables for different operations */
int32_t stdin_jmp_table[NUM_OF_OP] = {0, (int32_t)terminal_read, 0, 0};
int32_t stdout_jmp_table[NUM_OF_OP] = {0, 0, (int32_t)terminal_write, 0};
int32_t rtc_jmp_table[NUM_OF_OP] = {(int32_t)RTC_open, (int32_t)RTC_read, (int32_t)RTC_write, (int32_t)RTC_close};
int32_t file_jmp_table[NUM_OF_OP] = {(int32_t)regular_file_open, (int32_t)regular_file_read, (int32_t)regular_file_write, (int32_t)regular_file_close};
int32_t dir_jmp_table[NUM_OF_OP] = {(int32_t)directory_file_open, (int32_t)directory_file_read, (int32_t)directory_file_write, (int32_t)directory_file_close};


/*
 * stdin_init
 *		DESCRIPTION: open an stdin file in the given fd slot
 *		INPUTS: fd - the fd slot to open stdin file in
 *		OUTPUTS: none
 *		SIDE EFFECT: the given fd slot would be occupied after the call
 *
 */
void stdin_init(int32_t fd) {
	terminal[terminal_num].active_process->file_descriptor_table[fd].f_op = stdin_jmp_table;
	terminal[terminal_num].active_process->file_descriptor_table[fd].active = ACTIVE;
}

/*
 * stdout_init
 *		DESCRIPTION: open an stdout file in the given fd slot
 *		INPUTS: fd - the fd slot to open stdout file in
 *		OUTPUTS: none
 *		SIDE EFFECT: the given fd slot would be occupied after the call
 *
 */
void stdout_init(int32_t fd) {
	terminal[terminal_num].active_process->file_descriptor_table[fd].f_op = stdout_jmp_table;
	terminal[terminal_num].active_process->file_descriptor_table[fd].active = ACTIVE;
}


/*
 * execute
 *		DESCRIPTION: executes an executable file. First reads file into correct memory using paging and creates
 *                   process control block onto the process specific kernel stack.
 *		INPUTS: command - the name of the executable file followed by some arguments
 *		OUTPUTS: -1 if failure, 0 if success
 *		SIDE EFFECT: Push arguments necessary to execute in user space
 *
 */
int32_t execute(const uint8_t * command){

	/***************************************************/
	/*   The following is checking the file validity   */
	/***************************************************/

	cli();

	// check if command is null
	if(command == NULL)
		return FAILURE;
	// check if filename makes sense
	if(command[0] == '\0' || command[0] == '\n' || command[0] == '\t'|| command[0] == '\r') {
		return FAILURE;
	}
  
	// allocate a filename buffer
	//uint8_t filename_buf [file_length+1];
	uint8_t filename_buf[MAX_BUF_LENGTH];
	uint8_t arg_buff [cmd_length];
	int i = 0;
	int j = 0;
	// strip off the leading spaces
	while(command[i] == ' ') {
		i++;
	}

	// check again if our command is valid
	if(command[i] == '\0' || command[i] == '\n' || command[i] == '\t'|| command[i] == '\r') {
		return FAILURE;
	}

	// loop through the command to get the file name
	while(command[i] != '\0' && command[i] != '\n' && command[i] != '\t'&& command[i] != '\r' && command[i] != ' ' ) {
		filename_buf[j] = command[i];
		i++;
		j++;
	}
  
	// if our filename doesnt end with space, the file name is illegal
	if(j> file_length) {
		return FAILURE;
	}
	// otherwise we just add a end of string at the end of our filename
	else
		filename_buf[j] = '\0';

	// strip off the leading spaces
	while(command[i] == ' ') {
		i++;
	}

	// get arguments into our pcb
	j = 0;
	while(command[i] != '\0' && command[i] != '\n' && command[i] != '\t'&& command[i] != '\r' && i<cmd_length) {
		arg_buff[j] = command[i];
		i++;
		j++;
	}
	arg_buff[j] = '\0';

	// try to read
	dentry_t dentry;
	if (read_dentry_by_name (filename_buf, &(dentry)) == -1 ) {
		return FAILURE;
	}

	// check if it is excecutable
	uint8_t temp_read[executable_check_length];
	read_data(dentry.inode,0, temp_read, executable_check_length);

	// check if magic numbers for exe file is correct
	if(temp_read[0] != 0x7f || temp_read[1] != 0x45 || temp_read[2] != 0x4c || temp_read[3] != 0x46) {
		return FAILURE;
	}

	/***************************************************/
	/*        The following is Creating PCB/FDs        */
	/***************************************************/
	
	// loop over the pid array to find the empty entry
	i = 0;
	while(pid_array[i] == 1 && i< pid_size){
		i++;
	}
	if (i < pid_size)
		pid_array[i] = 1;
	else {
		puts("Exception: too many processes!\n");
		return FAILURE;
	}
	// store pcb to 8MB - (pid+1) * 8kb
	pcb_t* current_pcb2 = (pcb_t*)(eightmeg_page - (i + 1) * eightkilo_page);
	current_pcb2->pid = i;
	terminal[terminal_num].current_pcb = current_pcb2;
	int l=0;
	while(arg_buff[l] != '\0'){
		terminal[terminal_num].current_pcb->arg[l] = arg_buff[l];
		l++;
	}
	terminal[terminal_num].current_pcb->arg[l] = '\0';

	// open FDs, update the global process info
	// root case
	if(terminal[terminal_num].active_process == NULL) {
		terminal[terminal_num].active_process = terminal[terminal_num].current_pcb;
	}

	/* If not, set the parent before doing the process switch */
	else{
		terminal[terminal_num].previous_process = terminal[terminal_num].active_process;
		terminal[terminal_num].active_process = terminal[terminal_num].current_pcb;
		terminal[terminal_num].active_process->parent_pcb = terminal[terminal_num].previous_process;
	}

	// initialize stdin and stdout in fd 0 and 1
	stdin_init(Stdin_FD);
	stdout_init(Stdout_FD);

	// setting up paging
	user_page_init(terminal[terminal_num].active_process->pid);
	cli();
	// store esp and ebp into pcb
	uint32_t esp = 0;
	uint32_t ebp = 0;
	asm volatile(
		"movl %%esp, %0		\n \
		 movl %%ebp, %1"
		 : "=r"(esp), "=r"(ebp)
		 :
		 : "memory"
	);

    // see if we need to store parent pcb pointer
	if(terminal[terminal_num].active_process->parent_pcb != NULL){
		terminal[terminal_num].active_process->esp = esp;
		terminal[terminal_num].active_process->ebp = ebp;
	}

	else{
		terminal[terminal_num].active_process->parent_pcb = NULL;
	}
	
	// loads the file into memory
	read_data(dentry.inode,0,(uint8_t*)file_start, (four_mb-file_offset));

	// gets the entry point from the file
	uint32_t entry_point = 0;
	uint8_t buf[entry_point_length];
	read_data(dentry.inode, entry_point_start, buf, four_bytes);
	entry_point = (buf[3] << FOURTH_WORD) | (buf[2] << THIRD_WORD) | (buf[1] << SECOND_WORD) | buf[0];

	// update tss before iret
	tss.esp0 = (eightmeg_page - i * eightkilo_page - four_bytes);
	tss.ss0 = KERNEL_DS;
	terminal[terminal_num].active_process->esp0 = tss.esp0;
	terminal[terminal_num].active_process->ss0	= tss.ss0;

	asm volatile (
		"pushl %%eax			\n \
		 pushl %%ebx			\n \
		 pushfl                         \n \
		 popl %%ebx                     \n \
		 orl $0x00000200,%%ebx          \n \
		 pushl %%ebx                    \n \
		 pushl %%ecx			\n \
		 pushl %%edx			\n \
		 iret                     \n \
		 EXECUTE_RETURN:"
		:
		: "a"(USER_DS), "b"(USER_ESP), "c"(USER_CS), "d"(entry_point)
		: "memory");

	return ret_val;
}


/*
 * read
 *		DESCRIPTION: Utilize the jump table of the specified file type
 *		INPUTS: fd (the desired index of the file descriptor table to read the file from)
 *              buf (buffer containing the contents to read to file)
 *              nbytes (how many bytes to read)
 *		OUTPUTS: -1 if FAILURE, 0 if SUCCESS
 *		SIDE EFFECT: fd's file tracker will get incremented
 *
 */
int32_t read(int32_t fd, void* buf, int32_t nbytes) {

	/* Error checking start */
	/* If the fd given is not within the range, the buffer doesn't exist, or the bytes is not in a valid range */
	/* return FAILURE */
	if(fd < FD_MIN || fd > FD_MAX || buf == NULL || nbytes < 0) {
		return FAILURE;
	}
	/* If the entry has not been opened, invalidate the call */
	if(terminal[terminal_num].active_process->file_descriptor_table[fd].active != ACTIVE) {
		return FAILURE;
	}

	/* stdout in read is illegal */
	if(fd == FD_STDOUT)
		return FAILURE;

	/* Error checking ends */
	return read_func(fd, buf, nbytes);
}

/*
 * write
 *		DESCRIPTION: Utilize the jump table of the specified file type
 *		INPUTS: fd (the desired index of the file descriptor table to write the buffer to)
 *              buf (buffer containing the contents to write to file)
 *              nbytes (how many bytes to write)
 *		OUTPUTS: -1 if FAILURE, 0 if SUCCESS
 *		SIDE EFFECT: none
 *
 */
int32_t write(int32_t fd, void* buf, int32_t nbytes) {

	/* Error checking start */
	/* If the fd given is not within the range, the buffer doesn't exist, or the bytes is not in a valid range */
	/* return FAILURE */
	if(fd < FD_MIN || fd > FD_MAX || buf == NULL || nbytes < 0) {
		return FAILURE;
	}

	/* If the entry has not been opened, invalidate the call */
	if(terminal[terminal_num].active_process->file_descriptor_table[fd].active != ACTIVE) {
		return FAILURE;
	}

	/* stdin in write is illegal */
	if(fd == FD_STDIN)
		return FAILURE;

	/* Error checking ends */

	/* Apply the specific write for the given file type */
	return write_func(fd, buf, nbytes);
}

/*
 * open
 *		DESCRIPTION: Open a file at a free entry inside the file descriptor table
 *		INPUTS: filename (desided filename to open)
 *		OUTPUTS: -1 if FAILURE, fd number if SUCCESS
 *		SIDE EFFECT: A new entry will be turned active and associated to
 *                   the new file if there file descriptor table is not fully loaded.
 *
 */
int32_t open(const uint8_t* filename) {
	pcb_t* curr_process = terminal[terminal_num].active_process;
	file_t* fds = curr_process->file_descriptor_table;
	dentry_t d_entry;

	/* Find an empty entry inside the file descriptor table */
	uint32_t fd = 0;
	while(fds[fd].active != INACTIVE) {
		fd++;
	}

	/* If file descriptor table is fully loaded already, return failure */
	if(fd <= FD_MAX && read_dentry_by_name(filename, &d_entry) >= NOT_END_OF_FILE_OR_FAILURE) {
		//printf("FILE_OPEN!_3");
		/* Set the activate flag of the fd entry and specify the corresponding operation sets */
		switch(d_entry.file_type) {
			/* RTC */
			case RTC_FILE:
 				fds[fd].f_op = rtc_jmp_table;
				fds[fd].inode = NULL;
				fds[fd].f_offset = 0;
				fds[fd].active = ACTIVE;
				//fds[fd].fname = filename;
				RTC_open(filename);
				return fd;
			/* Directory */
			case DIRECTORY_FILE:
				fds[fd].f_op = dir_jmp_table;
				fds[fd].inode = NULL;
				fds[fd].f_offset = 0;
				fds[fd].active = ACTIVE;
				//fds[fd].fname = filename;
				directory_file_open();
				return fd;
			/* Regular file */
			case REGULAR_FILE:
				fds[fd].f_op = file_jmp_table;
				fds[fd].inode = d_entry.inode;
				fds[fd].f_offset = 0;
				fds[fd].active = ACTIVE;
				//fds[fd].fname = filename;
				regular_file_open(filename);
				return fd;

			default: // Invalid type
				return FAILURE;
		}
	}
	return FAILURE;
}

/*
 * close
 *		DESCRIPTION: Change the flag of the file specified by the fd to inactive
 *		INPUTS: fd (the desired index of the file descriptor table to close)
 *		OUTPUTS: none
 *		SIDE EFFECT: Corresponding entry of the file descriptor table will be marked inactive.
 *
 */
int32_t close(int32_t fd) {

	/* If the fd given is not within the range, the buffer doesn't exist, or the bytes is not in a valid range */
	/* return FAILURE */
	if(fd < FD_MIN || fd > FD_MAX) {
		return FAILURE;
	}

	if(fd == FD_STDIN || fd == FD_STDOUT)
		return FAILURE;

	/* If the entry has not been opened, invalidate the call */
	if(terminal[terminal_num].active_process->file_descriptor_table[fd].active == INACTIVE) {
		return FAILURE;
	}

	/* Turn off the active flag of the entry specified by fd */
	terminal[terminal_num].active_process->file_descriptor_table[fd].active = INACTIVE;
	return 0;
}

/*
 * read_func
 *		DESCRIPTION: Call the file-specific read function
 *		INPUTS: fd (the desired index of the file descriptor table to write the buffer to)
 *              buf (buffer containing the contents to write to file)
 *              nbytes (how many bytes to write)
 *		OUTPUTS: return the read function result
 *		SIDE EFFECT: none
 *
 */
int32_t read_func(int32_t fd, void* buf, int32_t nbytes) {
	read_t read_function = (read_t)terminal[terminal_num].active_process->file_descriptor_table[fd].f_op[READ];
	return read_function(fd, buf, nbytes);
}

/*
 * write_func
 *		DESCRIPTION: Call the file-specific write function
 *		INPUTS: fd (the desired index of the file descriptor table to write the buffer to)
 *              buf (buffer containing the contents to write to file)
 *              nbytes (how many bytes to write)
 *		OUTPUTS: return the write function result
 *		SIDE EFFECT: none
 *
 */
int32_t write_func(int32_t fd, void* buf, int32_t nbytes) {
	write_t write_function = (write_t)terminal[terminal_num].active_process->file_descriptor_table[fd].f_op[WRITE];
	return write_function(fd, buf, nbytes);
}

/*
 * halt
 *		DESCRIPTION: Halt the current process and goes back to parent process
 *		INPUTS: status - return value from user containing error info
 *		OUTPUTS: 0. Always success.
 *		SIDE EFFECT: Push esp and ebp onto stack
 *
 */
int32_t halt (uint8_t status){
	
	ret_val = status;
	int is_shell = 0;
	uint32_t fd = 2;
	// close any relevant FDs
	while(terminal[terminal_num].active_process->file_descriptor_table[fd].active == ACTIVE) {
		 close(fd);
		 fd++;
	}

	// restore parent paging
	if(terminal[terminal_num].active_process->parent_pcb != NULL) {
		user_page_init(terminal[terminal_num].active_process->parent_pcb->pid);
		// change the tss esp0 to point to the bottom of 8kb kernel process, the kernel stack
		tss.esp0 = terminal[terminal_num].active_process->parent_pcb->esp0;
		tss.ss0 = terminal[terminal_num].active_process->parent_pcb->ss0;
		pid_array[terminal[terminal_num].active_process->pid] =0;
		int i = 0;
		for(i=0; i< cmd_length; i++) {
			terminal[terminal_num].active_process->parent_pcb->arg[i] = '\0';
		}
	}
	// otherwise if the process is the first shell
	else {
		is_shell =1;
		pid_array[0] = 0;
	}

	/* If the current process is the first shell, restart the shell after halt */
	if(is_shell == 1) {
		terminal[terminal_num].active_process = NULL;
		printf("restarting the shell\n");
		execute((uint8_t *)"shell");
	}
	// jump to execute return
	cli();

	/* Save the esp and ebp return address into local variables */
	uint32_t esp = terminal[terminal_num].active_process->esp;
	uint32_t ebp = terminal[terminal_num].active_process->ebp;

	/* Set the active process back to parent process (restore the parent's pcb) */
	terminal[terminal_num].active_process = terminal[terminal_num].active_process->parent_pcb;

	/* context switch */
	asm volatile(
	 "movl %%eax, %%esp			\n \
	  movl %%ecx, %%ebp			\n \
	  jmp  EXECUTE_RETURN"
	 :
	 : "a" (esp), "c"(ebp)
	 : "memory"



	);

	return 0;
}

/*
 * vidmap
 *		DESCRIPTION: Assign a preset virtual address to the user program containing the data desired
 *                   to be printed out, and map the data to the video memory's physical address (0xB8000)
 *		INPUTS: screen_start - contains the virtual address of the data the screen should start printing
 *		OUTPUTS: -1 if failure, 0 if success
 *		SIDE EFFECT: A page directory & page table set would be initialized and map to the video memory
 *
 */
int32_t vidmap(uint8_t** screen_start) {
	/* Boundary check */
	if((uint32_t)screen_start < _128MB || (uint32_t)screen_start >= (_128MB + _4MB)) {
		return FAILURE;
	}

	/* Set the screen_start address to the preset virtual address chosen by the designer */
	*screen_start = (uint8_t*)USER_VID_MAP;
	return SUCCESS;
}

/*
 * getargs
 *		DESCRIPTION: Reads the program's command line arguments into a user-level buffer.
 *		INPUTS: buf - the user-level buffer
                nbytes - number of bytes to be copied to the buffer
 *		OUTPUTS: none
 *		SIDE EFFECT: none
 *
 */
int32_t getargs (uint8_t* buf, int32_t nbytes){
		if (buf == NULL || nbytes<=0)
			return -1;
		pcb_t* curr_process = terminal[terminal_num].active_process;
		int32_t pcb_arg_length = strlen((int8_t*)curr_process->arg);
		if (pcb_arg_length == 0)
			return -1;
		else if( pcb_arg_length<= nbytes)
			memcpy(buf, curr_process->arg, pcb_arg_length+1);
		else
			memcpy(buf, curr_process->arg, nbytes);

		return 0;
}
