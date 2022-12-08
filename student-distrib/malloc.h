/* malloc.h - Defines for dynamic memory allocation
 * created by ymh on 2022/11/26
 */

#ifndef MALLOC_H
#define MALLOC_H

#include "types.h"

#define DYNAMIC_START_ADDR  0xFF800000  // 4088~4092 MB is for dynamic memory allocation
#define BLOCK_NUM 		    4096        // 4KB
#define ALLOCBLOCK_SIZE          1024        // 1KB

extern uint16_t memory_block_tracker[BLOCK_NUM];
void init_dynamic_malloc ();
void* malloc (uint32_t size);
void free (void* ptr);

#endif /* MALLOC_H */
