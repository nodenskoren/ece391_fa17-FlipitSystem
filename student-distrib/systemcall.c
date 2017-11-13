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

uint32_t stdin_jmp_table[4] = {0, (uint32_t)terminal_read, 0, 0};
uint32_t stdout_jmp_table[4] = {0, 0, (uint32_t)terminal_write, 0};
uint32_t rtc_jmp_table[4] = {(uint32_t)RTC_open, (uint32_t)RTC_read, (uint32_t)RTC_write, (uint32_t)RTC_close};
uint32_t file_jmp_table[4] = {(uint32_t)regular_file_open, (uint32_t)regular_file_read, (uint32_t)regular_file_write, (uint32_t)regular_file_close};
uint32_t dir_jmp_table[4] = {(uint32_t)directory_file_open, (uint32_t)directory_file_read, (uint32_t)directory_file_write, (uint32_t)directory_file_close};	


void stdin_init(int32_t fd) {
	active_process->file_descriptor_table[fd].f_op = stdin_jmp_table;
	active_process->file_descriptor_table[fd].active = ACTIVE;
}

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
  uint8_t filename_buf [32];
  int filename_cnt = 0;
  int i = 0;

  // loop through the command to get the file name
  while(command[i] != ' ' && i < 32)
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
  
  
  // store pcb to 8MB - (pid+1) * 8kb
  pcb_t* current_pcb = (pcb_t*)(0x800000 - (i + 1) * 0x2000);
  current_pcb->pid = i;
  current_pcb->esp = NULL;
  current_pcb->ebp = NULL;
  
  // see if we need to store parent pcb pointer
  if( active_process != NULL){
	  current_pcb->parent_pcb = (pcb_t*)(0x800000 - (active_process->pid + 1) * 0x2000);
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
  read_data(dentry.inode,0,file_start,four_mb-file_offset);  
 
  // gets the entry point from the file
  uint32_t entry_point = 0;
  uint8_t buf[4];
  read_data(dentry.inode, 24, buf, 4);
  entry_point = (buf[3] << 24) | (buf[2] << 16) | (buf[1] << 8) | buf[0]; 

  uint32_t user_esp = 0x08400000;
  
  // update tss before iret
  tss.esp0 = (0x800000 - i * 0x2000);
  tss.ss0 = KERNEL_DS;
  
  asm volatile (
  	"pushl $0x002B			\n \
	 pushl $0x08400000			\n \
	 pushfl                         \n \
	 popl %%ebx                     \n \
	 orl $0x00000200,%%ebx          \n \
	 pushl %%ebx                    \n \
	 pushl $0x0023			\n \
	 pushl $134513384			\n \
	 iret                     \n \
	 EXECUTE_RETURN:"
	:	
	: 
	: "memory"); 	

	
	return 0;
  
}


int32_t read(int32_t fd, void* buf, int32_t nbytes) {
	int32_t bytes_read = terminal_read(buf, nbytes);
	return bytes_read;
}

int32_t write(int32_t fd, const void* buf, int32_t nbytes) {
	terminal_write(buf, nbytes);
	return SUCCESS;
}

int32_t open(const uint8_t* filename) {
	pcb_t* curr_process = active_process;
	file_t* fds = curr_process->file_descriptor_table;
	dentry_t d_entry;

	uint32_t fd = 0;
	while(fds[fd].active != 0) {
		fd++;
	}

	if(fd <= 7 && read_dentry_by_name(filename, &d_entry) >= 0) {
		switch(d_entry.file_type) {
			case 0: // RTC
				fds[fd].f_op = rtc_jmp_table;
				fds[fd].inode = NULL;
				fds[fd].f_offset = 0;
				fds[fd].active = ACTIVE;
				RTC_open(filename);
				return SUCCESS;

			case 1: // Directory
				fds[fd].f_op = dir_jmp_table;
				fds[fd].inode = NULL;
				fds[fd].f_offset = 0;
				fds[fd].active = ACTIVE;
				directory_file_open();
				return SUCCESS;

			case 2:	// File
				fds[fd].f_op = file_jmp_table;
				fds[fd].inode = (uint32_t*)(inode_start + d_entry.inode * 4096);
				fds[fd].f_offset = 0;
				fds[fd].active = ACTIVE;
				regular_file_open(filename);
				return SUCCESS;

			default: // Invalid type
				return FAILURE;
		}
	}
	return FAILURE;
}

int32_t close(int32_t fd) {
	pcb_t* curr_process = active_process;
	file_t fds[8];
	int i;
	for(i = 0; i < 8; i++) {
		fds[i] = curr_process->file_descriptor_table[i];
	}
	fds[fd].active = INACTIVE;
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













