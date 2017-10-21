#include "multiboot.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "debug.h"
#include "tests.h"
#include "paging.h"

page_directory_entry page_directory[PAGE_DIRECTORY_SIZE] __attribute__((aligned(4096)));
page_table_entry page_table[PAGE_TABLE_SIZE] __attribute__((aligned(4096)));

void page_directory_init() {
	for(int index = 0; index < PAGE_DIRECTORY_SIZE; index++) {
		page_directory[index].present_flag = 0;
		page_directory[index].read_write_flag = 0;
		page_directory[index].user_supervisor_flag = 0;
		page_directory[index].pwt_flag = 0;
		page_directory[index].pcd_flag = 1;
		page_directory[index].accessed_flag = 0;
		page_directory[index].reserved_flag = 0;
		page_directory[index].page_size_flag = 0;
		page_directory[index].global_page_flag = 0;
		page_directory[index].available_flag = 0;
		page_directory[index].base_address = 0;		
	}
}

void page_table_init() {
	for(int index = 0; index < PAGE_TABLE_SIZE; index++) {
		page_table[index].present_flag = 0;
		page_table[index].read_write_flag = 0;
		page_table[index].user_supervisor_flag = 0;
		page_table[index].pwt_flag = 0;
		page_table[index].pcd_flag = 1;
		page_table[index].accessed_flag = 0;
		page_table[index].dirty_flag = 0;
		page_table[index].page_size_flag = 0;
		page_table[index].global_page_flag = 0;
		page_table[index].available_flag = 0;
		page_table[index].base_address = index;		
	}
}

void paging_init() {
	page_directory_init();
	page_table_init();
	/* need to deal with video memory/kernel/user mode */
}
