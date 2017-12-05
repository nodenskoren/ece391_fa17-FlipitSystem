#include "systemcall.h"
#include "types.h"
#include "filesystem.h"
#include "paging.h"
#include "terminal_driver.h"
#include "RTC_driver.h"
#include "keyboard_handler.h"
#include "x86_desc.h"
#include "lib.h"

extern void scheduler();

typedef struct terminal_t{
	pcb_t* current_pcb;
	uint32_t esp;
	uint8_t keyboard_buffer[128];
} terminal_t;

terminal_t terminal[3];
