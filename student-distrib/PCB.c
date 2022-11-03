
/* PCB.c - PCB methods
 * created by LYS, JJL on 2022/10/29
 */

#include "PCB.h"

PCB_t *pid_table[MAX_PNUM];


PCB_t * get_PCB() {
    register uint32_t save_ebp asm("ebp"); 
    return (PCB_t*)((save_ebp-1) & MASK);
}
