/* signal_asm_helper.S - handle signal user stack
 * created by ymh
 */

#define ASM     1
#include "x86_desc.h"

.GLOBL signal_handler_stack_setup
.GLOBL sigreturn

para_handler: .long 0
para_signum:  .long 0
para_hw_context:    .long 0

user_ebp:      .long 0
user_esp:      .long 0

kernel_ebp:    .long 0
kernel_esp:    .long 0

return_address: .long 0

sigreturn_linkage:  movl	$10, %eax
                    int		$0x80


signal_handler_stack_setup:
    # prework - callee-saved register
    pushl  %ebp
    movl   %esp, %ebp

    pushl  %edi
    pushl  %esi
    pushl  %ebx

    # store the input parameters
    movl   8(%ebp), %ebx
    movl   %ebx, para_handler
    movl   12(%ebp), %ebx
    movl   %ebx, para_signum
    movl   16(%ebp), %ebx
    movl   %ebx, para_hw_context

    # store user ebp and esp
    movl   para_hw_context, %esi

    movl   20(%esi), %ebx
    movl   %ebx, user_ebp
    movl   60(%esi), %ebx
    movl   %ebx, user_esp

    # store kernel ebp and esp and then switch to user stack
    movl    %ebp, kernel_ebp
    movl    user_ebp, %ebp
    movl    %esp, kernel_esp
    movl    user_esp, %esp
    
    # two instructions in sigreturn_linkage, in total 7 bytes
    subl    $7, %esp
    movl    sigreturn_linkage, %eax
    movl	%eax, (%esp)
    movl    sigreturn_linkage+3, %eax
    movl    %eax, 3(%esp)

    # store the return address for sigreturn linkage
    movl    %esp, return_address
    subl    $1, %esp  # restore 4 bytes alignment

    # set the hard ware context for user level
    movl    para_hw_context, %esi
    
    pushl   64(%esi)    # ss
    pushl   60(%esi)    # esp
    pushl   56(%esi)    # eflags
    pushl   52(%esi)    # cs
    pushl   48(%esi)    # eip
    pushl   44(%esi)    # error
    pushl   40(%esi)    # irq
    pushl   36(%esi)    # fs
    pushl   32(%esi)    # es
    pushl   28(%esi)    # ds
    pushl   24(%esi)    # eax
    pushl   20(%esi)    # ebp
    pushl   16(%esi)    # edi
    pushl   12(%esi)    # esi
    pushl   8(%esi)     # edx
    pushl   4(%esi)     # ecx
    pushl   0(%esi)     # ebx

    # set paras for signal number, address of signal handler
    pushl   para_signum
    pushl   return_address

    # update esp on user stack for the IRET of system call linkage
    movl    %esp, 60(%esi)

    # call the user_level handler
    movl    para_handler, %edi
    movl    %edi, 48(%esi)

    # now we go back to current system call since IRET's context is set
    movl    kernel_ebp, %ebp
    movl    kernel_esp, %esp

    popl %ebx
    popl %esi
    popl %edi

    leave
    ret


sigreturn:
    # get user hw_context and kernel hw_context
    movl    64(%esp), %ebx  # user ESP
    addl    $4, %ebx   # user hw_context

    movl    %esp, %edi
    addl    $4, %edi   # kernel hw_context

    # Copy the hw_context
    andl    $0, %ecx  /* loop counter */
copy_hw_loop:
    movl    (%ebx, %ecx, 4), %eax 
    movl    %eax, (%edi, %ecx, 4)
    addl    $1, %ecx 
    cmpl    $17, %ecx   # hw_context has 17 elements
    jne     copy_hw_loop

    # Restore signal mask
    pushl   %eax 
    pushl   %ecx 
    pushl   %edx 
    call    restore_signal_mask
    popl    %edx 
    popl    %ecx   
    popl    %eax 
    
    # return the kernel hw_context's EAX
    movl    24(%edi), %eax 

    ret
