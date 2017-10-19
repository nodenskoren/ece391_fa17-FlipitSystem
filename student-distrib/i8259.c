/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"
#include <linux/spinlock.h>

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */
spinlock_t irq_lock;


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
	unsigned long flags;
	spin_lock_irqsave(irq_lock, flags);
	
	/* masks all interrupts (on both PICs) */
	/* active low */
	master_mask = 0xFF;
	slave_mask = 0xFF;
	outb(MASTER_8259_DATA, master_mask);
	outb(SLAVE_8259_DATA, slave_mask);
	
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
	master_mask = 0x00;
	slave_mask = 0x00;
	outb(MASTER_8259_DATA, master_mask);
	outb(SLAVE_8259_DATA, slave_mask);
	
	/* releases the lock, and restores the IF flag */
	spin_unlock_irqrestore(irq_lock, flags);
	return;
}

/* NODENS HAVEN'T COMMENTED YET */
/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num) {
	uint8_t interrupt_unmask;
	if(irq_num >= 0 && irq_num <= 7) {
		interrupt_mask = ~(0x01 << irq_num);
		outb(MASTER_8259_DATA, (master_mask & interrupt_unmask));
	}
	else if(irq_num >= 8 && irq_num <= 15) {
		interrupt_mask = ~(0x01 << (irq_num - 8));
		outb(SLAVE_8259_DATA, (slave_mask & interrupt_unmask));
	}
	return;
}

/* NODENS HAVEN'T COMMENTED YET */
/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num) {
	uint8_t interrupt_mask;
	if(irq_num >= 0 && irq_num <= 7) {
		interrupt_mask = (0x01 << irq_num);
		outb(MASTER_8259_DATA, (master_mask | interrupt_mask));
	}
	else if(irq_num >= 8 && irq_num <= 15) {
		interrupt_mask = (0x01 << (irq_num - 8));
		outb(SLAVE_8259_DATA, (slave_mask | interrupt_mask));
	}
	return;	
}

/* NODENS HAVEN'T COMMENTED YET */
/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {
	if(irq_num >= 0 && irq_num <= 7) {
		outb(MASTER_8259_PORT, (EOI | irq_num));
	}
	else if(irq_num >= 8 && irq_num <= 15) {
		outb(MASTER_8259_PORT, (EOI | 2));
		outb(SLAVE_8259_PORT, (EOI | (irq_num - 8));
	}
	return;
}
