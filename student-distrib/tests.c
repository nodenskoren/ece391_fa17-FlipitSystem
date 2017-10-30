#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "terminal_driver.h"
#include "RTC_driver.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 *
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) &&
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

// helper function for paging test
void paging_test(){

	// access kernel memory
	int *c = (int*) 0x400000;
	*c = 3;
	printf("paging result= %d\n", *c);
	// access 0x0 
	int *d = NULL;
	printf("paging result= %d\n", *d);


}


// add more tests here

/* Checkpoint 2 tests */

/*echo
 *Description: reads from terminal and echo's back to terminal
 *Input: NONE
 *Output: NONE
 *Side Effect: reads last command from terminal and writes it back to terminal
 */
void echo(){
   while(1){
     char buf[128];
     int bytes_recieved;
     bytes_recieved = terminal_read(buf,128);
     terminal_write("echo: ",6);
     terminal_write(buf,bytes_recieved);

  }
}

/* RTC_test
 * 
 * Test that open, read, write and close for RTC is working
 * Inputs: None
 * Outputs: prints 1 for different frequencies
 * Side Effects: None
 * Coverage: open, read, write and close for RTC
 * Files: RTC_driver.h
 */
void RTC_test(){
	void* buf;
	RTC_open((uint8_t*)"rtc");
	uint32_t freq = 2;
	RTC_write(0, &freq, 4);
	RTC_read(0, buf, 0);
	RTC_close(0);		
}

/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */
extern void test_read_file();
extern void test_regular_file();
extern void test_directory_file();
extern void test_read_dentry();
extern void print_out_every_file();
extern void test_read_file_non_text();
extern void test_read_index();
/* Test suite entry point */
/* Test suite entry point */
void launch_tests(){
	//TEST_OUTPUT("idt_test", idt_test());
	//printf("hello world");
	// launch your tests here
	//paging_test();
	//echo();
	
	//test_read_dentry();

	/* Piazza demo 1 */
	print_out_every_file();	
	
	/* Piazza demo 2 */
	//test_read_file();

	//test_read_file_non_text();
	//test_regular_file();
	//test_directory_file();
	
	/* Piazza demo 3 */
	//RTC_test();
	
	/* Piazza demo 5 */
	test_read_index();
}
