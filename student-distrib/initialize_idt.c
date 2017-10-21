#include "multiboot.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "debug.h"
#include "tests.h"     
#include "initialize_idt.h"

#define KEYBOARD_ENTRY 0x21
#define RTC_ENTRY      0x28

/*  This function is called during the booting. Sets the exception and interrupt handlers. */
void initialize_idt(){

        int i;
        /* Fills in the idt entry for first 19 exceptions reserved by Intel. */
		for(i = 0; i < 19; i++){
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

        SET_IDT_ENTRY(idt[0], &exception_0);
        SET_IDT_ENTRY(idt[1], &exception_1);
        SET_IDT_ENTRY(idt[2], &exception_2);
        SET_IDT_ENTRY(idt[3], &exception_3);
        SET_IDT_ENTRY(idt[4], &exception_4);
        SET_IDT_ENTRY(idt[5], &exception_5);
        SET_IDT_ENTRY(idt[6], &exception_6);
        SET_IDT_ENTRY(idt[7], &exception_7);
        SET_IDT_ENTRY(idt[8], &exception_8);
        SET_IDT_ENTRY(idt[9], &exception_9);
        SET_IDT_ENTRY(idt[10], &exception_10);
        SET_IDT_ENTRY(idt[11], &exception_11);
        SET_IDT_ENTRY(idt[12], &exception_12);
        SET_IDT_ENTRY(idt[13], &exception_13);
        SET_IDT_ENTRY(idt[14], &exception_14);
        SET_IDT_ENTRY(idt[15], &exception_15);
        SET_IDT_ENTRY(idt[16], &exception_16);
        SET_IDT_ENTRY(idt[17], &exception_17);
        SET_IDT_ENTRY(idt[18], &exception_18);		
		
		lidt(idt_desc_ptr);


}


/* This exception handlers are called when the PIC raises exceptions. It simply prints out the exception and make the program stuck */
void exception_0 (){
        printf("Divide by zero error!");
        while(1);
}

void exception_1 (){
        printf("Debug exception!");
        while(1);
}
void exception_2 (){
        printf("Non-Maskable Interrupt!");
        while(1);
}
void exception_3 (){
        printf("Breakpoint!");
        while(1);
}
void exception_4 (){
        printf("Overflow!");
        while(1);
}
void exception_5 (){
        printf("Bound exception!");
        while(1);
}

void exception_6 (){
        printf("Invalid Opcode!");
        while(1);
}

void exception_7 (){
        printf("FPU not available!");
        while(1);
}
void exception_8 (){
        printf("Double Fault!");
        while(1);
}
void exception_9 (){
        printf("Coprocessor Segment Overrun!");
        while(1);
}
void exception_10 (){
        printf("Invalid TSS!");
        while(1);
}
void exception_11 (){
        printf("Segment not present!");
        while(1);
}
void exception_12 (){
        printf("Stack exception!");
        while(1);
}
void exception_13 (){
        printf("General Protection!");
        while(1);
}

void exception_14 (){
        printf("Page Fault!");
        while(1);
}
void exception_15 (){
        printf("Reserved!");
        while(1);
}
void exception_16 (){
        printf("Floating-point error!");
        while(1);
}
void exception_17 (){
        printf("Alignment Check!");
        while(1);
}
void exception_18 (){
        printf("Machine Check!");
        while(1);
}







