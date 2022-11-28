/* scheduler_asm_helper.S - system call for execute and halt
 * created by LYS on 2022/11/23
 */

#define ASM     1
#include "x86_desc.h"

.GLOBL scheduler_asm
.GLOBL asm_init_terminal_stack

/* 
 * scheduler_asm assembly helper: save old esp, do stack switch and ret
 */
 scheduler_asm:
    # save old esp and do stack switch
    movl	4(%esp), %edx     # kesp value
	movl	8(%esp), %ecx    # kebp value
    movl	12(%esp), %eax    # &p->esp
    movl	%esp, (%eax)
    movl	%edx, %esp
    movl	%ecx, %ebp
    
    ret



 /* 
  * asm_init_terminal_stack assembly wrapper: use this asm wrapper to assure kernel stack structure not corrupted
  */
asm_init_terminal_stack:
    pushl	%ebx
    pushl	%esi
	movl	12(%esp), %ebx    # terminal_esp
	movl	16(%esp), %ecx    # ustack pointer
	movl	20(%esp), %edx    # prog_code_start
    movl    %esp, %esi        # store current esp
    movl    %ebx, %esp        # switch stack to terminal process
    # push iret context (5 pushes)
    pushl   $USER_DS          # LYS. should be SS, but DS and SS makes no difference (CS is different for it contains previledge level)
    pushl   %ecx
    pushl   $0x00000202       # ????, pushfl won't work...
    pushl   $USER_CS
    pushl   %edx

    pushl   $pit_wrapper_init_ret_addr # push init ret addr on stack (return to 'iret' instruction)
    movl    %esp, %eax
    movl	%esi, %esp        # switch stack back
    popl	%esi
    popl    %ebx
    ret
    

