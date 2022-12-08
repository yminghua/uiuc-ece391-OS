/* malloc.c
 * created by ymh on 2022/11/26
 */

#include "malloc.h"

/* Dynamic memory usage tracker */
uint16_t memory_block_tracker[BLOCK_NUM];

extern void test_interrupts(void);

/*
 *  init_dynamic_malloc:
 *    DESCRIPTION: initialize the dynamic memory usage tracker
 *    INPUTS: none
 *    RETURN VALUE: none
 */
void init_dynamic_malloc() {
	int i = 0;
	for (i = 0; i < BLOCK_NUM; i++) {
		memory_block_tracker[i] = 0;  // set all memory blocks to free
	}
}


/*
 *  malloc:
 *    DESCRIPTION: perform dynamic memory allocation
 *    INPUTS: size - the memory size to be allocated
 *    RETURN VALUE: memory starting address if succeed, NULL if fail
 */
void* malloc(uint32_t size) {
    /* calculate the number of memory blocks need to be allocated */
    int num = size / ALLOCBLOCK_SIZE;
    if (size % ALLOCBLOCK_SIZE != 0)
        num++;

    int block_index = 0;
    while (block_index < BLOCK_NUM)
    {
        if (memory_block_tracker[block_index] != 0)
        {
            block_index += memory_block_tracker[block_index];   // skip the allocated blocks
        }
        else
        {
            int check = 1;
            int i;
            /* check if we have enough consecutive memory blocks */
            for ( i = block_index; i < (block_index + num); i++)
            {
                if (memory_block_tracker[i] != 0) {
                    check = 0;
                    break;
                }
            }
            if (check)
            {
                memory_block_tracker[block_index] = num;    // mark the block as allocated
                return ((void*)(DYNAMIC_START_ADDR + block_index * ALLOCBLOCK_SIZE));
            }
            block_index++;
        }
    }

    return NULL;
}


/*
 *  free:
 *    DESCRIPTION: free the allocted memory space
 *    INPUTS: ptr - starting address of the memory
 *    RETURN VALUE: none
 */
void free (void* ptr) {
    /* calculate the starting address of the ptr */
    int block_index = ((int)ptr - DYNAMIC_START_ADDR) / ALLOCBLOCK_SIZE;
    memory_block_tracker[block_index] = 0;  // free the memory
}
