#define SUCCESS 0
#define FAILURE -1
#define ACTIVE 1
#define INACTIVE 0
#include "types.h"

typedef struct {
	uint32_t* f_op;
	uint32_t* inode;
	uint32_t f_offset;
	uint32_t active;
} file_t;

typedef struct {
		file_t file_descriptor_table[8];
		uint8_t pid;
		uint32_t esp0;
} pcb_t;


extern int32_t read(int32_t fd, void* buf, int32_t nbytes);
extern int32_t write(int32_t fd, const void* buf, int32_t nbytes);
extern int32_t open(const uint8_t* filename);
extern int32_t close(int32_t fd);
extern void stdin_init(int32_t fd);
extern void stdout_init(int32_t fd);
extern int32_t execute(const uint8_t * command);
