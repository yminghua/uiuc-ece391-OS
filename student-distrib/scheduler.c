/* scheduler.c - system call for execute and halt
 * created by LYS, JJL on 2022/11/23
 */

#include "scheduler.h"
#include "Syscalls.h"

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




/* pit_init() - initialize pit
 * created by drush8
 */
void pit_init() {

}