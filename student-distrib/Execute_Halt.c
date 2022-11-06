/* Execute_Halt.c - system call for execute and halt
 * created by LYS, JJL on 2022/10/29
 */

#include "Syscalls.h"
#include "fileSystem.h"
#include "x86_page.h"

#define EXE_HEADER_BYTES 40
#define EXE_MAG0 0x7f
#define EXE_MAG1 0x45
#define EXE_MAG2 0x4c
#define EXE_MAG3 0x46
#define PROG_LOAD_VM 0x08048000

int32_t execute (const uint8_t* command) {
    uint8_t* fname; //get this file name after parsing the command
    uint32_t prog_code_start;
    int32_t fd;

    //LYS: open and check file and copy prog image section
    fd = open((uint8_t*)fname);
    if (fd==-1) {
        printf("file name %s does not exist, execute fail", fname);
        return -1;
    }
    prog_code_start = file_check(fd);
    if (prog_code_start==-1) {
        printf("file check file, %s is not an executable", fname);
    }
    loader(fd); //load program image into contiguous physical address
}


int32_t halt (uint32_t status) {

}

//LYS: check whether a file is executable (by leading 4 bytes magical number.)
//return value: if not exe, return -1. if is exe, return code virtual start addr of the program
int file_check(int32_t fd) {
    uint8_t buf[EXE_HEADER_BYTES];
    int bytes_read = read(fd, buf, EXE_HEADER_BYTES);
    uint32_t prog_start_vm=0;

    if (bytes_read != EXE_HEADER_BYTES) return -1;

    //now check for exe magic
    if (buf[0]!=EXE_MAG0) return -1;
    if (buf[1]!=EXE_MAG1) return -1;
    if (buf[2]!=EXE_MAG2) return -1;
    if (buf[3]!=EXE_MAG3) return -1;

    //it is a exe, return with program start virtal address
    prog_start_vm = (buf[24]<<(32-8)) + (buf[25]<<(24-8)) + (buf[26]<<(16-8)) + (buf[27]<<(8-8));
    return prog_start_vm;
}

//LYS: get the smallest available pid (not in use in pid_table) and return it.
//return value: available pid, or -1 if the pid_table is full
uint32_t get_new_pid() {
    int i;
    for (i=0; i<MAX_PNUM; i++) {
        if (!pid_table[i]) return i;
    }
    return -1;
}

/*LYS
 *  clear_file_position: set file position back to 0
 */
void clear_file_position(int32_t fd) {
    PCB_t *PCB_ptr = get_PCB();
    PCB_ptr->fd_array[fd].file_position=0;
}

//LYS: set up PD for exe (map vm 128MB to phys 8MB+4MB*(pid-1), big page) and load the program image
void loader(int32_t fd) {
    uint32_t prog_pid = get_new_pid();
    uint8_t *prog_img = PROG_LOAD_VM;
    map_4M(128*MB, 8*MB+4*MB*(prog_pid-1)); //we do flush TLB inside this
    clear_file_position(fd);
    read(fd, prog_img, 4*MB); //read will stop when we reach the file size, so no worry
}

