/* scheduler.c - system call for execute and halt
 * created by LYS, JJL on 2022/11/23
 */

#include "scheduler.h"
#include "Syscalls.h"
#include "x86_page.h"
#include "x86_desc.h"

#define REAL_VID_PAGE 0xB8000

sche_node_t sche_list[3];
sche_node_t *cur_sche_node;
uint32_t cur_visible_terminal;

/* scheduler() - round-robin for 3 process
 * created by LYS
 */
void scheduler() {
    uint32_t cur_pid = cur_sche_node->pcb_ptr->pid;
    uint32_t next_pid = cur_sche_node->next->pcb_ptr->pid;
    uint32_t terminal_pid; //the terminal pid for next process
    // cur_sche_node = cur_sche_node->next;

    // get terminal pid for the next process
    terminal_pid = next_pid;
    if (next_pid <= 3) terminal_pid = next_pid;
    else while ((terminal_pid=get_PCB_withpid(terminal_pid)->parent_pid) > 3);

    // do paging switch (map next user program image to vm 128MB)
    paging_switch(next_pid, cur_pid);

    // remap kernel video memory page (the vm of REAL_VID_PAGE)
    unmap_4K(REAL_VID_PAGE);
    if (get_PCB_withpid(terminal_pid)->visible) map_4K(REAL_VID_PAGE, REAL_VID_PAGE);
    else map_4K(REAL_VID_PAGE, BackupVP(terminal_pid));

    // set TSS.esp0 for next process
    tss.esp0 = (uint32_t)getKStack(next_pid);

    // save old ebp, return &p->kesp; do saving old esp and stack switch back in asm
    PCB_t * p = get_PCB();
    PCB_t * nextp = get_PCB_withpid(next_pid);
    register uint32_t the_ebp asm("ebp");
    p->kebp = the_ebp;
    scheduler_asm(nextp->kesp, nextp->kebp, &p->kesp);

    return;
}


/* init_multiple_terminal() - init 3 shell program
 * created by LYS
 */
void init_multiple_terminal() {
    const int8_t *fnamep = "shell";  
    uint32_t prog_code_start;
    int32_t fd;
    uint32_t childpid;
    int i, i_prev, i_next;

    //open './shell' 3 times and load the user program image
    for (childpid=3; childpid>=1; childpid--) {
        fd = open((uint8_t*)fnamep);
        prog_code_start = file_check(fd);

        //fill in child PCB
        pid_table[childpid] = get_PCB_withpid(childpid);
        init_PCB(childpid);
        pid_table[childpid]->parent_pid = 0;
        openStdInOut(childpid);
        loader(fd,childpid); 
        close(fd);

        //init sche_list pcb_ptr field
        sche_list[childpid-1].pcb_ptr = get_PCB_withpid(childpid);

        //init the Kstack for IRET and scheduler() context (for shell 2 and 3 only?)
        if (childpid==1) break;
        asm_init_terminal_stack(getKStack(childpid), getUStack(childpid), prog_code_start);

    }

    //init the prev and next field of sche_list
    for (i=0; i<3; i++) {
        if (i==0) i_prev=2;
        else i_prev=i-1;
        if (i==2) i_next=0;
        else i_next=i+1;

        sche_list[i].prev = &sche_list[i_prev];
        sche_list[i].next = &sche_list[i_next];
    }

    //set up paging for backup video page
    map_4K(BackupVP(1), BackupVP(1));
    map_4K(BackupVP(2), BackupVP(2));
    map_4K(BackupVP(3), BackupVP(3));

    //set up cur_visible_terminal to 1, cur_sche_node to first and execute first shell
    PCB_t * p = get_PCB();
    cur_visible_terminal=1;
    sche_list[0].pcb_ptr->visible=1;
    cur_sche_node = &sche_list[0];
    childpid=1;
    tss.esp0 = (uint32_t)getKStack(childpid);
    register uint32_t the_ebp asm("ebp");
    p->kebp = the_ebp;
    asm_exec_end(prog_code_start, getUStack(childpid),(int32_t)(&p->kesp));
}




/* pit_init() - initialize pit
 * created by drush8
 */
void pit_init() {

}
