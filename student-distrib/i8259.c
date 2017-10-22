/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */
unsigned long flags;

/* Initialize the 8259 PIC */
void i8259_init(void) {
	cli_and_save(flags); 	
	/* executes the initialization sequence */
	/* for master PIC */
	outb(ICW1, MASTER_8259_PORT);
	outb(ICW2_MASTER, MASTER_8259_DATA);
	outb(ICW3_MASTER, MASTER_8259_DATA);
	outb(ICW4, MASTER_8259_DATA);
	/* for slave PIC */	
	outb(ICW1, SLAVE_8259_PORT);
	outb(ICW2_SLAVE, SLAVE_8259_DATA);
	outb(ICW3_SLAVE, SLAVE_8259_DATA);
	outb(ICW4, SLAVE_8259_DATA);
	
	/* restores the mask settings */
	//outb(MASTER_8259_DATA, master_mask);
	//outb(SLAVE_8259_DATA, slave_mask);
	
	outb(0xFF, MASTER_8259_DATA);
	outb(0xFF, SLAVE_8259_DATA);
	restore_flags(flags);
	
	enable_irq(2); // irq 2 is slave pic
	
}

/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num) {
	
	cli_and_save(flags); 
	uint8_t interrupt_unmask;
	if(irq_num >= 0 && irq_num <= 7) {
		interrupt_unmask = ~(0x01 << irq_num);
		master_mask = inb(MASTER_8259_DATA) & interrupt_unmask;
		outb(master_mask, MASTER_8259_DATA);
	}
	else if(irq_num >= 8 && irq_num <= 15) {
		interrupt_unmask = ~(0x01 << (irq_num - 8));
		slave_mask = inb(SLAVE_8259_DATA) & interrupt_unmask;
		outb(slave_mask, SLAVE_8259_DATA);
	}
	restore_flags(flags);
	
}

/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num) {
	
	cli_and_save(flags); 
	uint8_t interrupt_mask;
	if(irq_num >= 0 && irq_num <= 7) {
		interrupt_mask = (0x01 << irq_num);
		master_mask = inb(MASTER_8259_DATA) | interrupt_mask;
		outb(master_mask, MASTER_8259_DATA);
	}
	else if(irq_num >= 8 && irq_num <= 15) {
		interrupt_mask = (0x01 << (irq_num - 8));
		slave_mask = inb(SLAVE_8259_DATA) | interrupt_mask;
		outb(slave_mask, SLAVE_8259_DATA);
	}
	restore_flags(flags);
	
}

/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {
	
	//cli_and_save(flags); 
	if(irq_num >= 0 && irq_num <= 7) {
		outb((EOI | irq_num), MASTER_8259_PORT);
	}
	else if(irq_num >= 8 && irq_num <= 15) {
		outb((EOI | 2), MASTER_8259_PORT);
		outb((EOI | (irq_num - 8)), SLAVE_8259_PORT);
	}
	//restore_flags(flags);
	
}
