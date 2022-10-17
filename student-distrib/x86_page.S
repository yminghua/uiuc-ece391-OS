# x86_page.S - Set up x86 page directory and page table
# created by LYS on 2022/10/15 

#define ASM     1
#include "x86_page.h"



/**********************************************************************/
/*                                                                    */
/*                           Global Variables                         */
/*                                                                    */
/**********************************************************************/

.text

.globl PD, PT, PD_size, PT_size




/**********************************************************************/
/*                                                                    */
/*                           Memory Assign                            */
/*                                                                    */
/**********************************************************************/
.align 4

PD_size:
    .long PD_bottom - PD - 1

PT_size:
    .long PT_bottom - PT - 1

#1024 PDEs and 1024 PTEs
#.align 0x1000
PD:
    .rept 1024
    .long 0
    .endr
PD_bottom:

#.align 0x1000
PT:
    .rept 1024
    .long 0
    .endr
PT_bottom:

