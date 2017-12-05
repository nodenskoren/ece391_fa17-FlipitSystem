#ifndef _SYS_HANDLER_H
#define _SYS_HANDLER_H

// system call asm linkage
extern void sys_handler(void);
extern void rtc_wrapper(void);
extern void keyboard_wrapper(void);
extern void pit_wrapper(void);
#endif
