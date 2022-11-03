/* Syscalls.h - .h file for all system calls including execute and halt
 * created by LYS, JJL on 2022/10/29
 */

#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "types.h"
#include "PCB.h"
#include "rtc.h"
#include "e391terminal.h"
#include "e391keyboard.h"


/* System Call Wrapper */
extern int32_t sys_halt (uint8_t status);
extern int32_t sys_execute (const uint8_t* command);
extern int32_t sys_read (int32_t fd, void* buf, int32_t nbytes);
extern int32_t sys_write (int32_t fd, const void* buf, int32_t nbytes);
extern int32_t sys_open (const uint8_t* filename);
extern int32_t sys_close (int32_t fd);
extern int32_t sys_getargs (uint8_t* buf, int32_t nbytes);
extern int32_t sys_vidmap (uint8_t** screen_start);
extern int32_t sys_set_handler (int32_t signum, void* handler);
extern int32_t sys_sigreturn (void);


extern PCB_t PCB_current;

extern void SYSTEM_CALL_handler();

/* -------------------- System Call Functions ---------------------- */

extern int32_t halt (uint32_t status);

extern int32_t execute (const uint8_t* command);

extern int32_t read (int32_t fd, void* buf, int32_t nbytes);

extern int32_t write (int32_t fd, const void* buf, int32_t nbytes);

extern int32_t open (const uint8_t* filename);

extern int32_t close (int32_t fd);

extern int32_t getargs (uint8_t* buf, int32_t nbytes);

extern int32_t vidmap (uint8_t** screen_start);

extern int32_t set_handler (int32_t signum, void* handler_address);

extern int32_t sigreturn (void);


/* ----------------------- test functions ------------------------ */

extern void Syscalls_test_file ();

extern void Syscalls_test_dir ();

void Syscalls_test_terminal ();

#endif /* SYSCALLS_H */
