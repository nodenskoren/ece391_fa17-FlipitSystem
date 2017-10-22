#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "initialize_idt.h"
#include "RTC_driver.h"

#define KEYBOARD_ENTRY 0x21
#define RTC_ENTRY      0x28
#define INTEL_EXCEPTION_NUM 20
#define IDT_MAX_ENTRY  256
#define SYSTEM_CALL_ENTRY 0x80


/*
 * initialize_RTC_driver
 *   DESCRIPTION: Fills in the IDT table for first 20 exceptions defined by Intel and
 *                set the other interrupt vector entries to not present.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: Fills in the IDT entry for exceptions and interrupt handlers.
 */

void initialize_idt(){

        int i;
        /* Fills in the idt entry for first 20 exceptions reserved by Intel. Referenced from x86 ISA Manual */
		for(i = 0; i < INTEL_EXCEPTION_NUM; i++){
			idt[i].seg_selector = KERNEL_CS;
			idt[i].reserved4   = 0x0; 
			idt[i].reserved3   = 0x0;  /* referenced from x86 ISA Manual */
			idt[i].reserved2   = 0x1;
			idt[i].reserved1   = 0x1;
			idt[i].size        = 0x1;  /* 32 bit */
			idt[i].reserved0   = 0x0;
			idt[i].dpl         = 0x0;  /* kernel priviledged mode */
			idt[i].present     = 0x1;
			
		}	
		
		/* Fills in the idt entry for other interrupt handlers. Set the present entry to not present.*/		
		for(i = INTEL_EXCEPTION_NUM; i < IDT_MAX_ENTRY; i++){
			idt[i].seg_selector = KERNEL_CS;
			idt[i].reserved4   = 0x0; 
			idt[i].reserved3   = 0x0;  /* referenced from x86 ISA Manual */
			idt[i].reserved2   = 0x1;
			idt[i].reserved1   = 0x1;
			idt[i].size        = 0x1;  /* 32 bit */
			idt[i].reserved0   = 0x0;
			idt[i].dpl         = 0x0;  /* kernel priviledged mode */
			idt[i].present     = 0x0;			
		}
		
		idt[SYSTEM_CALL_ENTRY].present = 0x1;
		
        SET_IDT_ENTRY(idt[0], exception_0); /* Sets the IDT entry for divide by zero error */ 
        SET_IDT_ENTRY(idt[1], exception_1); /* Sets the IDT entry for debug exception error */ 
        SET_IDT_ENTRY(idt[2], exception_2); /* Sets the IDT entry for non-maskable interrupt */ 
        SET_IDT_ENTRY(idt[3], exception_3); /* Sets the IDT entry for breakpoint */ 
        SET_IDT_ENTRY(idt[4], exception_4); /* Sets the IDT entry for overflow*/ 
        SET_IDT_ENTRY(idt[5], exception_5); /* Sets the IDT entry for bound exception */ 
        SET_IDT_ENTRY(idt[6], exception_6); /* Sets the IDT entry for invalid opcode */ 
        SET_IDT_ENTRY(idt[7], exception_7); /* Sets the IDT entry for FPU not available error */ 
        SET_IDT_ENTRY(idt[8], exception_8); /* Sets the IDT entry for double fault */ 
        SET_IDT_ENTRY(idt[9], exception_9); /* Sets the IDT entry for coprocessor segment overrun*/ 
        SET_IDT_ENTRY(idt[10], exception_10); /* Sets the IDT entry for invalid TSS*/ 
        SET_IDT_ENTRY(idt[11], exception_11); /* Sets the IDT entry for segment not present error */ 
        SET_IDT_ENTRY(idt[12], exception_12); /* Sets the IDT entry for stack exception*/ 
        SET_IDT_ENTRY(idt[13], exception_13); /* Sets the IDT entry for general protection */ 
        SET_IDT_ENTRY(idt[14], exception_14); /* Sets the IDT entry for page fault */ 
        SET_IDT_ENTRY(idt[15], exception_15); /* Sets the IDT entry for reserved */ 
        SET_IDT_ENTRY(idt[16], exception_16); /* Sets the IDT entry for floating point error */ 
        SET_IDT_ENTRY(idt[17], exception_17); /* Sets the IDT entry for alignment check */ 
        SET_IDT_ENTRY(idt[18], exception_18); /* Sets the IDT entry for machine check */ 
        SET_IDT_ENTRY(idt[19], exception_19); /* Sets the IDT entry for SIMD floating point exception*/ 	
		SET_IDT_ENTRY(idt[SYSTEM_CALL_ENTRY], syscall_handler);
		
		lidt(idt_desc_ptr); /* load the idt into idt descriptor pointer */
		
}




/*
 * exception_0
 *   DESCRIPTION: Prints out divide by zero error on screen 
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: Same as description
 */
void exception_0 (){
        printf("Divide by zero error!");
        while(1);
}

/*
 * exception_1
 *   DESCRIPTION: Prints out debug exception error
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: Same as description
 */
void exception_1 (){
        printf("Debug exception!");
        while(1);
}

/*
 * exception_2
 *   DESCRIPTION: Prints out non maskable interrupt on screen 
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: Same as description
 */
void exception_2 (){
        printf("Non-Maskable Interrupt!");
        while(1);
}

/*
 * exception_3
 *   DESCRIPTION: Prints out breakpoint on screen 
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: Same as description
 */
void exception_3 (){
        printf("Breakpoint!");
        while(1);
}

/*
 * exception_4
 *   DESCRIPTION: Prints out overflow on screen 
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: Same as description
 */
void exception_4 (){
        printf("Overflow!");
        while(1);
}

/*
 * exception_5
 *   DESCRIPTION: Prints out bound exception on screen 
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: Same as description
 */
void exception_5 (){
        printf("Bound exception!");
        while(1);
}

/*
 * exception_6
 *   DESCRIPTION: Prints out invalid opcode on screen 
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: Same as description
 */
void exception_6 (){
        printf("Invalid Opcode!");
        while(1);
}

/*
 * exception_7
 *   DESCRIPTION: Prints out FPU not available on screen 
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: Same as description
 */
void exception_7 (){
        printf("FPU not available!");
        while(1);
}

/*
 * exception_8
 *   DESCRIPTION: Prints out double fault on screen 
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: Same as description
 */
void exception_8 (){
        printf("Double Fault!");
        while(1);
}

/*
 * exception_9
 *   DESCRIPTION: Prints out segment overrun on screen 
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: Same as description
 */
void exception_9 (){
        printf("Coprocessor Segment Overrun!");
        while(1);
}

/*
 * exception_10
 *   DESCRIPTION: Prints out invalid TSS on screen 
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: Same as description
 */
void exception_10 (){
        printf("Invalid TSS!");
        while(1);
}

/*
 * exception_11
 *   DESCRIPTION: Prints out segment not present on screen 
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: Same as description
 */
void exception_11 (){
        printf("Segment not present!");
        while(1);
}

/*
 * exception_12
 *   DESCRIPTION: Prints out stack exception on screen 
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: Same as description
 */
void exception_12 (){
        printf("Stack exception!");
        while(1);
}

/*
 * exception_13
 *   DESCRIPTION: Prints out general protection on screen 
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: Same as description
 */
void exception_13 (){
        printf("General Protection!");
        while(1);
}

/*
 * exception_14
 *   DESCRIPTION:  Prints out page fault 
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: Same as description
 */
void exception_14 (){
        printf("Page Fault!");
        while(1);
}

/*
 * exception_15
 *   DESCRIPTION: Prints out reserved on screen
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: Same as description
 */
void exception_15 (){
        printf("Reserved!");
        while(1);
}

/*
 * exception_16
 *   DESCRIPTION: Prints out floating point error on screen 
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: Same as description
 */
void exception_16 (){
        printf("Floating-point error!");
        while(1);
}

/*
 * exception_17
 *   DESCRIPTION: Prints out alignment check on screen 
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: Same as description
 */
void exception_17 (){
        printf("Alignment Check!");
        while(1);
}

/*
 * exception_18
 *   DESCRIPTION: Prints out machine check on screen 
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: Same as description
 */
void exception_18 (){
        printf("Machine Check!");
        while(1);
}

/*
 * exception_19
 *   DESCRIPTION: Prints out SIMD floating point exception on screen 
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: Same as description
 */
void exception_19 (){
        printf("SIMD Floating-Point Exception!");
        while(1);
		
}

void syscall_handler(){
		printf("System Call Invoked!");
		while(1);
	
	
}







