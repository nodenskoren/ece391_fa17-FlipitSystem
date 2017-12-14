#include "pit.h"
#include "x86_desc.h"

#define PIT_CMB_VAL  0x36
#define PIT_CH0_PORT 0x40
#define PIT_CMD_PORT 0x43
#define PIT_SD_PORT  0x42
#define PIT_CST      0xb6
#define Frequency        100
#define MAX_PIT_FREQ 1193180
#define EIGHT_BIT_MASK   0xFF
#define EIGHT_BIT_SHIFT  8
#define PIT_IRQ_NUM  0
#define PIT_ENTRY    0x20
#define IOPORT 0x61
#define IOMASK 0xFC
#define PIT_AD      3
#define EB_SHIFT    8

/* PIT functionality is inspired from OSDev! */


/*
 * pit_init
 *   DESCRIPTION: Sets the initial frequency to 20Hz and enable pit interrupt
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */

void pit_init(){
	
	idt[PIT_ENTRY].seg_selector = KERNEL_CS;
	idt[PIT_ENTRY].reserved4   = 0x0; 
	idt[PIT_ENTRY].reserved3   = 0x0;  /* referenced from x86 ISA Manual */
	idt[PIT_ENTRY].reserved2   = 0x1;
	idt[PIT_ENTRY].reserved1   = 0x1;
	idt[PIT_ENTRY].size        = 0x1;  /* 32 bit */
	idt[PIT_ENTRY].reserved0   = 0x0;
	idt[PIT_ENTRY].dpl         = 0x0;  /* kernel priviledged mode */
	idt[PIT_ENTRY].present     = 0x1;
	SET_IDT_ENTRY(idt[PIT_ENTRY], pit_wrapper);
	
	// sends the command byte
	outb(PIT_CMB_VAL, PIT_CMD_PORT);
	
	// calculate the dividor needed to set to the correct frequency
	uint32_t dividor = (MAX_PIT_FREQ / Frequency);
	
	// set the frequency of the pit
	outb((dividor & EIGHT_BIT_MASK), PIT_CH0_PORT);
	outb(((dividor >> EIGHT_BIT_SHIFT ) & EIGHT_BIT_MASK), PIT_CH0_PORT);
	
	// enable pit in pic
	enable_irq(PIT_IRQ_NUM);
	
}

/*
 * pit_handler
 *   DESCRIPTION: Interrupt handler for PIT
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */

void pit_handler(){
	/* uint32_t flags;	
	//cli_and_save(flags);  // make sure the loading is protected
	
	// call scheduler function 
	
	printf("hello");
	//restore_flags(flags); 
	send_eoi(PIT_IRQ_NUM); */
	
	
}

/*
 * pit_freq_change
 *   DESCRIPTION: Change the frequency of pit
 *   INPUTS: uint32_t nFrequence
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */

void pit_freq_change(uint32_t nFrequence){
	
 	uint32_t Div = MAX_PIT_FREQ / nFrequence;
 	outb((uint8_t) (Div), PIT_SD_PORT);
 	outb((uint8_t) (Div >> EB_SHIFT), PIT_SD_PORT);	
	
}

/*
 * play_sound
 *   DESCRIPTION: Play beep sound 
 *   INPUTS: uint32_t nFrequence
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */

 void play_sound(uint32_t nFrequence) {
 	uint32_t Div;
 	uint8_t tmp;
 
        //Set the PIT to the desired frequency
 	Div = MAX_PIT_FREQ / nFrequence;
 	outb(PIT_CST, PIT_CMD_PORT);
 	outb((uint8_t) (Div), PIT_SD_PORT);
 	outb((uint8_t) (Div >> EB_SHIFT), PIT_SD_PORT);
 
        //And play the sound using the PC speaker
 	tmp = inb(IOPORT);
  	if (tmp != (tmp | PIT_AD)) {
 		outb( tmp | PIT_AD, IOPORT);
 	}
 }
 
/*
 * nosound
 *   DESCRIPTION: Make the sound shut up
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
 void nosound() {
 	uint8_t tmp = inb(IOPORT) & IOMASK;
 
 	outb(tmp, IOPORT);
 }
 





