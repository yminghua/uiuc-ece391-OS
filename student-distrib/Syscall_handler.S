/* Syscall_handler.s - handle int 0x80
 * created by LYS, JJL on 2022/10/29
 */


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
    

/* the system call library wrappers */
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



SYSTEM_CALL_handler:
    pushfl
    pushl	%ebp
    pushl	%edi
    pushl	%esi
    pushl	%edx
    pushl	%ecx
    pushl	%ebx

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
    popl	%ebx
    popl	%ecx
    popl	%edx
    popl	%esi
    popl	%edi
    popl	%ebp
    popfl

    iret


# System Call Jump Table
sys_call_table:
    .long halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn
