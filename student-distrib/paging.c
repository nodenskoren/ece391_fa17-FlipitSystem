#include "multiboot.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "debug.h"
#include "tests.h"
#include "paging.h"

// initializing PD and PT for video memory
uint32_t page_directory[PAGE_DIRECTORY_SIZE] __attribute__((aligned(4096)));
uint32_t video_page_table[PAGE_TABLE_SIZE] __attribute__((aligned(4096)));


// helper function for video page table initialization
void video_table_init(){
	int i = 0;
	// initialize all entry to empty entry(read and write)
	for (i=0; i<PAGE_TABLE_SIZE; i++){
		video_page_table[i] = empty_entry;
	}
	// set the base memory for the entry and set to present and user accessable
	uint32_t temp = ((uint32_t)video_memory_address) >> 12;
	video_page_table[video_memory]= temp | 0x07;


};

// video descriptor is located at 0 entry in page directory, initialize it to user accessable
void video_desc_init(){
	page_directory[0] = ((uint32_t)(video_page_table))>>12 | 0x07;


};

// initialize kernel code discriptor in page directory
void kernel_desc_init(){
	page_directory[1] = 0x400000>>12 | 0x83;
};

// initialize the paging functionality
void paging_init() {
	int i = 0;
	for( i=0; i<PAGE_DIRECTORY_SIZE; i++ ){
		page_directory[i] = empty_entry;

	}
	video_table_init();
	video_desc_init();
	kernel_desc_init();

// set up cr0,3,4
	
	asm volatile(
		"movl $page_directory, %%eax		\n	\
		movl %%eax, %%cr3			\n				\
		movl %%cr0, %%eax			\n				\
		orl  $0x80000001, %%eax	\n	\
		movl %%eax, %%cr0		\n \
		movl %%cr4, %%eax	\n 	\
		orl  $0x00000000, %%eax	\n	\
		movl %%eax, %%cr4"	\ 	


		: \
		: "g"(page_directory) \
		: "eax", "cc", "memory"); \
	
	
};
