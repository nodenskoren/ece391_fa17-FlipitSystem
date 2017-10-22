#define	PAGE_DIRECTORY_SIZE 1024
#define	PAGE_TABLE_SIZE 1024
#define page_enable_mask 0x80000000
#define page_trans_enable_mask 0x00000010
#define video_memory 184
#define video_memory_address 0xB8000
#define empty_entry	0x00000002


void paging_init();
