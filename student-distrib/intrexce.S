//# include "intrexcenum.h"
//# include "e391device.h"
//# include "e391exception.h"
//for safety, we add the header func. Instead it is unnecessary.BUT ERROR.
//create by drush8

#define ASM     1
#include "x86_desc.h"

/* drush8:
 * Rather than create a case for each number of arguments, we simplify
 * use a macro. Well just learn form the sys_call.
 */


//we defined the assambly linkage macro to compute all the entrance of the interrrupt.
#define INTR_LINK(name,func)   \
.GLOBL name                   ;\
name:                         ;\
	pushw %ds				  ;\
	pushal                    ;\
	pushfl                    ;\

    movw    $KERNEL_DS, %di   ;\
    movw    %di, %ds          ;\ 
	 
	call func                 ;\
	popfl                     ;\
	popal                     ;\
	popw %ds				  ;\
	iret                      ;\

#define INTR_LINKR(name,func,n)\
.GLOBL name                   ;\
name:                         ;\
	pushw %ds				  ;\
	pushal                    ;\
	pushfl                    ;\

	movw    $KERNEL_DS, %di   ;\
    movw    %di, %ds          ;\

    pushl   $n                 ;\
	call func                 ;\
    addl	$4, %esp          ;\
	popfl                     ;\
	popal                     ;\
	popw %ds				  ;\
	iret                      ;\

/* the interrupt_handler wrappers */
INTR_LINK(rtc_handler_linkage, rtc_handler)
INTR_LINK(keyboard_handler_linkage,keyboard_handler)

/* the excption_handler wrappers */
//thanks to ymh
INTR_LINKR(Divide_Error, Print_exceptions,0 )
INTR_LINKR(REVERSED, Print_exceptions, 1)
INTR_LINKR(NMI_Interrupt, Print_exceptions,2 )
INTR_LINKR(Breakpoint, Print_exceptions,3 )
INTR_LINKR(Overflow, Print_exceptions,4 )
INTR_LINKR(BOUND_Range_Exceeded, Print_exceptions,5 )
INTR_LINKR(Invalid_Opcode, Print_exceptions,6 )
INTR_LINKR(Device_Not_Available, Print_exceptions,7 )
INTR_LINKR(Double_Fault, Print_exceptions,8 )
INTR_LINKR(Coprocessor_Segment_Overrun, Print_exceptions,9 )
INTR_LINKR(Invalid_TSS, Print_exceptions,10 )
INTR_LINKR(Segment_Not_Present, Print_exceptions,11 )
INTR_LINKR(Stack_Segment_Fault, Print_exceptions, 12)
INTR_LINKR(General_Protection, Print_exceptions, 13)
INTR_LINKR(Page_Fault, Print_exceptions,14 )
INTR_LINKR(Floating_Point_Error, Print_exceptions,16 )
INTR_LINKR(Alignment_Check, Print_exceptions,17 )
INTR_LINKR(Machine_Check, Print_exceptions, 18)
INTR_LINKR(Floating_Point_Exception, Print_exceptions,19)


# //for syscall, instead we should not add it here
# //the code below is just for cp1
# //drush8
# .global	System_calls


# System_calls:
#     cli
#     pushal
#     pushfl
#     call	Print_system_calls
#     popfl
#     popal
#     sti
#     iret

