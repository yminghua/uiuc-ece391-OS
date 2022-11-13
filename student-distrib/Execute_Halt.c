/* Execute_Halt.c - system call for execute and halt
 * created by LYS, JJL on 2022/10/29
 */

#include "Syscalls.h"
#include "fileSystem.h"
#include "x86_page.h"
#include "x86_desc.h"

#define EXE_HEADER_BYTES 40
#define EXE_MAG0 0x7f
#define EXE_MAG1 0x45
#define EXE_MAG2 0x4c
#define EXE_MAG3 0x46
#define PROG_LOAD_VM 0x08048000

#define MaxArgs 6 //drush8's flag: we os now accept 10 args for the command at most.
#define MaxCommand 128


int32_t execute (const uint8_t* command) {
    uint8_t* fnamep; //get this file name after parsing the command
    uint8_t fname[FNAME_LEN];       //drush8: we use the array instead.

    uint32_t prog_code_start;
    int32_t fd;

    //drush8: S T E P 1 : get one pid.
    uint32_t childpid = get_new_pid();

    int32_t pair_args_pointer[MaxArgs*2];
    char argsnum;
    int fnamelen = 0;

    if (childpid==-1) {
        printf("no more task slot, quit\n");
        return -1;
    }    

    //drush8: S T E P 2 : parse args
    argsnum = tiny_parse(pair_args_pointer,(int8_t *)command);
    if(argsnum == 0) {
        giveup_pid(childpid);
        return 0; 
    //nothing, success default.
    }

    //next, we try to get the file names after parsing.
    fnamelen = pair_args_pointer[1]-pair_args_pointer[0];   //first argument, 
    if(fnamelen < 32){
        strncpy((int8_t*)fname, (int8_t*)command, fnamelen);
        fname[fnamelen] = '\0';
        }
    else strncpy((int8_t*)fname, (int8_t*)command, 32);

    //below is: drush8's flag: small parse on dir.. will be fixed
    //for tree fs structure.
    fnamep = fname;
    if(fname[0] == '.' && fname[1] == '/') fnamep+=2;

    //LYS: S T E P 3 :open and check file and copy prog image section
    fd = open((uint8_t*)fnamep);
    if (fd==-1) {
        printf("file name %s does not exist, execute fail", fname);
        giveup_pid(childpid);
        return -1;
    }
    prog_code_start = file_check(fd);
    if (prog_code_start==-1) {
        printf("file check fail, %s is not an executable", fname);
        giveup_pid(childpid);
        return -1;
    }
    //drush8: S T E P 4: loader with paging
    loader(fd,childpid); //load program image into contiguous physical address
    //ok,over loading, we can close the file.

    close(fd);

    //drush8: S T E P 5: fill the PCB and open stdin/out for the child.
    PCB_t * p = get_PCB();
    init_PCB(childpid);
    pid_table[childpid]->parent_pid = p->pid;
    openStdInOut(childpid);

    //drush8: S T E P 6.1: seting the TSS segment
    tss.esp0 = (uint32_t)getKStack(childpid);
    //tss.ss0 = (uint16_t)KERNEL_DS; //unnecessary

    //drush8: S T/ E P 6.2: final step, use the asm linakage to do detail works.

    //before the calling, we need to get the args and set the kebp&kesp.
    //consider in the following calling we'll push 2 32int args, we'll do additional work
    register uint32_t the_ebp asm("ebp");
    //we won't use ebp in our asm, so save it in advance.
    p->kebp = the_ebp;

    //drush8: S T E P 6.3: real final asm calling:
    int32_t result = asm_exec_end(prog_code_start, getUStack(childpid),(int32_t)(&p->kesp));
    return result;
}

//drush8's flag: can do some simplify work
int32_t halt (uint32_t status) {
    PCB_t * pcurrent = get_PCB();

    //drush8:S T E P 1: reset the KStack in TSS
    tss.esp0 = (uint32_t)getKStack(pcurrent->parent_pid);
    //tss.ss0 = (uint16_t)KERNEL_DS;  

    //drush8:S T E P 2: close all files.
    int i;
    for(i=0;i<MAX_FD;i++){
        if(pcurrent->fd_array[i].flags == 1)  close(i);
    }

    //drush8:S T E P 3: restore paging:

    paging_switch(pcurrent->pid,pcurrent->parent_pid);

    //drush8:S T E P 4: call asm: restore stack and will jmp
    PCB_t * pp = get_PCB_withpid(pcurrent->parent_pid);
    uint32_t kkesp = pp->kesp;
    uint32_t kkebp = pp->kebp;
    giveup_pid(pcurrent->pid);
    asm_halt_end(kkesp,kkebp,status);

    printf("halt err: seriously cannot return to parents\n");
    return 1;       //it should not be reached! or it is a serious big bug.
}



//LYS: check whether a file is executable (by leading 4 bytes magical number.)
//return value: if not exe, return -1. if is exe, return code virtual start addr of the program
int file_check(int32_t fd) {
    uint8_t buf[EXE_HEADER_BYTES];
    clear_file_position(fd);
    int bytes_read = read(fd, buf, EXE_HEADER_BYTES);
    uint32_t prog_start_vm=0;

    if (bytes_read != EXE_HEADER_BYTES) return -1;

    //now check for exe magic
    if (buf[0]!=EXE_MAG0) return -1;
    if (buf[1]!=EXE_MAG1) return -1;
    if (buf[2]!=EXE_MAG2) return -1;
    if (buf[3]!=EXE_MAG3) return -1;

    //it is a exe, return with program start virtal address
    prog_start_vm = (buf[27]<<(32-8)) + (buf[26]<<(24-8)) + (buf[25]<<(16-8)) + (buf[24]<<(8-8));
    return prog_start_vm;
}

//LYS: get the smallest available pid (not in use in pid_table) and return it.
//return value: available pid, or -1 if the pid_table is full
int32_t get_new_pid() {
    int i;
    for (i=0; i<MAX_PNUM; i++) {
        if (!pid_table[i]) {
            pid_table[i] = get_PCB_withpid(i);
            return i;
            }
    }
    return -1;
}

int giveup_pid(uint32_t pid){
    if(pid>=MAX_PNUM) return 2; //fail, overload our pid array
    if(pid_table[pid]==NULL) return 1; //already freed.
    pid_table[pid]=NULL;
    return 0; 
};

/*LYS
 *  clear_file_position: set file position back to 0
 */
void clear_file_position(int32_t fd) {
    PCB_t *PCB_ptr = get_PCB();
    PCB_ptr->fd_array[fd].file_position=0;
}



//LYS: set up PD for exe (map vm 128MB to phys 8MB+4MB*(pid-1), big page) and load the program image
//drush8: first, we now do the mapping in the loader (may fix it in the future)
// second: we will open a fd temporary in parent process to realize the load work.
//(we will close it then during the execute sys_call)
void loader(int32_t fd, uint32_t pid) {
    uint32_t prog_pid = pid;
    uint8_t *prog_img = (uint8_t *)PROG_LOAD_VM;
    map_4M_U(128*MB, 8*MB+4*MB*(prog_pid-1)); //we do flush TLB inside this
    //above warning drush8: give the page user privilege.
    clear_file_position(fd);
    read(fd, prog_img, 4*MB); //read will stop when we reach the file size, so no worry
}

//get the start pointer of the exec img(after the loading)
//drush8//int getexecStart(int32_t fd, int32_t pid){}//combined to the file_check


//warning: the value is vm address
int32_t getUStack(int32_t pid){
    return 128*MB+4*MB;
}

//warning: the value is pm address
int32_t getKStack(int32_t pid){
    return 8*MB - pid*8*KB;
}

//now the paging switch is a 'fake' one.
//we operate on the same pt & pd instead
void paging_switch(int old_pid, int new_pid){
    unmap_4M(128*MB, 8*MB+4*MB*(old_pid-1));
    if(new_pid!=0) map_4M_U(128*MB, 8*MB+4*MB*(new_pid-1));
}

//return the num of the args. and record the position of the args.
//we fill the args_pointer pair by pair
//start pointer stuff is valid char, end pointer stuff is invalid char.
int tiny_parse(int * args_pointer, const int8_t* command){
    int len = strlen(command);
    char meetflag = 0;
    int i,j=0;
    for (i=0;i<len;i++){
        if(j == MaxArgs*2) break;
        if(i >= MaxCommand) break;
        if(command[i]!='\0' && command[i]!='\n' && command[i]!= ' ' 
        && command[i]!='\t' && command[i]!='\r'){ 
            //ok, it is hopfully a char or num..
            if(meetflag == 0 ){
                args_pointer[j++] = i;
                meetflag = 1;
            }

        }
        else{
            //ok it is regarded as the blank.
            if(meetflag == 1){
                meetflag = 0;
                args_pointer[j++] = i;
            }
            if(command[i] == '\0'){
                //end of this string
                i++;//instead useless
                break;
            }
        }
    }

    if (meetflag == 0) return j/2;  //here, j should always be the even
    else{
        args_pointer[j++] = i;
        return j/2;
    }
}


//small assistance func, get args len.
int get_arg_len(int i, int j, const uint8_t* command){
    int len = 0;
    if(i>=j) return 0;
    while(i<j){
        if(command[i]!='\0' && command[i]!='\n' && command[i]!= ' ' 
        && command[i]!='\t' && command[i]!='\r'){
            len++;
            i++;
        }
        else return len;
    }
    return len;
}

