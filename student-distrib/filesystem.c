#include "lib.h"
#include "filesystem.h"
#include "systemcall.h"
#include "scheduler.h"

static boot_block* filesystem_addr;
//static uint32_t file_offset;
//static uint32_t dentry_offset;
static uint32_t fname_length;
static uint32_t data_block_start;
unsigned int boot_addr;
unsigned int dentry_addr;
uint32_t inode_start;
extern pcb_t* active_process;

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
	
	/* If length is greater than 32 file name is not valid, return fail */
	if(fname_length > 32 || fname_length == 0) {
		return FAILURE;
	}
	
	/* Loop over every directory entry and search by name */
	for(i = 0; i < DENTRY_MAX_SIZE; i++){
		/* Create a local dentry structure and copy the directory info of current entry into it */
		dentry_t curr_dentry = filesystem_addr->dentries[i];
		
		if(fname_length != strlen((char*)curr_dentry.file_name) && fname_length != 32) {
			continue;
		}
		/*
		int j;
		for (j = 0; j < strlen((char*)curr_dentry.file_name); j++) {
			printf("%c", curr_dentry.file_name[j]);
		}
		printf("\n");
		printf("%d\n", strlen((char*)curr_dentry.file_name));
		printf("%d\n", fname_length);*/
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

/* 
 * test_read_dentry
 *		DESCRIPTION: Test dentry search with oversized file name
 *		INPUTS: none              
 *		OUTPUTS: none
 *		SIDE EFFECT: none
 *
 */
void test_read_dentry() {
	clear();
	/* Create local dentry structure */
	dentry_t dentry;
	int i;
	char *p = "verylargetextwithverylongname.txt";
	/* Search the dentry list by name and copy the dentry into local structure */
	read_dentry_by_name((uint8_t*)p, &(dentry));
	/* Print out the filename to check if the copied entry is correct */
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
 * test_read_index
 *		DESCRIPTION: Test dentry search with index
 *		INPUTS: none              
 *		OUTPUTS: none
 *		SIDE EFFECT: none
 *
 */
void test_read_index() {
	/* Create local dentry structure */
	dentry_t dentry;
	int i;
	/* Search the dentry list by index and copy the dentry into local structure */
	read_dentry_by_index(3, &(dentry));
	/* Print out the filename to check if the copied entry is correct */
	printf("\n\nFinding file corresponding to index 3:\n");
	printf("file_name: ");
	for(i = 0; i < fname_length; i++) {
		printf("%c", dentry.file_name[i]);
	}
	
	/* Test on non-existing dentry */
	printf("\n\nTesting on non-existing d_entry:\n");
	if(read_dentry_by_index(50, &(dentry)) == FAILURE) {
		printf("Invalid index!\n");
	}
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
		return FAILURE;
	}
	
	inode_t* current_inode = (inode_t*)(inode_start + inode * block_size);
	if(offset >= current_inode->length_in_b) {
		return END_OF_FILE;
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

/* 
 * test_read_file
 *		DESCRIPTION: Test read on text file
 *		INPUTS: none              
 *		OUTPUTS: none
 *		SIDE EFFECT: none
 *
 */
void test_read_file() {
	clear();
	//printf("Reading file test_case ...\n");
	/* Create local dentry structure and buffers for contents and filename */
	dentry_t test_dentry; //declare a dentry to fetch result from read file function
	uint8_t buffer[BUFFER_SIZE_TEST];
	uint8_t file_name_buffer[FILENAME_MAX];
	extern uint32_t inode_start;
	char *p = "frame0.txt";
	
	/* Check if the file exist */
	if (read_dentry_by_name((uint8_t* )p, &test_dentry) == -1) {	
		printf("Invalid read!\n");
		return;
	}

	//printf("The file type is: %d\n", test_dentry.file_type); //print file type
	//printf("The file inode number is: %d\n", test_dentry.inode); //print file inode number

	/* Read data in the data block */
	if (read_data(test_dentry.inode, 0, buffer, BUFFER_SIZE_TEST) <= 0) {
		printf("\nInvalid read_data or end of file!\n");
		return;
	}
	/* print out the data length */
	//printf("\nThe file length is: %d\n", ((inode_t *)(inode_start + test_dentry.inode * block_size))->length_in_b);
	int i;
	/* Print data in the file */
	for (i = 0; i < ((inode_t *)(inode_start + test_dentry.inode * block_size))->length_in_b; i++) {
		printf("%c", buffer[i]);
	}
	/* Copy and print the file name */
	memcpy((void*)file_name_buffer, (void*)&(test_dentry.file_name), fname_length);	
	printf("\n");
	printf("file_name: ");
	for (i = 0; i < fname_length; i++) {
		printf("%c", file_name_buffer[i]);
	}
	return;
}

/* 
 * test_read_file_non_text
 *		DESCRIPTION: Test read on non-text file
 *		INPUTS: none              
 *		OUTPUTS: none
 *		SIDE EFFECT: none
 *
 */
void test_read_file_non_text() {
	clear();
	//printf("Reading file test_case ...\n");
	/* Create local dentry structure and buffers for contents and filename */
	dentry_t test_dentry; //declare a dentry to fetch result from read file function
	uint8_t buffer[BUFFER_SIZE_TEST];
	uint8_t file_name_buffer[FILENAME_MAX];
	extern uint32_t inode_start;
	char *p = "hello";
	
	/* Check if the file exist */
	if (read_dentry_by_name((uint8_t* )p, &test_dentry) == -1) {	
		printf("Invalid read!\n");
		return;
	}

	//printf("The file type is: %d\n", test_dentry.file_type); //print file type
	//printf("The file inode number is: %d\n", test_dentry.inode); //print file inode number

	/* Read data in the data block */
	if (read_data(test_dentry.inode, 0, buffer, BUFFER_SIZE_TEST) <= 0) {
		printf("\nInvalid read_data or end of file!\n");
		return;
	}
	//print out the data length
	//printf("\nThe file length is: %d\n", ((inode_t *)(inode_start + test_dentry.inode * block_size))->length_in_b);
	int i;
	/* Print data in the file */
	for (i = 0; i < ((inode_t *)(inode_start + test_dentry.inode * block_size))->length_in_b; i++) {
		printf("%c", buffer[i]);
	}
	/* Copy and print the file name */
	memcpy((void*)file_name_buffer, (void*)&(test_dentry.file_name), fname_length);
	printf("\n");
	printf("file_name: ");
	for (i = 0; i < fname_length; i++) {
		printf("%c", file_name_buffer[i]);
	}
	//printf("\nread_data success!\n");
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
int32_t regular_file_open(const uint8_t* fname) {
	if(fname != NULL) {
		//file_offset = 0;
		return SUCCESS;
	}
	return FAILURE;
}


/* 
 * regular_file_read
 *		DESCRIPTION: read data from a file
 *		INPUTS: fd - The index of the file_struct inside the file descriptor table
 *              buf - the buffer to store the read data
 *              count - number of bytes to be read
 *		OUTPUTS: number of bytes read, -1 if fail
 *		SIDE EFFECT: read data into buffer
 *
 */
int32_t regular_file_read(int32_t fd, uint8_t* buf, uint32_t count) {
	/* Define a local dentry */
	//dentry_t dentry;
	/* Call the read_dentry_by_name function to find the file info and copy it to the local dentry */
	//read_dentry_by_name(fname, &(dentry));
	/* If the buffer doesn't exist or if the filetype isn't supported by this method, return with error message */
	if((buf == NULL) /*|| (dentry.file_type != REGULAR_FILE)*/) {
		printf("INVALID BUFFER!\n");
		return -1;
	}
	int32_t length_read;
	/* Read the data by calling read_data function */
	length_read = read_data(terminal[terminal_num].active_process->file_descriptor_table[fd].inode, terminal[terminal_num].active_process->file_descriptor_table[fd].f_offset, buf, count);
	/* Add the progress read */
	//file_offset += length_read;
	terminal[terminal_num].active_process->file_descriptor_table[fd].f_offset += length_read;
	return length_read;
}

/* 
 * test_regular_file
 *		DESCRIPTION: Test open/read/write/close functions of regular file.
 *		INPUTS: none              
 *		OUTPUTS: none
 *		SIDE EFFECT: none
 *
 */
void test_regular_file() {
	/*
	clear();
	dentry_t d_entry;
	read_dentry_by_name((uint8_t*)"frame0.txt", &(d_entry));
	if(regular_file_open((uint8_t* )"frame0.txt") != 0) {
		printf("Invalid file...\n");
		return;
	}
	uint8_t buffer[BUFFER_SIZE_TEST];
	uint8_t file_name_buffer[FILE_NAME_MAX_LENGTH];
	int i;
	memcpy((void*)file_name_buffer, (void*)&(d_entry.file_name), fname_length);
	printf("Regular file testing...");
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
	regular_file_read((uint8_t*)"frame0.txt", buffer, 33);
	for (i = 0; i < 33; i++) {
		printf("%c", buffer[i]);
	}
	regular_file_write();
	regular_file_close();
	return; */
}

/* 
 * regular_file_write
 *		DESCRIPTION: try to write to a file(not implemented)
 *		INPUTS: fd - The index of the file_struct inside the file descriptor table
 *              buf - the buffer to store the write data
 *              count - number of bytes to be write to the file        
 *		OUTPUTS: -1
 *		SIDE EFFECT: none
 *
 */
int32_t regular_file_write(int32_t fd, const uint8_t* buf, uint32_t count) {
	/* Print out error message and return */
	printf("... Write function not implemented yet!\nNow returning...!\n");
	return SUCCESS;
}

/* 
 * regular_file_close
 *		DESCRIPTION: try to close a file
 *		INPUTS: fd - the index of the file descriptor table to close
 *              
 *		OUTPUTS: 0
 *		SIDE EFFECT: none
 *
 */
int32_t regular_file_close(int32_t fd) {
	/* Reset the reading progress (offset) back to 0 */
	//file_offset = 0;
	printf("File closed.\n");
	return SUCCESS;
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
	//dentry_offset = 0;
	return SUCCESS;
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
int32_t directory_file_read(int32_t fd, uint8_t* buf, uint32_t length) {
	dentry_t dentry;
	/* If the buffer doesn't exist or if the directory entry doesn't exist, return -1 */
	if((buf == NULL) || (read_dentry_by_index(terminal[terminal_num].active_process->file_descriptor_table[fd].f_offset, &(dentry)) == -1)) {
		return FAILURE;
	}
	/* If EOF has been reached, read nothing and return 0 */
	if(terminal[terminal_num].active_process->file_descriptor_table[fd].f_offset >= filesystem_addr->dentry_count) {
		return SUCCESS;
	}
	/* If the input length is greater than 32, truncate it down to 32 */
	if(length > FILENAME_MAX) {
		length = FILENAME_MAX;
	}
	/* If length is still greater the file_name length, truncate it down to to the file_name length */
	/* If length is smaller than the file_name length, do nothing */
	if(fname_length < length) {
		length = fname_length;
	}
	/* Copy the string to the buffer */
	strncpy((int8_t*)buf, (int8_t*)&(dentry.file_name), length);
	terminal[terminal_num].active_process->file_descriptor_table[fd].f_offset += 1;
	//dentry_offset++;
	return length;
}

/* 
 * test_directory_file
 *		DESCRIPTION: Test open/read/write/close functions of directory file.
 *		INPUTS: None              
 *		OUTPUTS: None
 *		SIDE EFFECT: None
 *
 */
void test_directory_file() {
	/*
	clear();

	uint8_t file_name_buffer[FILENAME_MAX];
	int index;
	

	directory_file_open();

	directory_file_read(file_name_buffer, FILENAME_MAX);
	printf("Directory file testing... (There should be only one)");
	printf("\nTesting the read function...\n");

	for(index = 0; index < fname_length; index++) {
		printf("%c", file_name_buffer[index]);
	}
	printf("\n");
	directory_file_write();
	directory_file_close();
	*/
}

/* 
 * directory_file_write
 *		DESCRIPTION: Try to write a directory (does nothing and should print out a message showing that calling it won't cause problems)
 *		INPUTS: none  
 *		INPUTS: fd - The index of the file_struct inside the file descriptor table
 *              buf - the buffer to store the write data
 *              count - number of bytes to be write to the file
 *		OUTPUTS: -1
 *		SIDE EFFECT: None
 *
 */
int32_t directory_file_write(int32_t fd, const uint8_t* buf, uint32_t count) {
	/* Print out error message and return */
	printf("... Write function not implemented yet!\nNow returning...!\n");
	return SUCCESS;
}

/* 
 * directory_file_close
 *		DESCRIPTION: Try to close a directory
 *		INPUTS: fd - the index of the file descriptor table to close
 *		OUTPUTS: Always return 0
 *		SIDE EFFECT: Resets the offset to 0.
 *
 */
int32_t directory_file_close(int32_t fd) {
	/* Reset the reading progress (offset) back to 0 */
	//dentry_offset = 0;
	printf("File closed.\n");
	return SUCCESS;
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
	clear();
	int i;
	int name_char_index;
	int space_index;
	int name_length;
	int digit;
	int digit_count;
	/* Loop over every directory entry with information */
	for(i = 0; i < filesystem_addr->dentry_count; i++) {
		digit_count = 0;
		printf("file_name: ");
		name_length = strlen((char*)filesystem_addr->dentries[i].file_name);
		if(name_length > FILE_NAME_MAX_LENGTH) {
			name_length = FILE_NAME_MAX_LENGTH;
		}
		/* Allocate space for 32 characters to be displayed */
		/* Display the file_name and fill the remaining spaces with space character */
		for(space_index = 0; space_index < FILE_NAME_MAX_LENGTH - name_length; space_index++) {
			printf(" ");
		}
		for(name_char_index = 0; name_char_index < name_length; name_char_index++) {
			printf("%c", filesystem_addr->dentries[i].file_name[name_char_index]);
		}
		/* Print out the file type */
		printf(", file_type: %d", filesystem_addr->dentries[i].file_type);
		/* Calculate the digit of file_size */
		/* Allocate space for 6 digits of numbers to be displayed */
		/* Display the file_size and fill the remaining spaces with space character */
		digit = ((inode_t *)(inode_start + filesystem_addr->dentries[i].inode * (BLOCKSIZE)))->length_in_b;
		while(digit > SINGLE_DIGIT_LARGEST_NUMBER) {
			digit /= DOUBLE_DIGIT_SMALLEST_NUMBER;
			digit_count++;
		}
		printf(", file_size: ");
		for(space_index = 0; space_index < FILE_SIZE_MAX_LENGTH - digit_count; space_index++) {
			printf(" ");
		}
		printf("%d", ((inode_t *)(inode_start + filesystem_addr->dentries[i].inode * (BLOCKSIZE)))->length_in_b);
		printf("\n");
	}
}

