
/* PCB.c - PCB methods
 * created by LYS, JJL on 2022/10/29
 */

#include "PCB.h"

PCB_t *pid_table[MAX_PNUM];


PCB_t * get_PCB() {
    register uint32_t save_ebp asm("ebp"); 
    return (PCB_t*)((save_ebp-1) & MASK);
}

extern void  init_Syscall() {
    int i;
    for (i=0; i<MAX_PNUM; i++) {
        pid_table[i]=0;
    }
    PCB_t *PCB_prime = (PCB_t *) (8*MB - 8*KB);
    PCB_prime->pid = 0;
    pid_table[0] = PCB_prime;
}
