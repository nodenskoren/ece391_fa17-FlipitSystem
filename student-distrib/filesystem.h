#include "lib.h"
#include "filesystem.c"

#define DIRECTORY_FILE 1
#define REGULAR_FILE 2

typedef struct dentry_t {
	uint8_t file_name[32];
	uint32_t file_type;
	uint32_t inode;
	uint8_t dentry_reserved[24];
} dentry_t;

typedef struct boot_block {
	uint32_t dentry_count;
	uint32_t inode_count;
	uint32_t data_block_count;
	uint8_t boot_block_reserved[52];
	dentry_t dentries[63];
} boot_block;

typedef struct inode_t {
	uint32_t length_in_b;
	uint32_t data_block_number[1023];
} inode_t;

typedef struct data_block_t {
	uint8_t data[4096];
} data_block_t;

extern int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);
extern int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);
extern int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

/* METHODS FOR REGULAR FILES */
extern uint32_t regular_file_open(uint8_t* fname);
extern uint32_t regular_file_read(uint8_t* buf, uint32_t count);
extern uint32_t regular_file_write();
extern uint32_t regular_file_close();

/* METHODS FOR DIRECTORY FILES */
extern uint32_t directory_file_open(uint8_t* fname);
extern uint32_t directory_file_read(uint8_t* buf, uint32_t length);
extern uint32_t directory_file_write();
extern uint32_t directory_file_close();