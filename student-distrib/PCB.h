/* PCB.h - PCB structure and PID table
 * created by LYS, JJL on 2022/10/29
 */

#ifndef PCB_H
#define PCB_H

#include "types.h"
#include "fileSystem.h"

#define MASK 0x2000 //8MB, mask esp to find PCB
#define MAX_PNUM 10  //maximum process number, including the original boot up

typedef struct {
    fdInfo_t fd_array[8];
    uint32_t kesp;
    uint32_t kebp;
    uint32_t pid;
    uint32_t parent_pid;
} PCB_t;


/*====extern variable====*/
extern PCB_t *pid_table[MAX_PNUM];

/*====extern functions====*/
extern PCB_t * get_PCB();


#endif //PCB_H