/* PCB.h - PCB structure and PID table
 * created by LYS, JJL on 2022/10/29
 */

#ifndef PCB_H
#define PCB_H

#include "types.h"
#include "fileSystem.h"

#define MASK 0xFFFFE000     // 8KB, mask esp to find PCB
#define MAX_PNUM 3     // maximum process number, including the original boot up
#define MAX_FD 8        // maximum fd number in a fd_array   
#define KB 0x400
#define MB 0x100000
#define PCB_ARGLEN 128
#define GB 0x40000000

typedef struct {
    fdInfo_t fd_array[MAX_FD];
    uint32_t kesp;
    uint32_t kebp;
    uint32_t pid;
    uint32_t parent_pid;
    uint8_t argstr[PCB_ARGLEN];    //drush8: warning, another restriction is in execute_halt.c. Now they are the same:128 at 11.13
} PCB_t;


/*====extern variable====*/
extern PCB_t *pid_table[MAX_PNUM];

/*====extern functions====*/
extern PCB_t * get_PCB();
extern void  init_Syscall();
extern int init_PCB(int pid);//added by drush8.
PCB_t * get_PCB_withpid(int pid);
int32_t get_new_pid();
int giveup_pid(uint32_t pid);
// extern void init_PCBtable();

#endif //PCB_H
