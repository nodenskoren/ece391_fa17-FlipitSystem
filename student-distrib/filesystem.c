#include "lib.h"
#include "filesystem.h"

static boot_block* filesystem_addr;
static inode_t* inode_field_starting_addr;
static data_block_t* data_field_starting_addr;
static dentry_t* file_dir;
static uint32_t file_offset;

int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry) {
	if(fname == NULL || dentry == NULL) {
		return -1;
	}
	
	int dentry_index;
	for(dentry_index = 0; dentry_index < filesystem_addr->dentry_count; dentry_index++) {
		dentry_t* curr_dentry = &(filesystem_addr->dentries[dentry_index]);
		if(strncmp((uint8_t*)fname, (uint8_t*)curr_dentry->file_name, 32) == 0) {
			strncpy((uint8_t*)dentry->file_name, (uint8_t)curr_dentry->file_name, 32);
			dentry.file_type = curr_dentry.file_type;
			dentry.inode = curr_dentry.inode;
			return 0;
		}
	}
	
	return -1;
}

int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry) {
	if(index < filesystem_addr.dentry_count && dentry != NULL) {
		dentry_t* curr_dentry = &(boot_block->dentries[index]);
		dentry.file_type = curr_dentry.file_type;
		dentry.inode = curr_dentry.inode;
		return 0;
	}
	return -1;
}
	

int32_t regular_file_open(uint8_t* fname) {
	if(fname != NULL) {
		read_dentry_by_name(fname, file_dir);
		file_offset = 0;
		return 0;
	}
	return -1;
}

int32_t regular_file_read(uint8_t* buf, uint32_t count) {	
	if(file_dir.file_type != REGULAR_FILE) {
		/* Invalid file type for this method */
		return -1;
	}
	file_offset += read_data(file_dir.inode, file_offset, buf, count);
	return 0;
}

int32_t regular_file_write() {
	return -1;
}

int32_t regular_file_close() {
	return 0;
}

int32_t directory_file_open(uint8_t* fname) {
	if(fname != NULL) {
		read_dentry_by_name(fname, file_dir);
		file_offset = 0;
		return 0;
	}
	return -1;
}

int32_t directory_file_read(uint8_t* buf, uint32_t length) {
	if(file_dir.file_type != DIRECTORY_FILE) {
		/* Invalid file type for this method */
		return -1;
	}
	if(file_offset >= filesystem_addr->dentry_count) {
		return -1;
	}
	if(read_dentry_by_index(file_offset, file_dir) != 0) {
		return -1;
	}	
	strncpy((uint8_t*)buf, (uint8_t*)file_dir.file_name, 32);
	file_offset++;
	return 0;
}

int32_t directory_file_write() {
	return -1;
}

int32_t directory_file_close() {
	return 0;
}