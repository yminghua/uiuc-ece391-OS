
//create by drush8
#define ASM     1
#include "x86_desc.h"

.GLOBL asm_exec_end
.GLOBL asm_halt_return
.GLOBL asm_halt_end

asm_exec_end:
    pushl	%ebx
	movl	8(%esp), %ebx     # start pointer
	movl	12(%esp), %ecx    # ustack pointer
	movl	16(%esp), %edx    # kesp_loc
    movl    %esp,   (%edx,,)  # save the parent kstack in PCB.
    # begin the iret work:
    pushw   $USER_DS
    pushl   %ecx
    pushfl
    pushw   $USER_CS
    pushl   %ebx
    iret                      # goes to the user program

asm_halt_return:
    popl    %ebx              # halt jmp here.
    ret


asm_halt_end:       # the stack now used will soon be dropped
                    # so we don't care much about maintaining
    movl	4(%esp), %ebx     # kesp value
	movl	8(%esp), %ecx    # kebp value
    movl	12(%esp), %eax    # return value
    movl    %ecx, %ebp
    movl    %ebx, %esp
    # ok, now all is done...
    jmp asm_halt_return
