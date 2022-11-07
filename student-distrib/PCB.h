/* PCB.h - PCB structure and PID table
 * created by LYS, JJL on 2022/10/29
 */

#ifndef PCB_H
#define PCB_H

#include "types.h"
#include "fileSystem.h"

#define MASK 0x2000     // 8KB, mask esp to find PCB
#define MAX_PNUM 10     // maximum process number, including the original boot up
#define MAX_FD 8        // maximum fd number in a fd_array   
#define KB 0x400
#define MB 0x100000

typedef struct {
    fdInfo_t fd_array[MAX_FD];
    uint32_t kesp;
    uint32_t kebp;
    uint32_t pid;
    uint32_t parent_pid;
} PCB_t;


/*====extern variable====*/
extern PCB_t *pid_table[MAX_PNUM];

/*====extern functions====*/
extern PCB_t * get_PCB();
extern void  init_Syscall();
extern int init_PCB(int pid);//added by drush8.
PCB_t * get_PCB_withpid(int pid);
// extern void init_PCBtable();

#endif //PCB_H
