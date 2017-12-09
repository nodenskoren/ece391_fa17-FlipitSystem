#include "lib.h"
#include "i8259.h"
#include "sys_handler.h"

extern void pit_init();

extern void pit_handler();

extern void pit_freq_change(uint32_t nFrequence);

extern void play_sound(uint32_t nFrequence);

extern void nosound();