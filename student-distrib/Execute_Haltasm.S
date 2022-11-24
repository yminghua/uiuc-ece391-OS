//two asm c-referenced important func.
//create by drush8
#define ASM     1
#include "x86_desc.h"

.GLOBL asm_exec_end
.GLOBL asm_halt_return
.GLOBL asm_halt_end


/* 
 * end step of syscall execute
 * save final info for parent process;push necessary info for child process, then use the iret
 * 
 */

asm_exec_end:
    pushl	%ebx
	movl	8(%esp), %ebx     # start pointer (for eip)
	movl	12(%esp), %ecx    # ustack pointer
	movl	16(%esp), %edx    # kesp_loc
    movl    %esp,   (%edx)  # save the parent kstack in PCB.
    # begin the iret work:
    pushl   $USER_DS         # LYS. should be SS, but DS and SS makes no difference (CS is different for it contains previledge level)
    pushl   %ecx
    pushl   $0x00000202       # ????, pushfl won't work...
    pushl  $USER_CS
    pushl   %ebx
    iret                      # goes to the user program

asm_halt_return:              # halt return finally jumps to parent's senario location here
    popl    %ebx              # halt jmp here.
    ret                       # LYS. the return addr is pushed on stack as the 'asm_exec_end' call in execute c function.




/* 
 * end step of syscall halt
 * restore the stack info of the parent, and prepare for the parent's execute return value.
 * 
 */

asm_halt_end:       # the stack now used will soon be dropped
                    # so we don't care much about maintaining
    movl	4(%esp), %ebx     # kesp value
	movl	8(%esp), %ecx    # kebp value
    movl	12(%esp), %eax    # return value
    movl    %ecx, %ebp
    movl    %ebx, %esp
    # ok, now all is done...
    jmp asm_halt_return
