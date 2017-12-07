
#include "paging.h"

page_directory_entry page_directory[PAGE_DIRECTORY_SIZE] __attribute__((aligned(FOUR_KB)));
page_table_entry video_page_table[PAGE_TABLE_SIZE] __attribute__((aligned(FOUR_KB)));
page_table_entry vidmap_page_table[PAGE_TABLE_SIZE] __attribute__((aligned(FOUR_KB)));

/*
 * video_table_init
 *     helper function to initialize video page table to not present
 *     input -- none
 *     output -- none
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
	page_directory[0].base_address = ((uint32_t)video_page_table >> FOUR_KB_BINARY_DIGITS);
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
	page_directory[1].base_address = (KERNEL_ADDRESS >> FOUR_KB_BINARY_DIGITS);

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
	vidmap_page_initialization();
	vidmap_desc_init();
	term_page_int();

	//map video memory table frame onto video page table, we only care about the first 20 bits of the address
	video_page_table[VIDEO_MEMORY].base_address = (VIDEO_MEMORY_ADDRESS >> FOUR_KB_BINARY_DIGITS);
	video_page_table[VIDEO_MEMORY].present_flag = 1;
	video_page_table[VIDEO_MEMORY].user_supervisor_flag = 1;

	//map video memory table frame onto video page table, we only care about the first 20 bits of the address
	/* Calculate the offset of the virtual address in the 2nd level page table */
	uint32_t pt_index = (USER_VID_MAP & PT_MASK) >> FOUR_KB_BINARY_DIGITS;
	/* Set the base address of the page table to the video memory to create mapping */
	vidmap_page_table[pt_index].base_address = (VIDEO_MEMORY_ADDRESS >> FOUR_KB_BINARY_DIGITS);
	vidmap_page_table[pt_index].present_flag = 1;
	vidmap_page_table[pt_index].user_supervisor_flag = 1;

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
	//printf("process_num %d/n",process_num);
		page_directory[USER_PAGE].present_flag = 1;
		page_directory[USER_PAGE].user_supervisor_flag = 1;
		page_directory[USER_PAGE].accessed_flag = 1;
		page_directory[USER_PAGE].page_size_flag = 1;
		// set up the base address for video page table, we only care about the first 20 bits
		page_directory[USER_PAGE].base_address = ( uint32_t)((eight_mb + four_mb*process_num) >> FOUR_KB_BINARY_DIGITS);
		//printf("add %d\n", page_directory[USER_PAGE].base_address);

		//TLB flush
		asm volatile (
 			"movl %%cr3, %%eax		\n\
 		 	 movl %%eax, %%cr3"
 			 : 									\
 		   : 									\
 		   : "memory", "cc");		\

}

/*
 * vidmap_page_initialization
 *     helper function to initialize a new vidmap page table
 *     input -- none
 *     output -- none
 *     side effect -- all entries in vidmap_page_table would be initialized to non-present
 */
void vidmap_page_initialization() {

		int i;
	// loop through the page table and set all the page entry to read write enabled
	for (i = 0; i < PAGE_TABLE_SIZE; i++){

		vidmap_page_table[i].present_flag = 0;
		vidmap_page_table[i].read_write_flag = 1;
		vidmap_page_table[i].user_supervisor_flag = 0;
		vidmap_page_table[i].pwt_flag = 0;
		vidmap_page_table[i].pcd_flag = 0;
		vidmap_page_table[i].accessed_flag = 0;
		vidmap_page_table[i].dirty_flag = 0;
		vidmap_page_table[i].page_size_flag = 0;
		vidmap_page_table[i].available_flag = 0;
		vidmap_page_table[i].global_page_flag = 0;
		vidmap_page_table[i].base_address = 0;
		//video_page_table[i] = EMPTY_ENTRY;
	}
	//vidmap_page_table[0].base_address = (((uint32_t)(USER_VID_MAP) & FOUR_KB_MASK) >> FOUR_KB_BINARY_DIGITS);
}

/*
 * vidmap_desc_init
 *     helper function to initialize vidmap descriptor table to be present and accessable by all
 *     input -- none
 *     output -- none
 *     side effect -- the page directory entry corresponding to the vidmap page would be marked as present, and the base address
 *                    would be set to that of the vidmap_page_table which points to the video memory.
 */
void vidmap_desc_init(){

	/* Calculate the offset of the vidmap inside the page directory */
	uint32_t pd_index = USER_VID_MAP >> PD_SIZE_DIGITS;
	/* Mark the page as present and give the user level process privilage to access, also mark the page as 4KB page */
	page_directory[pd_index].present_flag = 1;
	page_directory[pd_index].accessed_flag = 1;
	page_directory[pd_index].page_size_flag = 0;
	page_directory[pd_index].user_supervisor_flag = 1;
	// set up the base address for video page table, we only care about the first 20 bits
	page_directory[pd_index].base_address = ((uint32_t)vidmap_page_table >> FOUR_KB_BINARY_DIGITS);
	//TLB flush
	asm volatile (
		"movl %%cr3, %%eax		\n\
		 movl %%eax, %%cr3"
		 : 									\
		 : 									\
		 : "memory", "cc");		\


	//page_directory[0] = (((uint32_t)video_page_table >> 12) | 0x07);
}

/*
 * term_page_int
 *     helper function to initialize the backups for terminal VGA
 *     input -- none
 *     output -- none
 *     side effect -- inintialize vidmap table entries for backup terminal VGAs
 */
 void term_page_int()
{
		// initalize terminal 0 vga backup
		video_page_table[term_zero_entry].base_address = (term_zero >> FOUR_KB_BINARY_DIGITS);
		video_page_table[term_zero_entry].present_flag = 1;
		video_page_table[term_zero_entry].user_supervisor_flag = 0;

		// initialize terminal 1 vga backup
		video_page_table[term_one_entry].base_address = (term_one >> FOUR_KB_BINARY_DIGITS);
		video_page_table[term_one_entry].present_flag = 1;
		video_page_table[term_one_entry].user_supervisor_flag = 0;

		// initialize terminal 2 vga backup
		video_page_table[term_two_entry].base_address = (term_two >> FOUR_KB_BINARY_DIGITS);
		video_page_table[term_two_entry].present_flag = 1;
		video_page_table[term_two_entry].user_supervisor_flag = 0;

		// initialize backup vidmap as well
		// initialize terminal 0 vidmap backup
		vidmap_page_table[1].base_address = (term_zero >> FOUR_KB_BINARY_DIGITS);
		vidmap_page_table[1].present_flag = 1;
		vidmap_page_table[1].user_supervisor_flag = 1;
		// initialize terminal 1 vidmap backup
		vidmap_page_table[2].base_address = (term_one >> FOUR_KB_BINARY_DIGITS);
		vidmap_page_table[2].present_flag = 1;
		vidmap_page_table[2].user_supervisor_flag = 1;
		// initialize terminal 2 vidmap backup
		vidmap_page_table[3].base_address = (term_two >> FOUR_KB_BINARY_DIGITS);
		vidmap_page_table[3].present_flag = 1;
		vidmap_page_table[3].user_supervisor_flag = 1;
}

/*
 * term_page_switch
 *     helper function to initialize the backups for terminal VGA
 *     input -- term_num: the terminal number that we want to swtich to
 *     output -- none
 *     side effect -- copy the current vid memory into backup vga
 *     and then copy the back up vga into the current vga
 */

 void term_visible_switch(uint32_t dest)
{
		switch(current_visible)
		{
				case 0:
					memcpy( (void*)term_zero,(void*)VIDEO_MEMORY_ADDRESS,FOUR_KB);
					//clear();
					memcpy((void*)VIDEO_MEMORY_ADDRESS,(void*)((term_zero_entry+dest)*FOUR_KB),FOUR_KB);

					current_visible = dest;
					//term_page_switch(term_num);
					break;
				case 1:
					memcpy( (void*)term_one,(void*)VIDEO_MEMORY_ADDRESS,FOUR_KB);
					//clear();
					memcpy((void*)VIDEO_MEMORY_ADDRESS,(void*)((term_zero_entry+dest)*FOUR_KB),FOUR_KB);
          current_visible = dest;
					//vidmap_page_table[2].base_addr = VIDEO_MEMORY_ADDRESS; =
					break;
				case 2:
					memcpy((void*)term_two,(void*)VIDEO_MEMORY_ADDRESS, FOUR_KB);
					//clear();
					memcpy((void*)VIDEO_MEMORY_ADDRESS,(void*)((term_zero_entry+dest)*FOUR_KB),FOUR_KB);

          current_visible = dest;
					break;

		}

}

/*
 * term_page_switch
 *     helper function to initialize the backups for terminal VGA
 *     input -- term_num: the terminal number that we want to swtich to
 *     output -- none
 *     side effect -- copy the current vid memory into backup vga
 *     and then copy the back up vga into the current vga
 */
 void term_page_switch()
 {
	 if(current_visible == terminal_num)
	 video_page_table[VIDEO_MEMORY].base_address = VIDEO_MEMORY_ADDRESS>>12;
	 //vidmap_page_table[1].base_addr = VIDEO_MEMORY_ADDRESS;
	 else
	 video_page_table[VIDEO_MEMORY].base_address = ((term_zero_entry+terminal_num)*FOUR_KB)>>12;
 
 }
