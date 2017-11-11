#include "systemcall.h"
#include "types.h"
#include "filesystem.h"
#include "paging.h"
#include "terminal_driver.h"
#include "rtc_driver.h"
#include "keyboard_handler.h"
#include "x86_desc.h"
#include "lib.h"

pcb_t* active_process;
extern uint32_t inode_start;
uint32_t pid_array[8];

uint32_t stdin_jmp_table[4] = {0, (uint32_t)terminal_read, 0, 0};
uint32_t stdout_jmp_table[4] = {0, 0, (uint32_t)terminal_write, 0};
uint32_t rtc_jmp_table[4] = {(uint32_t)RTC_open, (uint32_t)RTC_read, (uint32_t)RTC_write, (uint32_t)RTC_close};
uint32_t file_jmp_table[4] = {(uint32_t)regular_file_open, (uint32_t)regular_file_read, (uint32_t)regular_file_write, (uint32_t)regular_file_close};
uint32_t dir_jmp_table[4] = {(uint32_t)directory_file_open, (uint32_t)directory_file_read, (uint32_t)directory_file_write, (uint32_t)directory_file_close};	

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
				fds[fd].inode = inode_start + d_entry.inode * 4096;
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

void stdin_init(int32_t fd) {
	active_process->file_descriptor_table[fd].f_op = stdin_jmp_table;
	active_process->file_descriptor_table[fd].active = ACTIVE;
}

void stdout_init(int32_t fd) {
	active_process->file_descriptor_table[fd].f_op = stdout_jmp_table;
	active_process->file_descriptor_table[fd].f_op = ACTIVE;
}






int32_t execute(const uint8_t * command){

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
  char filename_buf [32];
  int filename_cnt = 0;
  int i = 0;

  // loop through the command to get the file name
  while(command[i] != ' ' && i < 32)
  {
    filename_buf[i] = command[i];
    i++;
  }
  // if our filename doesnt end with space, the file name is illegal
  if(command[i] != ' ')
  {
    puts("Exception: illegal file name!\n");
    return -1;
  }

  // try to read
  dentry_t dentry;
  if ( read_dentry_by_name (filename_buf, &(dentry)) == -1 )
  {
    puts("Exception: illegal file name!\n");
    return -1;
  }

  uint32_t eip_value = 0;
  uint8_t buf[4];
  read_data(dentry.inode, 24, buf, 4);
  eip_value = (buf[0] >> 24) | (buf[1] >> 16) | (buf[2] >> 8) | buf[3]; 

  uint32_t whatever = 0x08400000;
  
  asm volatile (
	"pushw %0			\n \
	 pushl %1			\n \
	 pushfl                         \n \
	 popl %%ebx                     \n \
	 orl $0x00000100,%%ebx          \n \
	 pushl %%ebx                    \n \
	 pushw %2			\n \
	 push %3			\n \
	 movw %0,%%ds             \n \
	 iret                     \n \
	 EXECUTE_RETURN:"
	:	
	: "a"(USER_DS), "b"(whatever), "c"(USER_CS), "d"(eip_value) 
	: "memory" ); 
	
	return 0;
}
