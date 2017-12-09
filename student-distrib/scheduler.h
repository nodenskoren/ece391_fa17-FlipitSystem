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
#include "i8259.h"

#define INITIAL_VALUE -1
#define MINIMUM_TERMINAL_INDEX 0
#define MAXIMIM_TERMINAL_INDEX 2
#define NUM_OF_TERMINAL 3
#define PIT_IRQ_NUM 0

//TEMPORARY SET TO 0
extern int32_t terminal_num;
extern int32_t current_visible;
//TEMPORARY SET TO 0
extern void scheduler();

/* Scheduler structure */
typedef struct terminal_t{
	pcb_t* current_pcb;
	pcb_t* active_process;
	pcb_t* previous_process;
	uint32_t esp;
	uint32_t ebp;
	uint8_t keyboard_buffer[128];
	int previous_buf_length;
	int buf_position;
	int screen_x;
	int screen_y;
	volatile int command_ready_flag;
	volatile int RTC_flag;
} terminal_t;

/* Declaration of 3 terminal structures */
terminal_t terminal[3];

#endif
