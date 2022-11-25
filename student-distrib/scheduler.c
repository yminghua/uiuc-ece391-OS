/* scheduler.c - system call for execute and halt
 * created by LYS, JJL on 2022/11/23
 */

#include "scheduler.h"
#include "Syscalls.h"
#include "lib.h"
#include "types.h"
#include "intrexcenum.h"

/* scheduler() - round-robin for 3 process
 * created by LYS
 */
void scheduler() {

}


/* init_multiple_terminal() - init 3 shell program
 * created by LYS
 */
void init_multiple_terminal() {
    const int8_t *fnamep = "./shell";  
    uint32_t prog_code_start;
    int32_t fd;
    uint32_t childpid;
    int i;

    //open './shell' 3 times and load the user program image
    for (childpid=1; childpid<=3; childpid++) {
        fd = open((uint8_t*)fnamep);
        prog_code_start = file_check(fd);

        //fill in child PCB
        init_PCB(childpid);
        pid_table[childpid]->parent_pid = 0;
        openStdInOut(childpid);
        loader(fd,childpid); 
        close(fd);

        //init sche_list pcb_ptr field
        sche_list[childpid-1].pcb_ptr = get_PCB_withpid(childpid);
    }

    //init the prev and next field of sche_list
    for ()




}




/*
 * 
 * 
 * 
 * drush8: below is the code of the PIT
 * for the OS, only init func will be called in the critical section at beginning.
 * 
 * 
 * 
*/


void set_pit_count(uint16_t count) {
	// Disable interrupts
	cli();
 
	// Set low byte
	outb(count&0xFF,CCZERO);		// Low byte
	outb((count&0xFF00)>>8,CCZERO);	// High byte
	return;
}


/* pit_init() - initialize pit
 * created by drush8
 */
// pit base frequency is 1.193182 MHz
// what we need is 20--100hz
// so we set the devider as: 16384, so the final hz is 72.82
// 2^14, so we need to set as 0100 0000 0000 0000 
void pit_init() {
    outb(0x34, CMDPORT);               //0x34 == 0b00110100 ::  channel 0, lobyte/hibyte, rate generator
    set_pit_count(0x4000);
    enable_irq(PIT_IRQ);  //unmask the irq of the PIT(0).
    return;
}