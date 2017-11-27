#define SUCCESS 0
#define FAILURE -1
#define ACTIVE 1
#define INACTIVE 0
#define SUCCESS 0
#define FAILURE -1
#define ACTIVE 1
#define INACTIVE 0
#define USER_ESP (0x08400000 - 4)
#define EIGHT_MEGABYTES 0x800000
#define EIGHT_KILOBYTES 0x2000
#define FOUR_KILOBYTES 4096
#define USER_OFFSET 4
#define EXECUTE_BYTE_OFFSET 24
#define FIRST_QUARTER_THIRTYTWO 24
#define SECOND_QUARTER_THIRTYTWO 16
#define THIRD_QUARTER_THIRTYTWO 8
#define FILE_NAME_MAX_SIZE 32
#define EIP_VALUE 134513384
#define OPEN 0
#define READ 1
#define WRITE 2
#define CLOSE 3
#define NUM_OF_OP 4
#define RTC_FILE 0
#define DIRECTORY_FILE 1
#define REGULAR_FILE 2
#include "types.h"

// structure for our fd array
typedef struct file_t{
	int32_t* f_op;
	uint32_t inode;
	uint32_t f_offset;
	uint32_t active;
} file_t;

// structure for our process control block
typedef struct pcb_t{
	
	file_t file_descriptor_table[8];
	uint32_t pid;
	uint32_t esp;
	uint32_t ebp;
	struct pcb_t *parent_pcb;
	uint32_t esp0;
	uint32_t ss0;
	uint8_t arg[128];
		
} pcb_t;


// functions necessary to support system call
extern int32_t read(int32_t fd, void* buf, int32_t nbytes);
extern int32_t write(int32_t fd, void* buf, int32_t nbytes);
extern int32_t open(const uint8_t* filename);
extern int32_t close(int32_t fd);
extern void stdin_init(int32_t fd);
extern void stdout_init(int32_t fd); 
extern int32_t execute(const uint8_t * command);
extern int32_t vidmap(uint8_t** screen_start);
extern int32_t getargs (uint8_t* buf, int32_t nbytes);
