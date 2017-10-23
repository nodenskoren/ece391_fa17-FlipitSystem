#include "lib.h"
#include "i8259.h"   
#include "RTC_driver.h"

/* Initialization function for IDT */
extern void initialize_idt();

/* Exception handlers for first 20 exceptions defined by Intel */
void exception_0 ();
void exception_1 ();
void exception_2 ();
void exception_3 ();
void exception_4 ();
void exception_5 ();
void exception_6 ();
void exception_7 ();
void exception_8 ();
void exception_9 ();
void exception_10 ();
void exception_11 ();
void exception_12 ();
void exception_13 ();
void exception_14 ();
void exception_15 ();
void exception_16 ();
void exception_17 ();
void exception_18 ();
void exception_19 ();

/* Handler for system call */
void syscall_handler();

