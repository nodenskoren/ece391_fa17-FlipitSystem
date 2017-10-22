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

/* IMPORTANT NOTE: The following code is inspired by OSDev. The web is available at: http://wiki.osdev.org/RTC */


/*
 * initialize_RTC_driver
 *   DESCRIPTION: Initialize the driver for RTC 
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: Fills in the IDT entry for RTC, enable IRQ8 and set up frequency
 */
 
void initialize_RTC_driver(){
	
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
	SET_IDT_ENTRY(idt[RTC_ENTRY], RTC_interrupt_handler);
	
	/* Enable IRQ8  */
	outb(REG_B, REG_NUM_PORT);		           // select register B, and disable NMI	
	char prev = inb(IO_PORT);    	           // read the current value of register B
	outb(REG_B, REG_NUM_PORT);		           // select register B, and disable NMI
	outb(prev | 0x40, IO_PORT);     	       // write the previous value ORed with 0x40. This turns on bit 6 of register B
	
	
    /* Set frequency to 2Hz */
    outb(REG_A, REG_NUM_PORT);		           // set index to register A, disable NMI
    prev = inb(IO_PORT);              	       // get initial value of register A
    outb(REG_A, REG_NUM_PORT);		           // reset index to A
    outb(((prev & 0xF0) | RATE) , IO_PORT);    // write rate 2Hz to regsiter A
	
	enable_irq(IRQ_RTC_NUM);
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
	
	//test_interrupts(); //test cases for RTC interrupt
	
	/* Enable another RTC interrupt */
	outb(REG_C, REG_NUM_PORT);            // select register C
	cli();                                // make sure the loading is protected
	inb(IO_PORT);                         // just throw away contents
	sti();                                
	
	send_eoi(IRQ_RTC_NUM);                // send EOI

}
