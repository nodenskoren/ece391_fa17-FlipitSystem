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
        idt_desc_t the_idt_desc;
        the_idt_desc.seg_selector = KERNEL_CS;
        the_idt_desc.reserved4   = 0x0; 
        the_idt_desc.reserved3   = 0x0;  /* referenced from x86 ISA Manual */
        the_idt_desc.reserved2   = 0x1;
        the_idt_desc.reserved1   = 0x1;
        the_idt_desc.size        = 0x1;  /* 32 bit */
        the_idt_desc.reserved0   = 0x0;
        the_idt_desc.dpl         = 0x0;  /* kernel priviledged mode */
        the_idt_desc.present     = 0x1;

        /* Fills in the idt entry for first 19 exceptions reserved by Intel. */
        idt_desc_t* test = (idt_desc_t*) idt;

        for(i = 0; i < 19; i++){
                test[i] = the_idt_desc;
        }

        test[KEYBOARD_ENTRY] = the_idt_desc;
        test[RTC_ENTRY]      = the_idt_desc;

        SET_IDT_ENTRY(the_idt_desc, &exception_1);
        SET_IDT_ENTRY(the_idt_desc, &exception_2);
        SET_IDT_ENTRY(the_idt_desc, &exception_3);
        SET_IDT_ENTRY(the_idt_desc, &exception_4);
        SET_IDT_ENTRY(the_idt_desc, &exception_5);
        SET_IDT_ENTRY(the_idt_desc, &exception_6);
        SET_IDT_ENTRY(the_idt_desc, &exception_7);
        SET_IDT_ENTRY(the_idt_desc, &exception_8);
        SET_IDT_ENTRY(the_idt_desc, &exception_9);
        SET_IDT_ENTRY(the_idt_desc, &exception_10);
        SET_IDT_ENTRY(the_idt_desc, &exception_11);
        SET_IDT_ENTRY(the_idt_desc, &exception_12);
        SET_IDT_ENTRY(the_idt_desc, &exception_13);
        SET_IDT_ENTRY(the_idt_desc, &exception_14);
        SET_IDT_ENTRY(the_idt_desc, &exception_15);
        SET_IDT_ENTRY(the_idt_desc, &exception_16);
        SET_IDT_ENTRY(the_idt_desc, &exception_17);
        SET_IDT_ENTRY(the_idt_desc, &exception_18);
        //SET_IDT_ENTRY(the_idt_desc, &exception_19);
        //SET_IDT_ENTRY(the_idt_desc, &keyboard_interrupt_handler);

        






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







