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
static uint32_t file_offset;
static uint32_t dentry_offset;
static uint32_t fname_length;

#define block_size 4096
static uint32_t inode_start;
static uint32_t data_block_start;

unsigned int boot_addr;
unsigned int dentry_addr;

/* 
 * filesystem_init
 *		DESCRIPTION: initialize filesystem and get boot block structure
 *		INPUTS: boot block address
 *		OUTPUTS: None
 *		SIDE EFFECT: initialize static filesystem boot block strucutre
 *
 */
void filesystem_init(unsigned int boot_address) {
	
	boot_addr = boot_address; //get the pointer that points to the boot block
	dentry_addr = boot_addr + DENTRY_OFFSET; //get the starting address of dentry in bootblock
	
	filesystem_addr = (boot_block*) boot_address;
	
	filesystem_addr->dentry_count = (uint32_t)(*(uint32_t*)boot_addr); // add number of dentry to boot block struct
	filesystem_addr->inode_count = (uint32_t)(*(uint32_t*)(boot_addr + BOOTBLOCK_INODE_OFFSET)); // add number of inode to boot block struct
	filesystem_addr->data_block_count = (uint32_t)(*(uint32_t*)(boot_addr+BOOTBLOCK_DATABLOCK_OFFSET)); // add number of data block to boot block struct 
	inode_start = boot_address + block_size; // initialize the starting address of inode field
	data_block_start = inode_start + filesystem_addr->inode_count * block_size; // initialize the starting address of data field
}

/* 
 * read_dentry_by_name
 *		DESCRIPTION: read dentry structure given a file name
 *		INPUTS: fname- a char array pointer of file name
 *              dentry- dentry structure to store the dentry read
 *		OUTPUTS: 0 if success, -1 if fail
 *		SIDE EFFECT: store dentry info into dentry structure passed
 *
 */
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry){
	
	int i;
	/* If name fname doesn't exist or if dentry doesn't exist, return with error message */
	if(fname == NULL || dentry == NULL) {
		return FAILURE;
	}

	fname_length = strlen((char*)fname); // get the fname length
	
	/* If length is greater than 32, truncate the length to 32 (maximum size) of buffer */
	if(fname_length > 32) {
		fname_length = 32;
	}
	
	/* Loop over every directory entry and search by name */
	for(i = 0; i < DENTRY_MAX_SIZE; i++){
		/* Create a local dentry structure and copy the directory info of current entry into it */
		dentry_t curr_dentry = filesystem_addr->dentries[i];
		/* Check if the filename matches */
		if(strncmp((int8_t*)fname, (int8_t*)curr_dentry.file_name, fname_length) == 0) { 
			int index;
			/* If matched, copy all the directory info of this entry into the dentry structure passed in as argument */
			for(index = 0; index < fname_length; index++) {
				dentry->file_name[index] = curr_dentry.file_name[index];
			}
			dentry->file_type = curr_dentry.file_type;
			dentry->inode = curr_dentry.inode;	
			return SUCCESS; 
		}
	}
	
	/* If filename not found, return with error message */
	return FAILURE;
}

// helper function for read dentry test
void test_read_dentry() {
	dentry_t dentry;
	int i;
	char *p = "verylargetextwithverylongname.txt";	
	read_dentry_by_name((uint8_t*)p, &(dentry));
	
	for(i = 0; i < fname_length; i++) {
		printf("%c", p[i]);
	}
}


/* 
 * read_dentry_by_index
 *		DESCRIPTION: read dentry structure given an index
 *		INPUTS: index- a dentry index
 *              dentry- dentry structure to store the dentry read
 *		OUTPUTS: 0 if success, -1 if fail
 *		SIDE EFFECT: store dentry info into dentry structure passed
 *
 */
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry){
	
	if((index > filesystem_addr->dentry_count) || dentry == NULL ) // check if the index is out of bound or dentry is null
		return FAILURE; 

	int8_t* dentry_file_name = (int8_t*)(dentry_addr + DENTRY_OFFSET*index); // get the file name array for the corresponding dentry
	fname_length = strlen((int8_t*)dentry_file_name); // get the fname length
	if(fname_length > FILENAME_MAX) fname_length = FILENAME_MAX; // check if the file name is longer than 32
	
	strncpy((int8_t*)dentry->file_name, (int8_t*)dentry_file_name, fname_length); // copy the file name to dentry struct
	dentry->file_type = (uint32_t)(*(uint32_t*)(dentry_addr + DENTRY_OFFSET*index + DENTRY_FILETYPE_OFFSET)); // copy the filetype to dentry struct
	dentry->inode = (uint32_t)(*(uint32_t*)(dentry_addr + DENTRY_OFFSET*index + DENTRY_INODE_OFFST)); // copy the inode index to dentry struct
	
	return SUCCESS; 	
}


/* 
 * read_data
 *		DESCRIPTION: read data from a specific inode given length and offset
 *		INPUTS: inode - the inode number to be read
 *              offset - offset in bytes of the starting point of our read
 *              buf - the buffer for data to be copied
 *              length - length in bytes to be read
 *		OUTPUTS: 0 if end of file, -1 if fail, 32 bits number of bytes read
 *		SIDE EFFECT: store data into buffer passed
 *
 */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){

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
		for(i = offset/block_size; i <= (offset + length) / block_size; i++){
			// get the offset into our data block
			data_block_offset = offset % block_size ;
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

// test function to test read data from a file
void test_read_file() {
	clear();
	printf("Reading file test_case ...\n");
	dentry_t test_dentry; //declare a dentry to fetch result from read file function
	uint8_t buffer[6000];
	uint8_t file_name_buffer[32];
	extern uint32_t inode_start;
	//check if this file exist
	char *p = "verylargetextwithverylongname.txt";
	if (read_dentry_by_name((uint8_t* )p, &test_dentry) == -1) {	
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
	printf("\nThe file length is: %d\n", ((inode_t *)(inode_start + test_dentry.inode * block_size))->length_in_b);
	int i;
	memcpy((void*)file_name_buffer, (void*)&(test_dentry.file_name), fname_length);
	printf("The file name is: ");
	for (i = 0; i < fname_length; i++) {
		printf("%c", file_name_buffer[i]);
	}
	printf("\n");
	//print data in the file
	for (i = 0; i < ((inode_t *)(inode_start + test_dentry.inode * block_size))->length_in_b; i++) {
		printf("%c", buffer[i]);
	}
	//print out the data length
	printf("\nread_data success!\n");
	
	return;
}


/* 
 * regular_file_open
 *		DESCRIPTION: try to open a file
 *		INPUTS: fname - file name of the file to be opend
 *              
 *		OUTPUTS: 0 if success, -1 if fail
 *		SIDE EFFECT: none
 *
 */
int32_t regular_file_open(uint8_t* fname) {
	if(fname != NULL) {
		file_offset = 0;
		return 0;
	}
	return -1;
}


/* 
 * regular_file_read
 *		DESCRIPTION: read data from a file
 *		INPUTS: fname - file name of the file to be read
 *              buf - the buffer to store the read data
 *              count - number of bytes to be read
 *		OUTPUTS: number of bytes read, -1 if fail
 *		SIDE EFFECT: read data into buffer
 *
 */
int32_t regular_file_read(uint8_t* fname, uint8_t* buf, uint32_t count) {
	/* Define a local dentry */
	dentry_t dentry;
	/* Call the read_dentry_by_name function to find the file info and copy it to the local dentry */
	read_dentry_by_name(fname, &(dentry));
	/* If the buffer doesn't exist or if the filetype isn't supported by this method, return with error message */
	if((buf == NULL) || (dentry.file_type != REGULAR_FILE)) {
		printf("Wrong filetype!\n");
		return -1;
	}
	int32_t length_read;
	/* Read the data by calling read_data function */
	length_read = read_data(dentry.inode, file_offset, buf, count);
	/* Add the progress read */
	file_offset += length_read;
	return length_read;
}

// helper function to test file read
void test_regular_file() {
	dentry_t d_entry;
	read_dentry_by_name((uint8_t*)"hello", &(d_entry));
	//printf("\n%d", d_entry.file_type);
	/*
	int index;
	for(index = 0; index < filesystem_addr->dentry_count; index++){
		read_dentry_by_index(index, &(d_entry));
		printf("\n%d", d_entry.file_type);
	}
	return;
	*/
	if(regular_file_open((uint8_t* )"hello") != 0) {
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
/*
	regular_file_read((uint8_t*)"frame1.txt", buffer, 4);
	for (i = 0; i < 4; i++) {
		printf("%c", buffer[i]);
	} */
	regular_file_read((uint8_t*)"hello", buffer, 100);
	for (i = 0; i < 100; i++) {
		printf("%c", buffer[i]);
	}
	return;
}

/* 
 * regular_file_write
 *		DESCRIPTION: try to write to a file(not implemented)
 *		INPUTS: none
 *              
 *		OUTPUTS: -1
 *		SIDE EFFECT: none
 *
 */
int32_t regular_file_write() {
	/* Print out error message and return */
	printf("... Write function not implemented yet!\n Now returning...!");
	return -1;
}

/* 
 * regular_file_close
 *		DESCRIPTION: try to close a file
 *		INPUTS: fname - none
 *              
 *		OUTPUTS: 0
 *		SIDE EFFECT: none
 *
 */
int32_t regular_file_close() {
	/* Reset the reading progress (offset) back to 0 */
	file_offset = 0;
	printf("File closed. File offset = %d", dentry_offset);
	return 0;
}

/* 
 * directory_file_open
 *		DESCRIPTION: try to open a directory
 *		INPUTS: none
 *              
 *		OUTPUTS: 0
 *		SIDE EFFECT: none
 *
 */
int32_t directory_file_open() {
	/* Set the reading progress (offset) to 0 */
	dentry_offset = 0;
	return 0;
}

/* 
 * directory_file_read
 *		DESCRIPTION: Read dentry name into buffer
 *		INPUTS: buf - The buffer storing the dentry name
 *              length - Desired length of dentry name wish to be shown
 *              
 *		OUTPUTS: Length of the dentry name read. -1 if read fail, 0 if EOF reached.
 *		SIDE EFFECT: read dentry name into buffer
 *
 */
int32_t directory_file_read(uint8_t* buf, uint32_t length) {
	dentry_t dentry;
	/* If the buffer doesn't exist or if the directory entry doesn't exist, return -1 */
	if((buf == NULL) || (read_dentry_by_index(dentry_offset, &(dentry)) == -1)) {
		return -1;
	}
	/* If EOF has been reached, read nothing and return 0 */
	if(dentry_offset >= filesystem_addr->dentry_count) {
		return 0;
	}
	/* If the input length is greater than 32, truncate it down to 32 */
	if(length > 32) {
		length = 32;
	}
	/* If length is still greater the file_name length, truncate it down to to the file_name length */
	/* If length is smaller than the file_name length, do nothing */
	if(fname_length < length) {
		length = fname_length;
	}
	/* Copy the string to the buffer */
	strncpy((int8_t*)buf, (int8_t*)&(dentry.file_name), length);
	dentry_offset++;
	return length;
}

// helper function to test directory read
void test_directory_file() {
	uint8_t file_name_buffer[32];
	int index;
	directory_file_open();
	
	directory_file_read(file_name_buffer, 32);
	printf("\n\nDirectory file testing... (There should be only one)");
	printf("\nTesting the read function...\n");
	for(index = 0; index < fname_length; index++) {
		printf("%c", file_name_buffer[index]);
	}
	printf("\n");
}

/* 
 * directory_file_write
 *		DESCRIPTION: Try to write a directory (does nothing and should print out a message showing that calling it won't cause problems)
 *		INPUTS: none  
 *		OUTPUTS: -1
 *		SIDE EFFECT: None
 *
 */
int32_t directory_file_write() {
	/* Print out error message and return */
	printf("... Write function not implemented yet!\n Now returning...!");
	return -1;
}


/* 
 * directory_file_close
 *		DESCRIPTION: Try to close a directory
 *		INPUTS: None
 *		OUTPUTS: Always return 0
 *		SIDE EFFECT: Resets the offset to 0.
 *
 */
int32_t directory_file_close() {
	/* Reset the reading progress (offset) back to 0 */
	dentry_offset = 0;
	printf("File closed. Dentry offset = %d", dentry_offset);
	return 0;
}

/* 
 * print_out_every_file
 *		DESCRIPTION: print out information of every file in the directory
 *		INPUTS: None
 *		OUTPUTS: None
 *		SIDE EFFECT: None
 *
 */
void print_out_every_file() {
	int i;
	int name_char_index;
	int space_index;
	int name_length;
	int digit;
	int digit_count;
	printf("\n");
	for(i = 0; i < filesystem_addr->dentry_count; i++) {
		digit_count = 0;
		printf("file_name: ");
		name_length = strlen((char*)filesystem_addr->dentries[i].file_name);
		if(name_length > 32) {
			name_length = 32;
		}
		for(space_index = 0; space_index < 32 - name_length; space_index++) {
			printf(" ");
		}
		for(name_char_index = 0; name_char_index < name_length; name_char_index++) {
			printf("%c", filesystem_addr->dentries[i].file_name[name_char_index]);
		}
		printf(", file_type: %d", filesystem_addr->dentries[i].file_type);
		digit = ((inode_t *)(inode_start + filesystem_addr->dentries[i].inode * (4 * 1024)))->length_in_b;
		while(digit > 9) {
			digit /= 10;
			digit_count++;
		}
		printf(", file_size: ");
		for(space_index = 0; space_index < 6 - digit_count; space_index++) {
			printf(" ");
		}
		printf("%d", ((inode_t *)(inode_start + filesystem_addr->dentries[i].inode * (4 * 1024)))->length_in_b);
		printf("\n");
	}
}
