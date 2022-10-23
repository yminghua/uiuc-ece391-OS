/* fileSystemModule.h - 
 * created by LYS on 2022/10/15
 */

#ifndef FILESYSTEMMODULE_H
#define FILESYSTEMMODULE_H

#include "types.h"

/**********************************************************************/
/*                                                                    */
/*                           Data Structure                           */
/*                                                                    */
/**********************************************************************/
typedef struct {
    uint8_t  file_name[32];
    uint32_t file_type; //1 for dir, 2 for data file, 0 for rtc file
    uint32_t inode_num;
} dentry_t;

typedef struct {
    uint32_t num_dentries;
    uint32_t num_inodes; 
    uint32_t num_data_blocks;
} bootBlock_t;

/**********************************************************************/
/*                                                                    */
/*                          Extern FUNCTIONS                          */
/*                                                                    */
/**********************************************************************/
extern int32_t read_dentry_by_name (uint8_t* fname, dentry_t* dentry);
extern int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);
extern int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);
extern void init_fileSys(uint32_t* filesys_addr);



#endif //FILESYSTEMMODULE_H