#include "multiboot.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "debug.h"
#include "tests.h"
#include "paging.h"

page_directory_entry page_directory[PAGE_DIRECTORY_SIZE] __attribute__((aligned(4096)));
page_table_entry video_page_table[PAGE_TABLE_SIZE] __attribute__((aligned(4096)));


void video_table_init(){
	int i;
	for (i = 0; i < PAGE_TABLE_SIZE; i++){
		
		video_page_table[i].present_flag = 0;
		video_page_table[i].read_write_flag = 1;
		video_page_table[i].user_supervisor_flag = 0;
		video_page_table[i].pwt_flag = 0;
		video_page_table[i].pcd_flag = 0;
		video_page_table[i].accessed_flag = 0;
		video_page_table[i].dirty_flag = 0;
		video_page_table[i].page_size_flag = 0;
		video_page_table[i].available_flag = 0;
		video_page_table[i].global_page_flag = 0;
		video_page_table[i].base_address = 0;
		
		//video_page_table[i] = EMPTY_ENTRY;
	}
}

void video_desc_init(){
	
	page_directory[0].present_flag = 1;
	page_directory[0].user_supervisor_flag = 1;
	page_directory[0].base_address = ((uint32_t)video_page_table >> 12);
	
	//page_directory[0] = (((uint32_t)video_page_table >> 12) | 0x07);	
}

void kernel_desc_init(){
	
	page_directory[1].present_flag = 1;
	page_directory[1].page_size_flag = 1;
	page_directory[1].base_address = (KERNEL_ADDRESS >> 12);
	
	//page_directory[1] = ((KERNEL_ADDRESS >> 12) | 0x83);	
}

void paging_init() {
	int i;
	for(i = 0; i < PAGE_DIRECTORY_SIZE; i++){
		
		page_directory[i].present_flag = 0;
		page_directory[i].read_write_flag = 1;
		page_directory[i].user_supervisor_flag = 0;
		page_directory[i].pwt_flag = 0;
		page_directory[i].pcd_flag = 0;
		page_directory[i].accessed_flag = 0;
		page_directory[i].reserved_flag = 0;
		page_directory[i].page_size_flag = 0;
		page_directory[i].global_page_flag = 0;
		page_directory[i].available_flag = 0;		
		page_directory[i].base_address = 0;
		
		//page_directory[i] = EMPTY_ENTRY;		
	}
	video_table_init();
	video_desc_init();
	kernel_desc_init();
	
	video_page_table[VIDEO_MEMORY].base_address = (VIDEO_MEMORY_ADDRESS >> 12);
	video_page_table[VIDEO_MEMORY].present_flag = 1;
	video_page_table[VIDEO_MEMORY].user_supervisor_flag = 1;

	//video_page_table[VIDEO_MEMORY] = ((VIDEO_MEMORY_ADDRESS >> 12) | 0x07);	

	asm volatile(
		"movl $page_directory, %%eax		\n	\
		movl %%eax, %%cr3			\n				\
		movl %%cr4, %%eax			\n				\
		orl  $0x00000010, %%eax	\n	\
		movl %%eax, %%cr4		\n \
		movl %%cr0, %%eax	\n 	\
		orl  $0x80000001, %%eax	\n	\
		movl %%eax, %%cr0"	\ 	


		: \
		: "g"(page_directory) \
		: "eax", "cc", "memory"); \
}

