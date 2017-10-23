/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */
unsigned long flags;

/* 
 * i8259_init
 *		DESCRIPTION: After masking interrupts on the processor and acquiring a lock, 
 *					the code masks all interrupts (on both PICs),
 *					executes the initialization sequence,
 *					restores the mask settings,
 *					releases the lock, and restores the IF flag.
 *		INPUTS: None
 *		OUTPUTS: None
 *		SIDE EFFECT: All interrupts would be temporarily disabled during the initialization process, all PIC interrupts would be
 *					disabled after initialization until being enabled. Master's IRQ2 would be enabled anytime for the purpose of
 *					simplicity, regardless of the interrupt enabl/disabling status on slave PIC.
 *
 */
void i8259_init(void) {

	/*
	 * Executes the initialization sequence:
	 * The first word, ICW1, is delivered to the first PIC port—either 0x20 or 0xA0—and tells the PIC that it is being initialized
	 * The remaining ICWs are written to the second port.
	 *
	 * This is a critical section because once ICW1 is sent out,
	 * the initialization cannot be interrupted by other data sent to the 2nd port. 
	 *
	 */	
	
	/* Masking all interrupts and save the flags */
	cli_and_save(flags); 	
	
	/* For the master PIC */
	outb(ICW1, MASTER_8259_PORT);
	outb(ICW2_MASTER, MASTER_8259_DATA);
	outb(ICW3_MASTER, MASTER_8259_DATA);
	outb(ICW4, MASTER_8259_DATA);
	
	/* For the slave PIC */	
	outb(ICW1, SLAVE_8259_PORT);
	outb(ICW2_SLAVE, SLAVE_8259_DATA);
	outb(ICW3_SLAVE, SLAVE_8259_DATA);
	outb(ICW4, SLAVE_8259_DATA);
	
	
	/* outb(MASTER_8259_DATA, master_mask);
	outb(SLAVE_8259_DATA, slave_mask); */
	
	/* restores the mask settings */
	outb(DISABLE_ALL_INTERRUPTS, MASTER_8259_DATA);
	outb(DISABLE_ALL_INTERRUPTS, SLAVE_8259_DATA);
	restore_flags(flags);
	
	enable_irq(2); // irq 2 is slave pic
}

/* 
 * enable_irq
 *		DESCRIPTION: Enable (unmask) the specified IRQ
 *		INPUTS: Index number of the IRQ line desired to enable.
 *		OUTPUTS: None
 *		SIDE EFFECT: master_mask's and slave_mask's will be changed to reflect the current state of enabl/disabled interrupts.
 *
 */
void enable_irq(uint32_t irq_num) {
	
	/* 
	 * Mask all interrupts temporarily to prevent undesired mask values
	 * being written out to the PIC's data port due to other interrupts changing the value in between read/write.
	 *
	 * To enable a interrupt, opposite logic is used, and we need to change the IRQ's bit on PIC mask from 1 to 0.
	 * To do so, we AND the specific bit with a 0 and left other bits unchanged.
	 *
	 */
	cli_and_save(flags); 
	uint8_t interrupt_unmask;
	/* If the IRQ line we wish to enable is from the master PIC */
	if(irq_num >= MASTER_IRQ_0 && irq_num <= MASTER_IRQ_7) {
		/* Only the enabling bit would be 0 */
		interrupt_unmask = ~(0x01 << irq_num);
		/* Write back the enabled bit string */
		master_mask = inb(MASTER_8259_DATA) & interrupt_unmask;
		outb(master_mask, MASTER_8259_DATA);
	}
	/* If the IRQ line we wish to enable is from the slave PIC */	
	else if(irq_num >= SLAVE_IRQ_0 && irq_num <= SLAVE_IRQ_7) {
		/* Only the enabling bit would be 0, but irq_num should be referenced using slave PIC instead of master PIC */
		interrupt_unmask = ~(0x01 << (irq_num - SLAVE_IRQ_LOCAL_NUMBER));
		/* Write back the enabled bit string */
		slave_mask = inb(SLAVE_8259_DATA) & interrupt_unmask;
		outb(slave_mask, SLAVE_8259_DATA);
	}
	/* restores the mask settings */	
	restore_flags(flags);	
}

/* 
 * disable_irq
 *		DESCRIPTION: Disable (mask) the specified IRQ
 *		INPUTS: Index number of the IRQ line desired to disable.
 *		OUTPUTS: None
 *		SIDE EFFECT: master_mask's and slave_mask's will be changed to reflect the current state of enabl/disabled interrupts.
 *
 */
void disable_irq(uint32_t irq_num) {
	
	/* 
	 * Mask all interrupts temporarily to prevent undesired mask values
	 * being written out to the PIC's data port due to other interrupts changing the value in between read/write.
	 *
	 * To disable a interrupt, opposite logic is used, and we need to change the IRQ's bit on PIC mask from 0 to 1.
	 * To do so, we OR the specific bit with a 1 and left other bits unchanged.
	 *
	 */	
	cli_and_save(flags); 
	uint8_t interrupt_mask;
	/* If the IRQ line we wish to disable is from the master PIC */	
	if(irq_num >= MASTER_IRQ_0 && irq_num <= MASTER_IRQ_7) {
		/* Only the disabling bit would be 1 */		
		interrupt_mask = (0x01 << irq_num);
		/* Write back the disabled bit string */		
		master_mask = inb(MASTER_8259_DATA) | interrupt_mask;
		outb(master_mask, MASTER_8259_DATA);
	}
	/* If the IRQ line we wish to disable is from the slave PIC */		
	else if(irq_num >= SLAVE_IRQ_0 && irq_num <= SLAVE_IRQ_7) {
		/* Only the disabling bit would be 1, but irq_num should be referenced using slave PIC instead of master PIC */		
		interrupt_mask = (0x01 << (irq_num - SLAVE_IRQ_LOCAL_NUMBER));
		/* Write back the disabled bit string */
		slave_mask = inb(SLAVE_8259_DATA) | interrupt_mask;
		outb(slave_mask, SLAVE_8259_DATA);
	}
	/* restores the mask settings */
	restore_flags(flags);
}

/* 
 * send_eoi
 *		DESCRIPTION: Send end-of-interrupt signal for the specified IRQ
 *		INPUTS: Index number of the IRQ line acknowledged.
 *		OUTPUTS: None
 *		SIDE EFFECT: None.
 *
 */
void send_eoi(uint32_t irq_num) {
	/* 
	 * If the IRQ line acknowledged is from the master PIC: 
	 *		only need to write to the master port.
	 *
	 */
	if(irq_num >= MASTER_IRQ_0 && irq_num <= MASTER_IRQ_7) {
		outb((EOI | irq_num), MASTER_8259_PORT);
	}
	/* 
	 * If the IRQ line acknowledged is from the slave PIC: 
	 *		need to write to both the slave and the master port, because no matter where the interrupt is from,
	 *		signals are propageted from/to the master PIC.
	 *
	 */	
	else if(irq_num >= SLAVE_IRQ_0 && irq_num <= SLAVE_IRQ_7) {
		/* Slave PIC is always connected to IRQ2 in our scenario */
		outb((EOI | MASTER_IRQ_2), MASTER_8259_PORT);
		/* irq_num should be referenced using slave PIC instead of master PIC */
		outb((EOI | (irq_num - SLAVE_IRQ_LOCAL_NUMBER)), SLAVE_8259_PORT);
	}
}

