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


pcb_t* active_process;
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

/* type define the close functions */
typedef int32_t (*write_t)(int32_t, void*, int32_t);

/* type define the close functions */
typedef int32_t (*close_t)(int32_t);

/* 
 * stdin_init
 *		DESCRIPTION: Open a new stdin file at the corresponding fd index in the file descriptor table
 *		INPUTS: fd (the desired index of the file descriptor table to open the file on)              
 *		OUTPUTS: none
 *		SIDE EFFECT: corresponding entry inside the file descriptor table will have an active stdin file
 *
 */
void stdin_init(int32_t fd) {
	active_process->file_descriptor_table[fd].f_op = stdin_jmp_table;
	active_process->file_descriptor_table[fd].active = ACTIVE;
}

/* 
 * stdout_init
 *		DESCRIPTION: Open a new stdout file at the corresponding fd index in the file descriptor table
 *		INPUTS: fd (the desired index of the file descriptor table to open the file on)              
 *		OUTPUTS: none
 *		SIDE EFFECT: corresponding entry inside the file descriptor table will have an active stdout file
 *
 */
void stdout_init(int32_t fd) {
	active_process->file_descriptor_table[fd].f_op = stdout_jmp_table;
	active_process->file_descriptor_table[fd].active = ACTIVE;
}

int32_t execute(const uint8_t * command){

/***************************************************/
/*   The following is checking the file validity   */ 
/***************************************************/

  printf("%s\n", command);

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
  uint8_t filename_buf [FILE_NAME_MAX_SIZE];
  int filename_cnt = 0;
  int i = 0;

  // loop through the command to get the file name
  while(command[i] != ' ' && i < FILE_NAME_MAX_SIZE)
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
  uint8_t temp_read[4];
  read_data(dentry.inode,0, temp_read, 4);
  
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
  i = 0;
  while(pid_array[i] == 1 ){
	  i++;
  }
  
  pid_array[i] = 1;
  // store pcb to 8MB - (pid+1) * 8kb
  pcb_t* current_pcb = (pcb_t*)(EIGHT_MEGABYTES - (i + 1) * EIGHT_KILOBYTES);
  current_pcb->pid = i;
  current_pcb->esp = NULL;
  current_pcb->ebp = NULL;
  
  // see if we need to store parent pcb pointer
  if( active_process != NULL){
	  current_pcb->parent_pcb = (pcb_t*)(EIGHT_MEGABYTES - (active_process->pid + 1) * EIGHT_KILOBYTES);
  }
  else{
	  current_pcb->parent_pcb = NULL;
  }
  
  // open FDs, update the global process info
  active_process = current_pcb;
  
  stdin_init(Stdin_FD);
  stdout_init(Stdout_FD);
 
  // setting up paging 
  user_page_init(current_pcb->pid);
  
  // loads the file into memory
  read_data(dentry.inode, 0, file_start,four_mb-file_offset);  
 
  // gets the entry point from the file
  uint32_t entry_point = 0;
  uint8_t buf[4];
  read_data(dentry.inode, EXECUTE_BYTE_OFFSET, buf, 4);
  entry_point = (buf[3] << FIRST_QUARTER_THIRTYTWO) | (buf[2] << SECOND_QUARTER_THIRTYTWO) | (buf[1] << THIRD_QUARTER_THIRTYTWO) | buf[0]; 

  uint32_t user_esp = USER_ESP;
  
  // update tss before iret
  tss.esp0 = (EIGHT_MEGABYTES - i * EIGHT_KILOBYTES - USER_OFFSET);
  tss.ss0 = KERNEL_DS;
  
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

	
	return 0;
  
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


int32_t halt (uint8_t status){
	
	uint32_t fd = 0;
	// close any relevant FDs
	while(active_process->file_descriptor_table[fd].active == ACTIVE) {
		active_process->file_descriptor_table[fd].active = INACTIVE;		
		fd++;
	}

	// restore parent paging
	if(active_process->parent_pcb != NULL)
		user_page_init(active_process->parent_pcb->pid);

	// restore parent's pcb
	active_process = active_process->parent_pcb;

	// change the tss esp0 to point to the bottom of 8kb kernel process, the kernel stack
	tss.esp0 = active_process + 0x2000;
	tss.ss0 = KERNEL_DS;

	// setting the esp and ebp values 
	asm __volatile__("movl  %0, %%esp"  : :"r" (active_process->esp));
	asm __volatile__("movl  %0, %%ebp"  : :"r" (active_process->ebp));
	


	// jump to execute return




	return 0;

}













