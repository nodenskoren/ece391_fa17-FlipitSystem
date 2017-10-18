/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/* Initialize the 8259 PIC */
void i8259_init(void) {
}

/* NODENS HAVEN'T COMMENTED YET */
/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num) {
	uint8_t interrupt_unmask;
	if(irq_num >= 0 && irq_num <= 7) {
		interrupt_mask = (0x00000001 << irq_num);
		outb(MASTER_8259_DATA, (master_mask | interrupt_unmask));
	}
	else if(irq_num >= 8 && irq_num <= 15) {
		interrupt_mask = (0x00000001 << (irq_num - 8));
		outb(SLAVE_8259_DATA, (slave_mask | interrupt_unmask));
	}
	return;
}

/* NODENS HAVEN'T COMMENTED YET */
/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num) {
	uint8_t interrupt_mask;
	if(irq_num >= 0 && irq_num <= 7) {
		interrupt_mask = ~(0x00000001 << irq_num);
		outb(MASTER_8259_DATA, (master_mask & interrupt_mask));
	}
	else if(irq_num >= 8 && irq_num <= 15) {
		interrupt_mask = ~(0x00000001 << (irq_num - 8));
		outb(SLAVE_8259_DATA, (slave_mask & interrupt_mask));
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
}
