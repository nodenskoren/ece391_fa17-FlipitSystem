/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"
//#include <linux/spinlock.h>

/* Interrupt masks to determine which interrupts are enabled and disabled */
/* Initialized so that all IRQs were initially masked */
uint8_t master_mask = 0xFF; /* IRQs 0-7  */
uint8_t slave_mask = 0xFF;  /* IRQs 8-15 */
uint32_t flags;
//spinlock_t irq_lock;


/* Initialize the 8259 PIC */

/* 
 * After masking interrupts on the processor and acquiring a lock, 
 * the code masks all interrupts (on both PICs),
 * executes the initialization sequence,
 * restores the mask settings,
 * releases the lock, and restores the IF flag.
 * 
 * The first word, ICW1, is delivered to the first PIC port—either 0x20 or 0xA0—and tells the PIC that it is being initialized
 * The remaining ICWs are written to the second port.
 *
 */
void i8259_init(void) {
	
	/* masking interrupts on the processor and acquiring a lock */

	cli_and_save(flags); 	
	/* executes the initialization sequence */
	/* for master PIC */
	outb(MASTER_8259_PORT, ICW1);
	outb(MASTER_8259_DATA, ICW2_MASTER);
	outb(MASTER_8259_DATA, ICW3_MASTER);
	outb(MASTER_8259_DATA, ICW4);
	/* for slave PIC */	
	outb(SLAVE_8259_PORT, ICW1);
	outb(SLAVE_8259_DATA, ICW2_SLAVE);
	outb(SLAVE_8259_DATA, ICW3_SLAVE);
	outb(SLAVE_8259_DATA, ICW4);
	
	/* restores the mask settings */
	//outb(MASTER_8259_DATA, master_mask);
	//outb(SLAVE_8259_DATA, slave_mask);
	
	outb(MASTER_8259_DATA, 0xFF);
	outb(SLAVE_8259_DATA, 0xFF);
	restore_flags(flags);
	
	enable_irq(2); // irq 2 is slave pic
}

/* NODENS HAVEN'T COMMENTED YET */
/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num) {
	cli_and_save(flags); 
	uint8_t interrupt_unmask;
	if(irq_num >= 0 && irq_num <= 7) {
		interrupt_unmask = ~(0x01 << irq_num);
		master_mask = master_mask & interrupt_unmask;
		outb(MASTER_8259_DATA, master_mask);
	}
	else if(irq_num >= 8 && irq_num <= 15) {
		//enable_irq(2); // irq 2 is slave pic
		interrupt_unmask = ~(0x01 << (irq_num - 8));
		slave_mask = slave_mask & interrupt_unmask;
		outb(SLAVE_8259_DATA, slave_mask);
	}
	restore_flags(flags);
}

/* NODENS HAVEN'T COMMENTED YET */
/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num) {
	cli_and_save(flags); 
	uint8_t interrupt_mask;
	if(irq_num >= 0 && irq_num <= 7) {
		interrupt_mask = (0x01 << irq_num);
		master_mask = master_mask | interrupt_mask;
		outb(MASTER_8259_DATA, master_mask);
	}
	else if(irq_num >= 8 && irq_num <= 15) {
		interrupt_mask = (0x01 << (irq_num - 8));
		slave_mask = slave_mask | interrupt_mask;
		outb(SLAVE_8259_DATA, slave_mask);
	}
	restore_flags(flags);
}

/* NODENS HAVEN'T COMMENTED YET */
/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {
	cli_and_save(flags); 
	if(irq_num >= 0 && irq_num <= 7) {
		outb(MASTER_8259_PORT, (EOI | irq_num));
	}
	else if(irq_num >= 8 && irq_num <= 15) {
		outb(MASTER_8259_PORT, (EOI | 2));
		outb(SLAVE_8259_PORT, (EOI | (irq_num - 8)));
	}
	restore_flags(flags);
}
