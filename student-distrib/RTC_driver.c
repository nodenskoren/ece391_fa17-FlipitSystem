#include "x86_desc.h"
#include "lib.h"
#include "RTC_driver.h"

#define REG_B     0x8B
#define REG_A     0x8A
#define REG_C     0x0C
#define REG_NUM_PORT   0x70
#define IO_PORT        0x71
#define IRQ_RTC_NUM    8
#define RATE           0x0F
#define RTC_ENTRY      0x28
#define BIT_6_MASK     0x40
#define HIGH_BYTE_MASK 0xF0
#define DEFAULT_RATE   15
#define MAX_FREQUENCY  1024
#define MIN_FREQUENCY  2
#define SUCCESS        0
#define FAILURE        -1
#define TRUE           1

/* IMPORTANT NOTE: The following code is inspired by OSDev. The web is available at: http://wiki.osdev.org/RTC */


/*
 * RTC_open
 *   DESCRIPTION: Opens the RTC file. Sets the frequency to 2Hz default.
 *   INPUTS: filename - filename of RTC
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
 
//volatile int RTC_flag = 0;
 
int32_t RTC_open(const uint8_t* filename){
	
	/* Fills in IDT entry for RTC */
	idt[RTC_ENTRY].seg_selector = KERNEL_CS;
	idt[RTC_ENTRY].reserved4   = 0x0; 
	idt[RTC_ENTRY].reserved3   = 0x0;  /* referenced from x86 ISA Manual */
	idt[RTC_ENTRY].reserved2   = 0x1;
	idt[RTC_ENTRY].reserved1   = 0x1;
	idt[RTC_ENTRY].size        = 0x1;  /* 32 bit */
	idt[RTC_ENTRY].reserved0   = 0x0;
	idt[RTC_ENTRY].dpl         = 0x0;  /* kernel priviledged mode */
	idt[RTC_ENTRY].present     = 0x1;
	SET_IDT_ENTRY(idt[RTC_ENTRY], rtc_wrapper);
	
	/* Enable IRQ8  */
	outb(REG_B, REG_NUM_PORT);		           // select register B, and disable NMI	
	char prev = inb(IO_PORT);    	           // read the current value of register B
	outb(REG_B, REG_NUM_PORT);		           // select register B, and disable NMI
	outb(prev | BIT_6_MASK, IO_PORT);     	       // write the previous value ORed with 0x40. This turns on bit 6 of register B
	
	
    /* Set frequency to 2Hz */
    outb(REG_A, REG_NUM_PORT);		           // set index to register A, disable NMI
    prev = inb(IO_PORT);              	       // get initial value of register A
    outb(REG_A, REG_NUM_PORT);		           // reset index to A
    outb(((prev & HIGH_BYTE_MASK) | RATE) , IO_PORT);    // write rate 2Hz to regsiter A
	
	enable_irq(IRQ_RTC_NUM);
	
	return SUCCESS;
}

/*
 * RTC_interrupt_handler
 *   DESCRIPTION: Interrupt handler for RTC.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: prints out stuff onto screen showing that RTC interrupt works
 */

void RTC_interrupt_handler(){
	uint32_t flags;	
	/* Enable another RTC interrupt */

	outb(REG_C, REG_NUM_PORT);            // select register C
	cli_and_save(flags);                                // make sure the loading is protected
	inb(IO_PORT);                         // just throw away contents
	terminal[0].RTC_flag = 0;
	terminal[1].RTC_flag = 0;
	terminal[2].RTC_flag = 0;
	restore_flags(flags);        
	
	send_eoi(IRQ_RTC_NUM);                // send EOI

}

/*
 * RTC_read
 *   DESCRIPTION: Blocks until the next RTC interrupt comes.
 *   INPUTS: fd - file descriptor
			 buf
			 nbytes - frequency to be written to RTC
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */

int32_t RTC_read(int32_t fd, void* buf, int32_t nbytes){
	uint32_t flags;
	cli_and_save(flags);    
	terminal[terminal_num].RTC_flag = 1;  // sets the flag to be cleared by another RTC inpterrupt
	restore_flags(flags);  
 	while(terminal[terminal_num].RTC_flag){
		//printf("%d", RTC_flag);
	}

    return 0;
}

/*
 * RTC_write
 *   DESCRIPTION: changes the frequency of RTC interrupts. 
 *   INPUTS: filename 
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */

int32_t RTC_write(int32_t fd, const uint32_t* buf, int32_t nbytes){
	
	if(buf == NULL)    // check if the buf is null
		return FAILURE;
	
	if(nbytes <= 0)    // check if nbytes is a positive number
		return FAILURE;
	
	int frequency = *buf;
	
	if( frequency > MAX_FREQUENCY || frequency < MIN_FREQUENCY) // check if the frequency is within the bound
		return FAILURE;
	
	if((frequency & (frequency - 1)) != 0) // check if frequency is power of 2
		return FAILURE;
	
	unsigned int rate = DEFAULT_RATE;
	while(TRUE){
		if(frequency == MIN_FREQUENCY)
			break;
		frequency = frequency / MIN_FREQUENCY;   // divides the frequency by 2
		rate--;
		
	}

    outb(REG_A, REG_NUM_PORT);		       // set index to register A, disable NMI
    char prev = inb(IO_PORT);              // get initial value of register A
    outb(REG_A, REG_NUM_PORT);		       // reset index to A
    outb(((prev & HIGH_BYTE_MASK) | rate ) , IO_PORT);    // write rate 2Hz to regsiter A	
	
	
	return SUCCESS;
	
	
}

/*
 * RTC_close
 *   DESCRIPTION: closes the RTC file.
 *   INPUTS: ffd - file descriptor
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */

int32_t RTC_close(int32_t fd){
	
	return SUCCESS;
	
}



