#include "lib.h"
#include "i8259.h"

/* Initialization function for RTC driver */
extern void initialize_RTC_driver();

/* Interrupt handler for RTC driver */
void RTC_interrupt_handler();
