#include "lib.h"
#include "i8259.h"

/* Opens the RTC file */
extern int32_t RTC_open(const uint8_t* filename);

/* Interrupt handler for RTC driver */
void RTC_interrupt_handler();

/* Blocks until the next RTC interrupt comes */
extern int32_t RTC_read(int32_t fd, void* buf, int32_t nbytes);

/* Change the RTC frequency */
extern int32_t RTC_write(int32_t fd, const uint32_t* buf, int32_t nbytes);

/* Closes the RTC file */
extern int32_t RTC_close(int32_t fd);




