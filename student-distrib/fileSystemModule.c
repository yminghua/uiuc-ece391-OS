/* fileSystemModule.c - 
 * created by LYS on 2022/10/15
 */

//every passed in file name must be exact 32 bytes with zero padding! write a transform function to do that

#include "fileSystemModule.h"
#include "lib.h"

#define BLOCK_SIZE 4096  //block size in bytes
#define FNAME_LEN 32  //max length of file name in bytes
#define DENTRY_SIZE 64  //file system dentry size in bytes
#define FTYPE_OFF 32  //offset to get file type field
#define INODE_OFF 36  //offset to get inode field

static uint8_t* fileSys_addr;
static bootBlock_t bootBlock;

/*init_fileSys
 * description: load the start addr of file system into static variable fileSys_addr
 * return value: 0 on success, -1 on file not found
 */
void init_fileSys(uint32_t* filesys_addr) {
    fileSys_addr = (uint8_t *)filesys_addr;
    bootBlock.num_dentries = *fileSys_addr;
    bootBlock.num_inodes = *(fileSys_addr+FTYPE_OFF);
    bootBlock.num_data_blocks = *(fileSys_addr+INODE_OFF);
}

/*read_dentry_by_name
 * description: Search file by file name and put its file name, type and inode into dentry
 * return value: 0 on success, -1 on file not found
 */
int32_t read_dentry_by_name (uint8_t* fname, dentry_t* dentry) {
    uint32_t i = DENTRY_SIZE;
    uint8_t* cur_fname;
    while (i<BLOCK_SIZE) {
        cur_fname = *(fileSys_addr+i);
        if (!strncmp(fname, cur_fname, 32)) break;
    }
    if (i==BLOCK_SIZE) return -1;
    strncpy(dentry->file_name, cur_fname, 32);
    dentry->file_type = *(fileSys_addr+i+FTYPE_OFF);
    dentry->inode_num = *(fileSys_addr+i+INODE_OFF);
}

/*read_dentry_by_index
 * description: Search file by file index and put its file name, type and inode into dentry
 * return value: 0 on success, -1 on file not found
 */
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry) {
    uint32_t entry_ptr = (index+1)*DENTRY_SIZE;
    if (index >= bootBlock.num_dentries) return -1;
    strncpy(dentry->file_name, *(fileSys_addr+entry_ptr), 32);
    dentry->file_type = *(fileSys_addr+entry_ptr+FTYPE_OFF);
    dentry->inode_num = *(fileSys_addr+entry_ptr+INODE_OFF);
}

/*read_data
 * description: read length bytes of the file starting at offset into the buffer
 * return value: number of bytes read, or -1 if the inode number is invalid
 */
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length) {
    uint32_t N, D, inode_length, off_dblock, off_indblock, dblock_id, count=0;
    uint32_t *inode_ptr;
    uint8_t *byte_to_copy;

    N = bootBlock.num_inodes;
    D = bootBlock.num_data_blocks;
    inode_ptr = (inode+1)*BLOCK_SIZE + fileSys_addr;

    // check if inode is valid
    if (inode>=N) return -1;
    // check if offset >= inode_length
    inode_length = *inode_ptr;
    if (offset >= inode_length) return 0;

    //now start the read-data loop. remember to check the validity of data block number on each new data block
    off_dblock = offset/BLOCK_SIZE;
    off_indblock = offset - off_dblock*BLOCK_SIZE;
    dblock_id = *(inode_ptr+1+off_dblock);
    while ((off_dblock*BLOCK_SIZE+off_indblock<inode_length) && (count<length)) {
        byte_to_copy = fileSys_addr + (N+dblock_id+1)*BLOCK_SIZE;
        strncpy(buf+count, byte_to_copy, 1);
        count++;
        off_indblock++;
        if (off_indblock==BLOCK_SIZE) {
            off_indblock=0;
            off_dblock ++;
            dblock_id = *(inode_ptr+1+off_dblock);
        }
    }
    return count;
}
