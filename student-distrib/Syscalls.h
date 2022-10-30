/* Syscalls.h - .h file for all system calls including execute and halt
 * created by LYS, JJL on 2022/10/29
 */

#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "types.h"
#include "PCB.h"

//syscalls number, may not be used
#define SYS_HALT    1
#define SYS_EXECUTE 2
#define SYS_READ    3
#define SYS_WRITE   4
#define SYS_OPEN    5
#define SYS_CLOSE   6
#define SYS_GETARGS 7
#define SYS_VIDMAP  8
#define SYS_SET_HANDLER  9
#define SYS_SIGRETURN  10

extern void SYSTEM_CALL_handler();

extern int32_t halt(uint32_t status);

extern int32_t execute (const uint8_t* command);

extern int32_t read (int32_t fd, void* buf, int32_t nbytes);

extern int32_t write (int32_t fd, const void* buf, int32_t nbytes);

extern int32_t open (const uint8_t* filename);

extern int32_t close (int32_t fd);

extern int32_t getargs(uint8_t* buf, int32_t nbytes);

extern int32_t vidmap(uint8_t** screen_start);

extern int32_t set_handler (int32_t signum, void* handler_address);

extern int32_t sigreturn (void);

#endif