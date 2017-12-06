#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include "systemcall.h"
#include "types.h"
#include "filesystem.h"
#include "paging.h"
#include "terminal_driver.h"
#include "RTC_driver.h"
#include "keyboard_handler.h"
#include "x86_desc.h"
#include "lib.h"
//TEMPORARY SET TO 0
static int32_t terminal_num = 0;
//TEMPORARY SET TO 0
extern void scheduler();

typedef struct terminal_t{
	pcb_t* current_pcb;
	pcb_t* active_process;
	pcb_t* previous_process;
	uint32_t esp;
	uint8_t keyboard_buffer[128];
} terminal_t;

terminal_t terminal[3];

#endif
