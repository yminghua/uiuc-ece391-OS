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

#define UVIDEOADDR 256*MB

/* System Call Wrapper */
//only used for testing
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


//extern PCB_t *PCB_current;//we donot use it already.

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

// extern int32_t set_handler (int32_t signum, void* handler_address);

// extern int32_t sigreturn (void);

/* ----------------------- assitance functions ------------------------ */

extern int32_t openStdInOut(int pid);

//below is the asm code in Execute_Halt.S
extern int32_t asm_exec_end(uint32_t exe_ptr, uint32_t ustack_ptr, uint32_t kespaddr);
extern int32_t asm_halt_end(uint32_t kesp, uint32_t kebp, uint32_t retvalue);


//below is used in the execute_halt.c
extern int file_check(int32_t fd);
extern int32_t get_new_pid();
extern int giveup_pid(uint32_t pid);
extern void clear_file_position(int32_t fd);
extern void loader(int32_t fd, uint32_t pid);
extern int tiny_parse(int * args_pointer, const int8_t* command);
extern int get_arg_len(int i, int j, const uint8_t* command); //drush8: warning: i<j is the must requiring
extern int32_t getUStack(int32_t pid);
extern int32_t getKStack(int32_t pid);
extern void paging_switch(int new_pid, int old_pid);
extern int fillPCBargs(int *argsp, const uint8_t *command, int argnum, PCB_t *tarpcb,int mode);

/* ----------------------- test functions ------------------------ */

extern void Syscalls_test_file ();

extern void Syscalls_test_dir ();

void Syscalls_test_terminal ();

#endif /* SYSCALLS_H */
