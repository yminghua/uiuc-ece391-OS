/* fileSystem.h - Defines for the file system's open, close, read and write
 * created by YMH on 2022/10/22
*/

#ifndef _FILESYSTEM_H
#define _FILESYSTEM_H

#include "lib.h"


/****************** File System Structure *********************/

typedef struct dentry {
    uint8_t file_name[32];
    uint32_t file_type;
    uint32_t inode_index;
} dentry_t;

typedef struct bootBlock {
    uint32_t num_dentries;
    uint32_t num_inodes; 
    uint32_t num_data_blocks;
} bootBlock_t;

/************************** file descriptor ***********************************/

typedef struct fileOperations_table {
    int32_t (*open)(const uint8_t* filename, int fd);
    int32_t (*close)(int32_t fd);
    int32_t (*read)(int32_t fd, void* buf, int32_t nbytes);
    int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);
} fileOpT_t;

/* File Object structure - for PCB */
typedef struct fdInfo {
    fileOpT_t fileOpT_ptr;     /* Only valid for data file */
    uint32_t inode_index;
    uint32_t file_position;       /* Current position in file, updated by system calls */
    uint32_t flags;          /* If flag is set, file object is in use */
} fdInfo_t;



/***************************** file module ***********************************/

extern int32_t read_dentry_by_name (uint8_t* fname, dentry_t* dentry);

extern int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);

extern int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

extern void init_fileSys(uint32_t* filesys_addr);

/**************** file operations ****************/

 extern int32_t file_read(int32_t fd, void* buf, int32_t nbytes);

 extern int32_t file_write(int32_t fd, const void* buf, int32_t nbytes);

 extern int32_t file_open(const uint8_t* filename, int fd);

 extern int32_t file_close(int32_t fd);

/**************** dir operations ****************/

 extern int32_t dir_read(int32_t fd, void* buf, int32_t nbytes);

 extern int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes);

 extern int32_t dir_open(const uint8_t* filename);

 extern int32_t dir_close(int32_t fd);

#endif /* _FILESYSTEM_H */
