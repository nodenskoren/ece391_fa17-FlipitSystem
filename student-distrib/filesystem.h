#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#define DIRECTORY_FILE 1
#define REGULAR_FILE 2
#define BLOCKSIZE 4096
#define FILE_NAME_MAX_LENGTH 32
#define FILE_SIZE_MAX_LENGTH 5
#define SINGLE_DIGIT_LARGEST_NUMBER 9
#define DOUBLE_DIGIT_SMALLEST_NUMBER 10
#define DATA_BLOCK_NUMBER 1023
#define DENTRY_RESERVED 24
#define BOOT_BLOCK_RESERVED 52
#define BOOTBLOCK_INODE_OFFSET 4
#define BOOTBLOCK_DATABLOCK_OFFSET 8
#define DENTRY_OFFSET 64
#define DENTRY_FILETYPE_OFFSET 32
#define DENTRY_INODE_OFFST 36
#define DENTRY_MAX_SIZE 63
#define SUCCESS 0
#define FAILURE -1
#define FILENAME_MAX 32
#define block_size 4096
#define BUFFER_SIZE_TEST 6000
#define END_OF_FILE 0

extern uint32_t inode_start;




typedef struct dentry_t {
	uint8_t file_name[FILE_NAME_MAX_LENGTH];
	uint32_t file_type;
	uint32_t inode;
	uint8_t dentry_reserved[DENTRY_RESERVED];
} dentry_t;

typedef struct boot_block {
	uint32_t dentry_count;
	uint32_t inode_count;
	uint32_t data_block_count;
	uint8_t boot_block_reserved[BOOT_BLOCK_RESERVED];
	dentry_t dentries[DENTRY_MAX_SIZE];
} boot_block;

typedef struct inode_t {
	uint32_t length_in_b;
	uint32_t data_block_number[DATA_BLOCK_NUMBER];
} inode_t;

typedef struct data_block_t {
	uint8_t data[BLOCKSIZE];
} data_block_t;

/* initialize boot block */
extern void filesystem_init(unsigned int boot_address);

/* read dentry structure into buffer by file name*/
extern int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);
/* read dentry structure into buffer by index*/
extern int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);
/* read data into buffer from an inode*/
extern int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

/* METHODS FOR REGULAR FILES */
extern int32_t regular_file_open(const uint8_t* fname);
extern int32_t regular_file_read(int32_t fd, uint8_t* buf, uint32_t count);
extern int32_t regular_file_write(int32_t fd, const uint8_t* buf, uint32_t count);
extern int32_t regular_file_close(int32_t fd);

/* METHODS FOR DIRECTORY FILES */
extern int32_t directory_file_open();
extern int32_t directory_file_read(int32_t fd, uint8_t* buf, uint32_t length);
extern int32_t directory_file_write(int32_t fd, const uint8_t* buf, uint32_t count);
extern int32_t directory_file_close(int32_t fd);

/* Test functions */
extern void test_read_file();
extern void test_regular_file();
extern void test_directory_file();
extern void test_read_dentry();
extern void print_out_every_file();
extern void test_read_file_non_text();
extern void test_read_index();

#endif
