/* Syscalls.c - all system call functions aside from execute and halt
 * created by LYS, JJL on 2022/10/29
 */

#include "Syscalls.h"


// PCB_t * PCB_current;  //consider the multi-core situation, finally we tries to get it everytimes in syscall.
//fixed by drush8


/* file operations table for each file type (stdin/stdout/rtc/dir/file) */
static fileOpT_t stdin_jtable = {terminal_read, terminal_fail, terminal_open, terminal_close};
static fileOpT_t stdout_jtable = {terminal_fail, terminal_write, terminal_open, terminal_close};
static fileOpT_t rtc_jtable = {rtc_read, rtc_write, rtc_open, rtc_close};
static fileOpT_t dir_jtable = {dir_read, dir_write, dir_open, dir_close};
static fileOpT_t file_jtable = {file_read, file_write, file_open, file_close};


//int32_t halt (uint32_t status) {
//    return 0;
//}
//int32_t execute (const uint8_t* command) {
//    return 0;
//}
/**consider the special character of the halt&execute, we prepare the separate files**/


/*
 *  read:
 *    DESCRIPTION: reads data form kb, a file, RTC, or a dir.
 *    INPUTS: fd: file descriptor
 *            buf: buffer to store the data
 *            nbytes: number of bytes to be read
 *    RETURN VALUE: number of successfully read bytes
 */
int32_t read(int32_t fd, void* buf, int32_t nbytes) {

    PCB_t * PCB_current = get_PCB(); //

    /* check if inputs are valid */
    if(fd < 0 || fd >= MAX_FD || PCB_current->fd_array[fd].flags == 0 || buf == NULL || nbytes < 0)
        return -1;
    
    /* call the file-type-specific read function */
    int ret = PCB_current->fd_array[fd].fileOpT_ptr.read(fd, buf, nbytes);

    if(ret != -1 && ret != 0)
        if(PCB_current->fd_array[fd].fileOpT_ptr.read == dir_read ) PCB_current->fd_array[fd].file_position += 1;
        if(PCB_current->fd_array[fd].fileOpT_ptr.read == file_read ) PCB_current->fd_array[fd].file_position += ret;  // update the file position in fd_array after reading
    
    return ret;
}


/*
 *  write:
 *    DESCRIPTION: writes data to termianl or RTC,
 *                 and return -1 for regular file.
 *    INPUTS: fd: file descriptor
 *            buf: buffer to store the data
 *            nbytes: number of bytes to be read
 *    RETURN VALUE: number of bytes written, or -1 on failure
 */
int32_t write(int32_t fd, const void* buf, int32_t nbytes) {

    PCB_t * PCB_current = get_PCB();

    /* check if inputs are valid */
    if(fd < 0 || fd >= MAX_FD || PCB_current->fd_array[fd].flags == 0 || buf == NULL || nbytes < 0)
        return -1;

    /* call the file-type-specific write function */
    int ret = PCB_current->fd_array[fd].fileOpT_ptr.write(fd, buf, nbytes);

    return ret;
}


/*
 *  open:
 *    DESCRIPTION: find the dir entry corresponding to the filename, allocate
 *                 an unused fd, and set up any data necessary to handle the given
 *                 type of file(RTC/dir/file).
 *    INPUTS: filename: file name
 *    RETURN VALUE: fd, or -1 on failure
 */
int32_t open(const uint8_t* filename) {

    PCB_t * PCB_current = get_PCB();

    int fd = -1;
    dentry_t dentry;

    /* find the corresponding dentry through filename */
    if(read_dentry_by_name((uint8_t*)filename, &dentry) == -1)
        return -1;

    int i = 2;  // start from 2 since 0 and 1 are used as default fd
                // drush'sflag: may change for redirection
    while (i < MAX_FD)
    {
        /* allocate an unused fd */
        if(PCB_current->fd_array[i].flags == 0) {
            fd = i;
            break;
        }
        i++;
    }
    if(fd == -1)
        return -1;  // no fd is free

    /* set up the fd data */
    PCB_current->fd_array[fd].flags = 1;
    PCB_current->fd_array[fd].file_position = 0;
    PCB_current->fd_array[fd].inode_index = 0;

    /* set up the file operations table and inode_index(for file only) */
    switch (dentry.file_type)
    {
    case 0:
        /* file type: rtc */
        PCB_current->fd_array[fd].fileOpT_ptr = rtc_jtable;
        break;

    case 1:
        /* file type: dir */
        PCB_current->fd_array[fd].fileOpT_ptr = dir_jtable;
        break;

    case 2:
        /* file type: file */
        PCB_current->fd_array[fd].fileOpT_ptr = file_jtable;
        PCB_current->fd_array[fd].inode_index = dentry.inode_index;
        break;
    
    default:
        break;
    }

    /* call the file-type-specific open function */
    PCB_current->fd_array[fd].fileOpT_ptr.open(filename);

    return fd;
}


/*
 *  close:
 *    DESCRIPTION: closes the fd and makes it available for
 *                 return from later calls to open.
 *    INPUTS: fd: file descriptor
 *    RETURN VALUE: 0 on successful closes, -1 on failure
 */
int32_t close(int32_t fd) {

    PCB_t * PCB_current = get_PCB();

    /* check if fd is valid */
    if(fd < 2 || fd >= MAX_FD)   // not allow to close default fd (0 for input and 1 for output)
        return -1;
    if(PCB_current->fd_array[fd].flags == 0) return -1;  //cannota close an unopened fd.

    /* call the file-type-specific close function */
    int ret = PCB_current->fd_array[fd].fileOpT_ptr.close(fd);
    if(ret == 0)
        PCB_current->fd_array[fd].flags = 0;     // successful close, make this fd available

    return ret;
}


int32_t getargs (uint8_t* buf, int32_t nbytes) {
    PCB_t * PCB_current = get_PCB();
    int i,status = -1;
    for(i=0;i<nbytes;i++){
        buf[i] = PCB_current->argstr[i];
        if(PCB_current->argstr[i]=='\0'){
            status = 0;
            break;
        }
    }
    if(i==0) status = -1;
    return status;
}


int32_t vidmap (uint8_t** screen_start) {
    //PCB_t * PCB_current = get_PCB();
    return 0;
}


int32_t set_handler (int32_t signum, void* handler_address) {
    //PCB_t * PCB_current = get_PCB();
    return -1;
}


int32_t sigreturn (void) {
    //PCB_t * PCB_current = get_PCB();
    return -1;
}


/* --------------------------- assistance functions ----------------------------- */

int32_t openStdInOut(int pid){
    PCB_t * PCB_current = (PCB_t *)(8*MB - (pid+1)*8*KB); //get the pid's pcb
    /* Initialze STDIN */
    PCB_current->fd_array[0].fileOpT_ptr = stdin_jtable;
    PCB_current->fd_array[0].flags = 1;
    stdin_jtable.open(NULL);

    /* Initialize STDOUT */
    PCB_current->fd_array[1].fileOpT_ptr = stdout_jtable;
    PCB_current->fd_array[1].flags = 1;
    stdout_jtable.open(NULL);
    return 0;
}   
//drush8: we never close the stdin/out, except that we halt this 

/* --------------------------- test functions ----------------------------- */

void Syscalls_test_file() {

    PCB_t * PCB_current = get_PCB();
    /* Initialize PCB */
    int i;
    /* Set FD array */
    for(i = 0; i < MAX_FD ; i++)
    {
        PCB_current->fd_array[i].fileOpT_ptr.read = 0;
        PCB_current->fd_array[i].fileOpT_ptr.write = 0;
        PCB_current->fd_array[i].fileOpT_ptr.open = 0;
        PCB_current->fd_array[i].fileOpT_ptr.close = 0;
        PCB_current->fd_array[i].flags = 0;
        PCB_current->fd_array[i].file_position = 0;
        PCB_current->fd_array[i].inode_index = 0;
    }

    /* Initialze STDIN */
    PCB_current->fd_array[0].fileOpT_ptr = stdin_jtable;
    PCB_current->fd_array[0].flags = 1;

    /* Initialize STDOUT */
    PCB_current->fd_array[1].fileOpT_ptr = stdout_jtable;
    PCB_current->fd_array[1].flags = 1;


    fill_fname_list();
    int fd;

    uint8_t* fname = all_fname_list[10];
    printf((int8_t*)fname);
    printf(":\n");

    sys_open(fname);
    sys_open(fname);
    sys_open(fname);
    sys_open(fname);
    sys_open(fname);
    sys_open(fname);

    // no free fd now

    fd = sys_open(fname);
    if(fd == -1)
        printf("Fail to allocate fd!\n");
    else
        printf("fd: %d\n", fd);

    sys_close(5);
    // fd = 5 is free now
    fd = sys_open(fname);
    if(fd == -1)
        printf("Fail to allocate fd!\n");
    else
        printf("fd: %d\n", fd);

    int32_t count;
    int new_line = 0;
	uint8_t buf[100000];    // set large enough number for buffer size
    count = sys_read(fd, buf, 100000);
	printf("Successfully read %d Bytes!\n", count);
	for (i = 0; i < count; i++) {
        if (buf[i] != '\0') {
            putc(buf[i]);
            new_line++;
            if (buf[i]=='\n') new_line=0;
            if (new_line>=80) {putc('\n'); new_line=0;}     // number of display screen column is 80
        }
	}

    int ret;
    // write to fd = 5
    ret = sys_write(5, buf, 100000);
    if(ret == -1)
        printf("Read-only file!\n");

}


void Syscalls_test_dir() {

    PCB_t * PCB_current = get_PCB();
    clear();

    /* Initialize PCB */
    int i;
    /* Set FD array */
    for(i = 0; i < MAX_FD ; i++)
    {
        PCB_current->fd_array[i].fileOpT_ptr.read = 0;
        PCB_current->fd_array[i].fileOpT_ptr.write = 0;
        PCB_current->fd_array[i].fileOpT_ptr.open = 0;
        PCB_current->fd_array[i].fileOpT_ptr.close = 0;
        PCB_current->fd_array[i].flags = 0;
        PCB_current->fd_array[i].file_position = 0;
        PCB_current->fd_array[i].inode_index = 0;
    }

    /* Initialze STDIN */
    PCB_current->fd_array[0].fileOpT_ptr = stdin_jtable;
    PCB_current->fd_array[0].flags = 1;

    /* Initialize STDOUT */
    PCB_current->fd_array[1].fileOpT_ptr = stdout_jtable;
    PCB_current->fd_array[1].flags = 1;

    fill_fname_list();
    
    int fd;
    fd = sys_open(all_fname_list[0]);

    for (i = 0; i < 17; i++)    // print out the information of all 17 files and dir
    {
        uint8_t buf[33];    // set larger number for buffer size (33 > 32)
        buf[32] = 0;    // set the last char to 0 since max file name size is 32

        sys_read(fd, buf, 0);

        dentry_t d_dentry;
        read_dentry_by_name(buf, &d_dentry);

        /* print out the file information */
        int name_area=35, fn_len, j;    // total name area size is 35 in our design
        uint32_t *inode_ptr;
        // print file properties: name, type and size
        fn_len = strlen((int8_t*)d_dentry.file_name);
        if (fn_len > FNAME_LEN)
            fn_len = FNAME_LEN;
        printf("file_name:");
        for (j=0; j<name_area-fn_len; j++) printf(" ");
        printf((int8_t*)buf);
        printf(", ");
        printf("file_type: %d", d_dentry.file_type);
        printf(", ");
        inode_ptr = (uint32_t*)(fileSys_addr+(d_dentry.inode_index + 1)*BLOCK_SIZE);
        printf("file_size: %d\n", *inode_ptr);
    }

    sys_close(fd);

}


void Syscalls_test_terminal() {

    PCB_t * PCB_current = get_PCB();
    clear();

    /* Initialize PCB */
    int i;
    /* Set FD array */
    for(i = 0; i < MAX_FD ; i++)
    {
        PCB_current->fd_array[i].fileOpT_ptr.read = 0;
        PCB_current->fd_array[i].fileOpT_ptr.write = 0;
        PCB_current->fd_array[i].fileOpT_ptr.open = 0;
        PCB_current->fd_array[i].fileOpT_ptr.close = 0;
        PCB_current->fd_array[i].flags = 0;
        PCB_current->fd_array[i].file_position = 0;
        PCB_current->fd_array[i].inode_index = 0;
    }

    /* Initialze STDIN */
    PCB_current->fd_array[0].fileOpT_ptr = stdin_jtable;
    PCB_current->fd_array[0].flags = 1;

    /* Initialize STDOUT */
    PCB_current->fd_array[1].fileOpT_ptr = stdout_jtable;
    PCB_current->fd_array[1].flags = 1;

    printf("\n\n kb testing, type what you want(maximun128 char)...\n");
	printf("set 8 to make sure that test goes to terminal tests.\n");
	while(if9pressed!=-2);
	printf("terminal testing, type lines to see if it is correct.\n");
	printf("lines containing 9 or 8 or 7 will terminate this tests .\n");
	int num=40,readnum;
	char testbuf[40];

    PCB_current->fd_array[1].fileOpT_ptr.open(NULL);
    while(1){
        readnum = sys_read(1, (void *)testbuf, num);
		if(sys_write(1, (void *)testbuf, readnum) > readnum) break;
		if(if9pressed==0){
			break;
		}
	}

}
