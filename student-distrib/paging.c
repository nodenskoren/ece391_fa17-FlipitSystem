#include "multiboot.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "debug.h"
#include "tests.h"
#include "paging.h"

page_directory_entry page_directory[PAGE_DIRECTORY_SIZE] __attribute__((aligned(4096)));
page_table_entry video_page_table[PAGE_TABLE_SIZE] __attribute__((aligned(4096)));


/* helper function to initialize video page table to not present
 * input -- none
 * output -- none
 */
void video_table_init(){
	int i;
	// loop through the page table and set all the page entry to read write enabled
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

/* video_desc_init()
 * helper function to initialize video descriptor table to be present and accessable by all
 * input -- none
 * output -- none
 */
void video_desc_init(){

	page_directory[0].present_flag = 1;
	page_directory[0].user_supervisor_flag = 1;
	// set up the base address for video page table, we only care about the first 20 bits
	page_directory[0].base_address = ((uint32_t)video_page_table >> 12);
	//TLB flush
	asm volatile (
		"movl %%cr3, %%eax		\n\
		 movl %%eax, %%cr3"
		 : 									\
		 : 									\
		 : "memory", "cc");		\


	//page_directory[0] = (((uint32_t)video_page_table >> 12) | 0x07);
}

/* kernel_desc_init()
 * helper function to initialize kernel descriptor table to be present and not accessable by user
 * input -- none
 * output -- none
 */
void kernel_desc_init(){

	page_directory[1].present_flag = 1;
	page_directory[1].page_size_flag = 1;
	// set up kernel page (4mb) and base address we only care about the first 20 bits of the memory
	page_directory[1].base_address = (KERNEL_ADDRESS >> 12);

	//TLB flush
	asm volatile (
		"movl %%cr3, %%eax		\n\
		 movl %%eax, %%cr3"
		 : 									\
		 : 									\
		 : "memory", "cc");		\


	//page_directory[1] = ((KERNEL_ADDRESS >> 12) | 0x83);
}


/* paging_init()
 * initialize paging descriptors and tables and turn paging on
 * input -- none
 * output -- none
 */
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
	// intialize the page descriptor table
	video_table_init();
	video_desc_init();
	kernel_desc_init();

	//map video memory table frame onto video page table, we only care about the first 20 bits of the address
	video_page_table[VIDEO_MEMORY].base_address = (VIDEO_MEMORY_ADDRESS >> 12);
	video_page_table[VIDEO_MEMORY].present_flag = 1;
	video_page_table[VIDEO_MEMORY].user_supervisor_flag = 1;

	//video_page_table[VIDEO_MEMORY] = ((VIDEO_MEMORY_ADDRESS >> 12) | 0x07);

	// using asm to push the values into correct registers
	asm volatile(
		"movl $page_directory, %%eax		\n\
		movl %%eax, %%cr3			\n\
		movl %%cr4, %%eax			\n\
		orl  $0x00000010, %%eax	\n\
		movl %%eax, %%cr4		\n\
		movl %%cr0, %%eax	\n\
		orl  $0x80000001, %%eax	\n\
		movl %%eax, %%cr0"


		: \
		: "g"(page_directory) \
		: "eax", "cc", "memory"); \
}

/* user_page_add(uint32_t address, uint32_t process_num)
 * initialize a user page
 * input -- none
 * output -- index for the free page descriptor
 */
void user_page_init(uint32_t process_num)
{
		page_directory[USER_PAGE].present_flag = 1;
		page_directory[USER_PAGE].user_supervisor_flag = 1;
		// set up the base address for video page table, we only care about the first 20 bits
		page_directory[USER_PAGE].base_address = ( uint32_t)((eight_mb + four_mb*process_num) >> 12);

		//TLB flush
		asm volatile (
 			"movl %%cr3, %%eax		\n\
 		 	 movl %%eax, %%cr3"
 			 : 									\
 		   : 									\
 		   : "memory", "cc");		\

}
