/* Syscall_handler.s - handle int 0x80
 * created by LYS, JJL on 2022/10/29
 */

#define ASM     1
#include "x86_desc.h"

#define SYS_HALT    1
#define SYS_EXECUTE 2
#define SYS_READ    3
#define SYS_WRITE   4
#define SYS_OPEN    5
#define SYS_CLOSE   6
#define SYS_GETARGS 7
#define SYS_VIDMAP  8
#define SYS_SET_HANDLER  9
#define SYS_SIGRETURN  10

.global SYSTEM_CALL_handler



/* 
 * Rather than create a case for each number of arguments, we simplify
 * and use one macro for up to three arguments; the system calls should
 * ignore the other registers, and they're caller-saved anyway.
 */

#define Syscall_wrapper(name, number)   \
.GLOBL name                 ;\
name:                       ;\
    pushl	%ebx            ;\
	movl	$number, %eax   ;\
	movl	8(%esp), %ebx   ;\
	movl	12(%esp), %ecx  ;\
	movl	16(%esp), %edx  ;\
	int		$0x80           ;\
	popl	%ebx            ;\
	ret                     ;\
    


/* the system call library wrappers, it is brief, used for testing */
/* make sure that with prefix sys_, the call is used in the kernel for testing */
Syscall_wrapper(sys_halt, SYS_HALT)
Syscall_wrapper(sys_execute, SYS_EXECUTE)
Syscall_wrapper(sys_read, SYS_READ)
Syscall_wrapper(sys_write, SYS_WRITE)
Syscall_wrapper(sys_open, SYS_OPEN)
Syscall_wrapper(sys_close, SYS_CLOSE)
Syscall_wrapper(sys_getargs, SYS_GETARGS)
Syscall_wrapper(sys_vidmap, SYS_VIDMAP)
Syscall_wrapper(sys_set_handler, SYS_SET_HANDLER)
Syscall_wrapper(sys_sigreturn, SYS_SIGRETURN)
# drush8:  it is not our work, but can used for testing...


/* the system call handler: vector 0x80 */
SYSTEM_CALL_handler:
    pushfl
    pushw   %ds         // drush8: save the data seg
    pushl	%ebp
    pushl	%edi
    pushl	%esi
    pushl	%edx
    pushl	%ecx
    pushl	%ebx


    pushl %edx                  # 3rd arg first.
    pushl %ecx
    pushl %ebx                  # drush8: push the passing arguments for the sys_call c func


    movw    $KERNEL_DS, %di
    movw    %di, %ds            # drush8: we need to change the ds segment in level 0.

    # check if the system call number is valid
    cmpl	$1, %eax
    jb		invalid_syscall_number
    cmpl	$10, %eax
    ja		invalid_syscall_number

    # minus 1 to get the index in system call jump table
    addl	$-1, %eax
    

    call	*sys_call_table(, %eax, 4)
    jmp		done

invalid_syscall_number:
    movl	$-1, %eax   # return -1 for errors

done:
    addl    $12, %esp   # drush8: remove the 3 arguments.
    popl	%ebx
    popl	%ecx
    popl	%edx
    popl	%esi
    popl	%edi
    popl	%ebp
    popw    %ds     # drush8: restore the data seg..
    popfl

    iret


# System Call Jump Table
sys_call_table:
    .long halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn
