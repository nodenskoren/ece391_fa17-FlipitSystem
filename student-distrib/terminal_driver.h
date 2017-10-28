#ifndef KYEBOARD_HANDLER_H_
#define KEYBOARD_HANDLER_H_

#include "lib.h"

extern int32_t terminal_open();

extern int32_t terminal_close();

extern int32_t terminal_read(char * buf, int32_t nbytes);

extern int32_t terminal_write(const char* buf,int32_t nbytes);

extern void add_to_buffer(char c);

extern void clear_buffer();

extern void copy_command_buffer();

#endif
