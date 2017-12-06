#ifndef PAGING_H_
#define PAGING_H_


#define	PAGE_DIRECTORY_SIZE 1024
#define	PAGE_TABLE_SIZE 1024
#define VIDEO_MEMORY 184
#define VIDEO_MEMORY_ADDRESS 0xB8000
#define KERNEL_ADDRESS 0x400000
#define EMPTY_ENTRY 0x00000002
#define USER_PAGE 32
#define four_mb	0x400000
#define eight_mb 0x800000
#define USER_VID_MAP 0xFFC00000
#define FOUR_KB_MASK 0xFFFFF000
#define FOUR_KB_BINARY_DIGITS 12
#define PD_SIZE_DIGITS 22
#define PT_MASK 0x003FF000
#define FOUR_KB 4096

// bit string structure for page discriptor entry
typedef struct page_directory_entry {
	struct {
		uint32_t	present_flag : 1;
		uint32_t	read_write_flag : 1;
		uint32_t	user_supervisor_flag : 1;
		uint32_t	pwt_flag : 1;
		uint32_t	pcd_flag : 1;
		uint32_t	accessed_flag : 1;
		uint32_t	reserved_flag : 1;
		uint32_t	page_size_flag : 1;
		uint32_t	global_page_flag : 1;
		uint32_t	available_flag : 3;
		uint32_t	base_address : 20;
	}__attribute__((packed));
} page_directory_entry;

// bit string structure for page table entry
typedef struct page_table_entry {
	struct {
		uint32_t	present_flag : 1;
		uint32_t	read_write_flag : 1;
		uint32_t	user_supervisor_flag : 1;
		uint32_t	pwt_flag : 1;
		uint32_t	pcd_flag : 1;
		uint32_t	accessed_flag : 1;
		uint32_t	dirty_flag : 1;
		uint32_t	page_size_flag : 1;
		uint32_t	global_page_flag : 1;
		uint32_t	available_flag : 3;
		uint32_t	base_address : 20;
	}__attribute__((packed));
} page_table_entry;

/* initialize paging descriptors and tables and turn paging on
	 this initialization maps page discriptor table[0] into video memory(4kb) and [1] to kernel memory (4mb)
*/
extern void paging_init();
/* initialize a user page */
extern void user_page_init(uint32_t process_num);
extern void vidmap_page_initialization();
extern void vidmap_desc_init();

#endif
