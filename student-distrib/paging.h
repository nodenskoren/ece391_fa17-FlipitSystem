#define	PAGE_DIRECTORY_SIZE 1024
#define	PAGE_TABLE_SIZE 1024
#define VIDEO_MEMORY 184
#define VIDEO_MEMORY_ADDRESS 0xB8000
#define KERNEL_ADDRESS 0x400000
#define EMPTY_ENTRY 0x00000002

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

extern void paging_init();
