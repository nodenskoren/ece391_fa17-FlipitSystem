#ifndef KYEBOARD_HANDLER_H_
#define KEYBOARD_HANDLER_H_

#include "lib.h"


/*system call opens file*/
extern int32_t terminal_open();
/*system call to close file*/
extern int32_t terminal_close();
/*system call reads last entered command*/
extern int32_t terminal_read(char * buf, int32_t nbytes);
/*system call writes to terminal*/
extern int32_t terminal_write(const char* buf,int32_t nbytes);

/*adds ascii character from keyboard to keyboard buffer*/
extern void add_to_buffer(char c);
/*clears keyboard buffer*/
extern void clear_buffer();
/*coppies keyboard buffer to command buffer*/
extern void copy_command_buffer();

#endif
