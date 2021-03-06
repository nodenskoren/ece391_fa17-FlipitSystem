#ifndef KEYBOARD_HANDLER_H_
#define KEYBOARD_HANDLER_H_


/*
#include "multiboot.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "debug.h"
#include "tests.h"     
#include "initialize_idt.h"
*/


#define MAPPING_SIZE 60



/*interrupt handler code*/
extern void keyboard_interrupt_handler();

/*keyboard initialization code*/
extern void keyboard_initialization();



/*maps from press code to asii value*/
extern unsigned char keyboard_mapping_lowercase[MAPPING_SIZE];
extern unsigned char keyboard_mapping_capital[MAPPING_SIZE];

#endif
