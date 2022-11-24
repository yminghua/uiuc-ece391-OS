/* scheduler.h - system call for execute and halt
 * created by LYS, JJL on 2022/11/23
 */

#include "PCB.h"

/**********************************************************************/
/*                                                                    */
/*                           Global Variable                          */
/*                                                                    */
/**********************************************************************/
//scheduler list
sche_node_t sche_list[3];
sche_node_t *cur_sche_node;

//video
#define BackupVP(x) (0xB8000 + 0x1000 * x) //back up video page for terminal 1, 2, 3

//visible terminal
uint32_t cur_visible_pid;





/**********************************************************************/
/*                                                                    */
/*                           Data Structure                           */
/*                                                                    */
/**********************************************************************/
typedef struct sche_node{
    PCB_t *pcb_ptr;
    sche_node_t *next;
    sche_node_t *prev;
} sche_node_t;