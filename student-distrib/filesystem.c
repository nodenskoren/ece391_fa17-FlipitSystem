#include "lib.h"
#include "filesystem.h"

#define BOOTBLOCK_INODE_OFFSET 4
#define BOOTBLOCK_DATABLOCK_OFFSET 8
#define DENTRY_OFFSET 64
#define DENTRY_FILETYPE_OFFSET 32
#define DENTRY_INODE_OFFST 36
#define DENTRY_MAX_SIZE 63
#define SUCCESS 0
#define FAILURE -1
#define FILENAME_MAX 32

static boot_block* filesystem_addr;
static inode_t* inode_field_starting_addr;
static data_block_t* data_field_starting_addr;
static dentry_t file_dir;
static uint32_t file_offset;
static uint32_t dentry_offset;
static uint32_t fname_length;

#define block_size 4096
static uint32_t inode_start;
static uint32_t data_block_start;

unsigned int boot_addr;
unsigned int dentry_addr;

void filesystem_init(unsigned int boot_address) {
	
	boot_addr = boot_address; //get the pointer that points to the boot block
	dentry_addr = boot_addr + DENTRY_OFFSET;
	//printf("0x%#x\n", boot_addr);
	
	filesystem_addr = (uint32_t*) boot_address;
	printf("%d", filesystem_addr);
	filesystem_addr->dentry_count = (uint32_t)(*(uint32_t*)boot_addr); // add number of dentry to boot block struct
	filesystem_addr->inode_count = (uint32_t)(*(uint32_t*)(boot_addr + BOOTBLOCK_INODE_OFFSET)); // add number of inode to boot block struct
	filesystem_addr->data_block_count = (uint32_t)(*(uint32_t*)(boot_addr+BOOTBLOCK_DATABLOCK_OFFSET)); // add number of data block to boot block struct 
	inode_start = boot_address + block_size;
	data_block_start = inode_start + filesystem_addr->inode_count * block_size;
	inode_field_starting_addr = filesystem_addr + 4096;
	data_field_starting_addr = inode_field_starting_addr + 4096 * filesystem_addr->inode_count;
	//printf("%d\n", fs_info->dentry_count);
	//printf("%d\n", fs_info->inode_count);
	//printf("%d\n", fs_info->data_block_count);
}

int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry){
	
	if(fname == NULL || dentry == NULL)
		return FAILURE;
	
	int i;
	fname_length = strlen((int8_t*)fname); // get the fname length
	if(fname_length > 32) fname_length = 32;
	
	for(i = 0; i < DENTRY_MAX_SIZE; i++){
		int8_t* dentry_file_name = (int8_t*)(dentry_addr + DENTRY_OFFSET*i); // get the file name array for the corresponding dentry
		if(strlen(dentry_file_name) == fname_length) { // see if the length of the file names match
			if(strncmp((int8_t*)fname, dentry_file_name, fname_length) == 0) // see if the content of the filename matches
			{
				strncpy((int8_t*)dentry->file_name, (int8_t*)dentry_file_name, fname_length); // copy the file name to dentry struct
				dentry->file_type = (uint32_t)(*(uint32_t*)(dentry_addr + DENTRY_OFFSET*i + DENTRY_FILETYPE_OFFSET)); // copy the filetype to dentry struct
				dentry->inode = (uint32_t)(*(uint32_t*)(dentry_addr + DENTRY_OFFSET*i + DENTRY_INODE_OFFST)); // copy the inode index to dentry struct
				
				return SUCCESS; 
			}
		}	
	}
	return FAILURE; // fule not found, return failure
}

int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry){
	
	if((index > filesystem_addr->dentry_count) || dentry == NULL ) // check if the index is out of bound or dentry is null
		return FAILURE; 

	int8_t* dentry_file_name = (int8_t*)(dentry_addr + DENTRY_OFFSET*index); // get the file name array for the corresponding dentry
	uint32_t fname_length = strlen((int8_t*)dentry_file_name); // get the fname length
	if(fname_length > FILENAME_MAX) fname_length = FILENAME_MAX; // check if the file name is longer than 32
	
	strncpy((int8_t*)dentry->file_name, (int8_t*)dentry_file_name, fname_length); // copy the file name to dentry struct
	dentry->file_type = (uint32_t)(*(uint32_t*)(dentry_addr + DENTRY_OFFSET*index + DENTRY_FILETYPE_OFFSET)); // copy the filetype to dentry struct
	dentry->inode = (uint32_t)(*(uint32_t*)(dentry_addr + DENTRY_OFFSET*index + DENTRY_INODE_OFFST)); // copy the inode index to dentry struct
	
	return SUCCESS; 	
}

int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
	//uint8_t* buf_temp = buf;

	// if argument doesnt make sense, return -1
	if((inode < 0) || (inode > (filesystem_addr->inode_count - 1)) || (offset < 0) || (length < 0) || (buf == NULL)){
		return -1;
	}
	
	inode_t* current_inode = (inode_t*)(inode_start + inode * block_size);
	if(offset >= current_inode->length_in_b) {
		return 0;
	}
	
	// if arguments make sense, we find where and start copy
	else {
		// get the current inode
		//printf("length_in_b%d\n",current_inode->length_in_b);
		// if length is too much, we need to shrink it
		if((offset + length) > current_inode->length_in_b){
			length = (current_inode->length_in_b) - offset;
		}
			
		uint32_t data_copied = 0;
		uint32_t i;
		uint32_t byte_to_copy;
		uint32_t cur_datablock;
		uint32_t data_block_offset;

		//loop through blocks to copy data
		// if we only have one block to print
		//if we need to print from multiple data blocks
		for(i = 0; i <= (offset + length) / block_size; i++){
			//printf("length%d\n",i);
			//printf("length%d\n",data_copied);
			// get the offset into our data block
			data_block_offset = offset % block_size;
			// get the number of bytes to be copied
			if(length >= block_size) {
				byte_to_copy = block_size - offset % block_size;
			}
			else {
				byte_to_copy = length;
			}
			// get the location of current data block
			cur_datablock = data_block_start + (current_inode->data_block_number[i])*block_size;
			// copy the data into buffer
			memcpy((void*)buf, (void*)(cur_datablock + data_block_offset), byte_to_copy);

			// update offset and buffer
			buf = buf + byte_to_copy;
			offset = offset + byte_to_copy;
			data_copied = data_copied + byte_to_copy;
			length = length - byte_to_copy;
		}
		return  data_copied;
	}
}

void test_read_file() {
	clear();
	printf("Reading file test_case ...\n");
	dentry_t test_dentry; //declare a dentry to fetch result from read file function
	uint8_t buffer[6000];
	uint8_t file_name_buffer[32];
	extern uint32_t inode_start;
	//check if this file exist
	if (read_dentry_by_name((uint8_t* )"frame0.txt", &test_dentry) == -1) {	
		printf("Invalid read!\n");
		return;
	}

	printf("The file type is: %d\n", test_dentry.file_type); //print file type
	printf("The file inode number is: %d\n", test_dentry.inode); //print file inode number

	//read data in the data block
	if (read_data(test_dentry.inode, 0, buffer, 6000) <= 0) {
		//printf(buffer[6]);
		printf("\nInvalid read_data or end of file!\n");
		return;
	}
	printf("\nThe file length is: %d\n", ((inode_t *)(inode_start + test_dentry.inode * (4 * 1024)))->length_in_b);
	int i;
	memcpy((void*)file_name_buffer, (void*)&(test_dentry.file_name), fname_length);
	printf("The file name is: ");
	for (i = 0; i < fname_length; i++) {
		printf("%c", file_name_buffer[i]);
	}
	printf("\n");
	//print data in the file
	for (i = 0; i < ((inode_t *)(inode_start + test_dentry.inode * (4 * 1024)))->length_in_b; i++) {
		printf("%c", buffer[i]);
	}
	//print out the data length
	printf("\nread_data success!");
	return;
}

int32_t regular_file_open(uint8_t* fname) {
	if(fname != NULL) {
		file_offset = 0;
		return 0;
	}
	return -1;
}

int32_t regular_file_read(uint8_t* fname, uint8_t* buf, uint32_t count) {
	dentry_t dentry;
	read_dentry_by_name(fname, &(dentry));
	if((buf == NULL) || (dentry.file_type != REGULAR_FILE)) {
		printf("Wrong filetype!\n");
		return -1;
	}
	int32_t length_read;
	length_read = read_data(dentry.inode, file_offset, buf, count);
	file_offset += length_read;
	return length_read;
}

void test_regular_file() {
	dentry_t d_entry;
	read_dentry_by_name((uint8_t*)"frame0.txt", &(d_entry));
	//printf("\n%d", d_entry.file_type);
	/*
	int index;
	for(index = 0; index < filesystem_addr->dentry_count; index++){
		read_dentry_by_index(index, &(d_entry));
		printf("\n%d", d_entry.file_type);
	}
	return;
	*/
	if(regular_file_open((uint8_t* )"frame0.txt") != 0) {
		printf("Invalid file...\n");
		return;
	}
	uint8_t buffer[6000];
	uint8_t file_name_buffer[32];
	int i;
	memcpy((void*)file_name_buffer, (void*)&(d_entry.file_name), fname_length);
	printf("\n\nRegular file testing...");
	printf("\nThe opened file name is: ");
	for (i = 0; i < fname_length; i++) {
		printf("%c", file_name_buffer[i]);
	}
	printf("\nTesting the read function...\n");
	regular_file_read((uint8_t*)"frame0.txt", buffer, 4);
	for (i = 0; i < 4; i++) {
		printf("%c", buffer[i]);
	}
	regular_file_read((uint8_t*)"frame0.txt", buffer, 100);
	for (i = 0; i < 100; i++) {
		printf("%c", buffer[i]);
	}
	regular_file_read((uint8_t*)"frame0.txt", buffer, 50);
	for (i = 0; i < 50; i++) {
		printf("%c", buffer[i]);
	}
	return;
}

int32_t regular_file_write() {
	return -1;
}

int32_t regular_file_close() {
	return 0;
}

int32_t directory_file_open(uint8_t* fname) {
	if(fname != NULL) {
		dentry_offset = 0;
		return 0;
	}
	return -1;
}

int32_t directory_file_read(uint8_t* buf, uint32_t length) {
	dentry_t dentry;
	if((buf == NULL) || (read_dentry_by_index(dentry_offset, &(dentry)) == -1)) {
		return -1;
	}
	if(dentry_offset >= filesystem_addr->dentry_count) {
		return 0;
	}
	if(length > 32) {
		length = 32;
	}
	strncpy((int8_t*)buf, (int8_t*)&(dentry.file_name), length);
	dentry_offset++;
	return length;
}

int32_t directory_file_write() {
	return -1;
}

int32_t directory_file_close() {
	return 0;
}
