/* Execute_Halt.c - system call for execute and halt
 * created by LYS, JJL on 2022/10/29
 */

#include "Syscalls.h"
#include "fileSystem.h"
#include "x86_page.h"
#include "x86_desc.h"
#include "PCB.h"
#include "scheduler.h"

#define EXE_HEADER_BYTES 40
#define EXE_MAG0 0x7f
#define EXE_MAG1 0x45
#define EXE_MAG2 0x4c
#define EXE_MAG3 0x46
#define PROG_LOAD_VM 0x08048000

#define MaxArgs 6 //drush8's flag: we os now accept 10 args for the command at most.
#define MaxCommand 128



/*
 *  execute:  (in80_jump_table_pointed)
 *    DESCRIPTION: see manual for detail
 *    INPUTS: command
 *    OUTPUTS: set up new task with new (process)
 *    RETURN VALUE: 0 success, -1 failure.
 *    SIDE EFFECTS:
 */

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
        printf("no more task slot, quit...\n");
        return 1;//special situation, return 1 ...
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
        printf("file name %s does not exist, execute fail...", fname);
        giveup_pid(childpid);
        return -1;
    }
    prog_code_start = file_check(fd);
    if (prog_code_start==-1) {
        printf("file check file, %s is not an executable", fname);
        giveup_pid(childpid);
        return -1;
    }

    //drush8: S T E P 4: fill the PCB and open stdin/out for the child.
    PCB_t * p = get_PCB();
    init_PCB(childpid);
    pid_table[childpid]->parent_pid = p->pid;
    pid_table[childpid]->noterminal = p->noterminal; //for mp3.5:which one(terminal)//child follows parents
    if(p->visible == 1){
        p->visible = 0; 
        pid_table[childpid]->visible = 1;
    }
    else pid_table[childpid]->visible = 0;  //here, we set the visible property correctly...

    sche_list[p->noterminal].pcb_ptr = pid_table[childpid]; //maintain the list properties...

    openStdInOut(childpid);
    //step additional: fill the args
    fillPCBargs(pair_args_pointer,command, argsnum,pid_table[childpid],1);

    //4&5 excanged, because of loading will cover the location of previous arg info

    //drush8: S T E P 5: loader with paging
    loader(fd,childpid); //load program image into contiguous physical address
    //ok,over loading, we can close the file.

    close(fd);

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
    int32_t result = asm_exec_end(prog_code_start, getUStack(childpid),(int32_t)(&p->kesp));  //LYS: This return from the child program's halt, return value is the 'status' input of the halt function. I believe 0 means success and -1 means fail
    return result;  //This will return to the systemcall handler where we do iret again to go back to parent program user space
}


/*
 *  halt:  (in80_jump_table_pointed)
 *    DESCRIPTION: see manual for detail
 *    INPUTS: command
 *    OUTPUTS: rm/recycle/turn_down the task of the caller
 *    RETURN VALUE: none, 1 for unexpected failure...
 *    SIDE EFFECTS:
 */
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

    //additional for mp3.5: restore the parents' visible properties and then reset the schelist:
    if(pcurrent->visible == 1){
        pp->visible = 1;
    }
    else pp->visible = 0;  //here, we set the visible property correctly...
    sche_list[pp->noterminal].pcb_ptr = pp; //maintain the list properties...

    uint32_t kkesp = pp->kesp;
    uint32_t kkebp = pp->kebp;
    giveup_pid(pcurrent->pid);
    asm_halt_end(kkesp,kkebp,status);


    printf("halt err: seriously cannot return to parents\n");
    return 1;       //it should not be reached! or it is a serious big bug.
}



/************************************************************/
/************************************************************/
//             assistance functions                         //
/************************************************************/
/************************************************************/

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





/*LYS
 *  clear_file_position: set file position back to 0
 */
void clear_file_position(int32_t fd) {
    PCB_t *PCB_ptr = get_PCB();
    PCB_ptr->fd_array[fd].file_position=0;
}



//LYS: set up PD for exe (map vm 128MB to phys 8MB+4MB*(pid-1), big page) and load the program image
//make sure there has been one correspondign fd opened.

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

//page switch
//now the paging switch is a 'fake' one.
//we operate on the same pt & pd instead
void paging_switch(int old_pid, int new_pid){
    unmap_4M(128*MB);
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
                //end of this string// also we think the \n as end of the command line.
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

//fill the args to the PCB, mode 1 means give up the first arg:command/file arg.
int fillPCBargs(int *argsp, const uint8_t *command, int argnum, PCB_t *tarpcb,int mode){
    //by default, pcbarglen should be the same as the maximun command len.
    int i=0,j,start, end, status=0,pcbp=0;//0 means now is free status
    //below one command is unnecessary, drush8: but for safety...
    if(mode == 1) i=1;
    
    for(;i<argnum;i++){

        if(pcbp >= PCB_ARGLEN-1){      //ready to write the final.
            status = 1;
            break;
        }
        start = argsp[i*2];
        end = argsp[i*2+1];
        for(j=0;j<end-start;j++){


            if(pcbp == PCB_ARGLEN-1){   //ready to write the final.
                status = 1;
                break;
            }

            tarpcb->argstr[pcbp++] = command[start+j];
        }

        tarpcb->argstr[pcbp++] = ' ';

    }
    if (status == 0 ) tarpcb->argstr[pcbp-1] = '\0';
    tarpcb->argstr[PCB_ARGLEN-1] = '\0';  //always 0, if cannot fit it, the final arg element will be truncated
    return status;
}

//not used yey drush8
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

