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
	page_directory[0].present_flag = 1;
	page_directory[0].read_write_flag = 0;
	page_directory[0].user_supervisor_flag = 1;
	page_directory[0].pwt_flag = 0;
	page_directory[0].pcd_flag = 1; /* not sure */
	page_directory[0].accessed_flag = 0;
	page_directory[0].reserved_flag = 0;
	page_directory[0].page_size_flag = 0;
	page_directory[0].global_page_flag = 0;
	page_directory[0].available_flag = 0;
	page_directory[0].base_address = ((uint32_t)page_table >> 12);
	
	page_directory[1].present_flag = 1;
	page_directory[1].read_write_flag = 1;
	page_directory[1].user_supervisor_flag = 0;
	page_directory[1].pwt_flag = 0;
	page_directory[1].pcd_flag = 1; /* not sure */
	page_directory[1].accessed_flag = 0;
	page_directory[1].reserved_flag = 0;
	page_directory[1].page_size_flag = 0;
	page_directory[1].global_page_flag = 0;
	page_directory[1].available_flag = 0;
	page_directory[1].base_address = 1;	/* not sure */
	
	for(int index = 2; index < PAGE_DIRECTORY_SIZE; index++) {
		page_directory[index].present_flag = 1;
		page_directory[index].read_write_flag = 1;
		page_directory[index].user_supervisor_flag = 1;
		page_directory[index].pwt_flag = 0;
		page_directory[index].pcd_flag = 1; /* not sure */
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
		page_table[index].pcd_flag = 1; /* not sure */
		page_table[index].accessed_flag = 0;
		page_table[index].dirty_flag = 0;
		page_table[index].page_size_flag = 0;
		page_table[index].global_page_flag = 0;
		page_table[index].available_flag = 0;
		page_table[index].base_address = index;		
	}
}

void paging_init() {
	page_table_init();	
	page_directory_init();
	/* need linkage */
}
