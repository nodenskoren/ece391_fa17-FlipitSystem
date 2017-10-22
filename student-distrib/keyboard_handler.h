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

extern void keyboard_interrupt_handler();
extern void keyboard_initialization();

extern unsigned char keyboard_mapping_lowercase[60];
extern unsigned char keyboard_mapping_capital[60];

#endif
