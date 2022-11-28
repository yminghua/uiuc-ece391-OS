/* scheduler.h - system call for execute and halt
 * created by LYS, JJL on 2022/11/23
 */

#include "PCB.h"


/**********************************************************************/
/*                                                                    */
/*                           Data Structure                           */
/*                                                                    */
/**********************************************************************/
typedef struct sche_node{
    PCB_t *pcb_ptr;
    struct sche_node *next;
    struct sche_node *prev;
} sche_node_t;

/**********************************************************************/
/*                                                                    */
/*                     Global Variable and Macro                      */
/*                                                                    */
/**********************************************************************/
//scheduler list
extern sche_node_t sche_list[3];
extern sche_node_t *cur_sche_node;

//video
#define BackupVP(x) (0xB8000 + 0x1000 * x) //back up video page for terminal 1, 2, 3

//visible terminal
extern uint32_t cur_visible_terminal;


/**********************************************************************/
/*                                                                    */
/*                             Functions                              */
/*                                                                    */
/**********************************************************************/
//from scheduler.c
extern void scheduler();
extern void init_multiple_terminal();
extern void _init_multiple_terminal();
void sche_main();

//from scheduler_asm_helper.S
extern void scheduler_asm(uint32_t kesp, uint32_t kebp, uint32_t kespaddr);
extern int asm_init_terminal_stack(uint32_t kstack_ptr, uint32_t ustack_ptr, uint32_t exe_ptr);
extern int aasm_init_terminal_stack(uint32_t kstack_ptr, uint32_t ustack_ptr, uint32_t exe_ptr);



/**********************************************************************/
/*                                                                    */
/*                           PIT stuff                                */
/*                                                                    */
/**********************************************************************/

#define CCZERO 0x40
#define CMDPORT 0x43

void set_pit_count(uint16_t count);
void pit_init();



