#include "systemcall.h"
#include "types.h"
#include "filesystem.h"
#include "paging.h"
#include "terminal_driver.h"
#include "RTC_driver.h"
#include "keyboard_handler.h"
#include "x86_desc.h"
#include "lib.h"

//#define 8MB_PAGE 0x800000
//#define 8KB_PAGE 0x2000
#define Stdin_FD 0
#define Stdout_FD 1
#define file_length 32
#define cmd_length  32
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

static int ret_val =0;
pcb_t* active_process = NULL;
pcb_t* previous_process = NULL;
uint32_t pid_array[8];

/* jump tables for different operations */
int32_t stdin_jmp_table[NUM_OF_OP] = {0, (int32_t)terminal_read, 0, 0};
int32_t stdout_jmp_table[NUM_OF_OP] = {0, 0, (int32_t)terminal_write, 0};
int32_t rtc_jmp_table[NUM_OF_OP] = {(int32_t)RTC_open, (int32_t)RTC_read, (int32_t)RTC_write, (int32_t)RTC_close};
int32_t file_jmp_table[NUM_OF_OP] = {(int32_t)regular_file_open, (int32_t)regular_file_read, (int32_t)regular_file_write, (int32_t)regular_file_close};
int32_t dir_jmp_table[NUM_OF_OP] = {(int32_t)directory_file_open, (int32_t)directory_file_read, (int32_t)directory_file_write, (int32_t)directory_file_close};	

/* type define the open functions */
typedef int32_t (*open_t)(char*);

/* type define the read functions */
typedef int32_t (*read_t)(int32_t, void*, int32_t);

/* type define the write functions */
typedef int32_t (*write_t)(int32_t, void*, int32_t);

/* type define the close functions */
typedef int32_t (*close_t)(int32_t);

void stdin_init(int32_t fd) {
	active_process->file_descriptor_table[fd].f_op = stdin_jmp_table;
	active_process->file_descriptor_table[fd].active = ACTIVE;
}

void stdout_init(int32_t fd) {
	active_process->file_descriptor_table[fd].f_op = stdout_jmp_table;
	active_process->file_descriptor_table[fd].active = ACTIVE;
}


/* 
 * execute
 *		DESCRIPTION: executes an executable file. First reads file into correct memory using paging and creates 
 *                   process control block onto the process specific kernel stack.
 *		INPUTS: command - the name of the executable file followed by some arguments
 *		OUTPUTS: none
 *		SIDE EFFECT: Push arguments necessary to execute in user space
 *
 */

int32_t execute(const uint8_t * command){

/***************************************************/
/*   The following is checking the file validity   */ 
/***************************************************/

  //printf("%s\n", command);

  // check if command is null
  if(command == NULL)
  return -1;

  // check if filename makes sense
  if(command[0] == '\0' || command[0] == ' ' || command[0] == '\t')
  {
    puts("Exception: illegal file name!\n");
    return -1;
  }

  // allocate a filename buffer
  uint8_t filename_buf [file_length];
  int i = 0;

  
  // loop through the command to get the file name
  while(command[i] != ' ' && i < file_length)
  {
    filename_buf[i] = command[i];
    i++;
  }
  // if our filename doesnt end with space, the file name is illegal
 /* if(command[i] != ' ')
  {
    puts("Exception: illegal file name!\n");
    return -1;
  }
*/
  //printf("%s\n", command);
  
  
  // try to read
  dentry_t dentry;
  if ( read_dentry_by_name (filename_buf, &(dentry)) == -1 )
  {
    puts("Exception: illegal file name!\n");
    return -1;
  }

  // check if it is excecutable
  uint8_t temp_read[executable_check_length];
  read_data(dentry.inode,0, temp_read, executable_check_length);
  
  // check if magic numbers for exe file is correct
  if(temp_read[0] != 0x7f || temp_read[1] != 0x45 || temp_read[2] != 0x4c || temp_read[3] != 0x46)
  {
    puts("Exception: Not an executable file!\n");
    return -1;
  }
 
  /***************************************************/
  /*        The following is Creating PCB/FDs        */ 
  /***************************************************/

 
  // loop over the pid array to find the empty entry

  //printf("esp = %x\n", esp);
  //printf("ebp = %x\n", ebp);
  
  
  i = 0;
  while(pid_array[i] == 1 && i< pid_size){
	  i++;
  }
  if (i < pid_size)
	pid_array[i] = 1;
  else 
  {
	puts("Exception: too many processes!\n");
	return -1;
  }
  // store pcb to 8MB - (pid+1) * 8kb
  pcb_t* current_pcb = (pcb_t*)(eightmeg_page - (i + 1) * eightkilo_page);
  current_pcb->pid = i; 
  
  // open FDs, update the global process info
  // root case
	if(active_process == NULL) {
		//printf("1\n");
		active_process = current_pcb;
		active_process->parent_pcb == NULL;
	}
	
	else{
		//printf("2\n");
		previous_process = active_process;
		active_process = current_pcb;
		active_process->parent_pcb = previous_process;
	}
	
	//active_process = current_pcb;

  
  stdin_init(Stdin_FD);
  stdout_init(Stdout_FD);
 
  // setting up paging 
  user_page_init(current_pcb->pid);
  //printf("pid: %d\n", current_pcb->pid);
  
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
  
  /* bug */
    // see if we need to store parent pcb pointer
  if(active_process->parent_pcb != NULL){
	  //printf("output I\n");
	  //current_pcb->parent_pcb = (pcb_t*)(eightmeg_page - (active_process->pid) * eightkilo_page );
	  current_pcb->esp = esp;
	  current_pcb->ebp = ebp;
  }
  
  else{
	  //printf("output II\n");
	  current_pcb->parent_pcb = NULL;
  }
  
 
  // loads the file into memory
  read_data(dentry.inode,0,(uint8_t*)file_start, (four_mb-file_offset));  
 
  // gets the entry point from the file
  uint32_t entry_point = 0;
  uint8_t buf[entry_point_length];
  read_data(dentry.inode, entry_point_start, buf, 4);
  entry_point = (buf[3] << 24) | (buf[2] << 16) | (buf[1] << 8) | buf[0]; 

  // update tss before iret
  tss.esp0 = (eightmeg_page - i * eightkilo_page - 4);
  tss.ss0 = KERNEL_DS;
  current_pcb->esp0 = tss.esp0;
  current_pcb->ss0	= tss.ss0;
  //active_process = current_pcb;
  
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
 *		OUTPUTS: none
 *		SIDE EFFECT: none
 *
 */
int32_t read(int32_t fd, void* buf, int32_t nbytes) {
	/* If the entry has not been opened, invalidate the call */
	if(active_process->file_descriptor_table[fd].active != ACTIVE) {
		return FAILURE;
	}
	/* If the fd given is not within the range, the buffer doesn't exist, or the bytes is not in a valid range */
	/* return FAILURE */
	if(fd < 0 || fd > 7 || buf == NULL || nbytes < 0) {
		return FAILURE;
	}
	/* Apply the specific read for the given file type */
	int32_t* jmp_table = active_process->file_descriptor_table[fd].f_op;
	read_t read_func = (read_t)jmp_table[READ];
	int32_t bytes_read = read_func(fd, buf, nbytes);
	active_process->file_descriptor_table[fd].f_offset += bytes_read;
	return bytes_read;
}

/* 
 * write
 *		DESCRIPTION: Utilize the jump table of the specified file type  
 *		INPUTS: fd (the desired index of the file descriptor table to write the buffer to)
 *              buf (buffer containing the contents to write to file)
 *              nbytes (how many bytes to write)
 *		OUTPUTS: none
 *		SIDE EFFECT: none
 *
 */
int32_t write(int32_t fd, void* buf, int32_t nbytes) {
	/* If the entry has not been opened, invalidate the call */
	if(active_process->file_descriptor_table[fd].active != ACTIVE) {
		return FAILURE;
	}
	/* If the fd given is not within the range, the buffer doesn't exist, or the bytes is not in a valid range */
	/* return FAILURE */
	if(fd < 0 || fd > 7 || buf == NULL || nbytes < 0) {
		return FAILURE;
	}
	
	/* Apply the specific write for the given file type */
	int32_t* jmp_table = active_process->file_descriptor_table[fd].f_op;	
	write_t write_func = (write_t)jmp_table[WRITE];
	write_func(fd, buf, nbytes);
	return SUCCESS;
}

/* 
 * open
 *		DESCRIPTION: Open a file at a free entry inside the file descriptor table
 *		INPUTS: filename (desided filename to open)
 *		OUTPUTS: none
 *		SIDE EFFECT: A new entry will be turned active and associated to
 *                   the new file if there file descriptor table is not fully loaded.
 *
 */
int32_t open(const uint8_t* filename) {
	//printf("FILE_OPEN!_1");
	pcb_t* curr_process = active_process;
	file_t* fds = curr_process->file_descriptor_table;
	dentry_t d_entry;

	/* Find an empty entry inside the file descriptor table */
	uint32_t fd = 0;
	while(fds[fd].active != 0) {
		fd++;
	}
	//printf("FILE_OPEN!_2");

	/* If file descriptor table is fully loaded already, return failure */
	if(fd <= 7 && read_dentry_by_name(filename, &d_entry) >= 0) {
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
				return SUCCESS;
			/* Directory */
			case DIRECTORY_FILE: 
				fds[fd].f_op = dir_jmp_table;
				fds[fd].inode = NULL;
				fds[fd].f_offset = 0;
				fds[fd].active = ACTIVE;
				//fds[fd].fname = filename;
				directory_file_open();
				return SUCCESS;
			/* Regular file */
			case REGULAR_FILE:
				fds[fd].f_op = file_jmp_table;
				fds[fd].inode = d_entry.inode;
				fds[fd].f_offset = 0;
				fds[fd].active = ACTIVE;
				//fds[fd].fname = filename;
				regular_file_open(filename);
				return SUCCESS;

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
	/* Turn off the active flag of the entry specified by fd */
	active_process->file_descriptor_table[fd].active = INACTIVE;
	return 0;
}

/* 
 * halt
 *		DESCRIPTION: Halt the current process and goes back to parent process
 *		INPUTS: status - return value from user containing error info
 *		OUTPUTS: none
 *		SIDE EFFECT: Push esp and ebp onto stack 
 *
 */

int32_t halt (uint8_t status){
	ret_val = status;
	int is_shell = 0;
	//uint32_t retval = (uint32_t)status;
	uint32_t fd = 0;
	// close any relevant FDs
	
	while(active_process->file_descriptor_table[fd].active == ACTIVE && fd < 2) {
		active_process->file_descriptor_table[fd].active = INACTIVE;		
		fd++;
	}
	
	
	
	// restore parent paging
	if(active_process->parent_pcb != NULL)
	{	
		//printf("Output 1\n");
		user_page_init(active_process->parent_pcb->pid);
		//printf("Output 3\n");
		// change the tss esp0 to point to the bottom of 8kb kernel process, the kernel stack
		tss.esp0 = active_process->parent_pcb->esp0;
		//printf("Output 4\n");
		tss.ss0 = active_process->parent_pcb->ss0;
		//printf("Output 5\n");
		pid_array[active_process->pid] =0;
		//printf("Output 6\n");
	}
	else
	{
		printf("Output 2\n");
		is_shell =1;
		pid_array[0] = 0;
	}
	// restore parent's pcb
	


	
	

	if(is_shell == 1)
	{
		//printf("Output 7\n");
		active_process = NULL;
		printf("restarting the shell\n");
		execute((uint8_t *)"shell");
	}
	// jump to execute return
	
	uint32_t esp = active_process->esp;
			//printf("Output 8\n");

	uint32_t ebp = active_process->ebp;
			//printf("Output 9\n");

	active_process = active_process->parent_pcb;
		//printf("Output 10\n");
	
	
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

int32_t vidmap(uint8_t** screen_start) {
	if((uint32_t)screen_start < _128MB || (uint32_t)screen_start >= (_128MB + _4MB)) {
		return -1;
	}
	
	*screen_start = (uint8_t*)USER_VID_MAP;
	return 0;
}

