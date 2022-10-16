# handlers_asm.S - Assembly linkage for exceptions, keyboard and RTC handlers
# vim:ts=4 noexpandtab
# created by YMH on 2022/10/15

#define ASM     1

.global	Divide_Error
.global REVERSED
.global NMI_Interrupt
.global Breakpoint
.global Overflow
.global BOUND_Range_Exceeded
.global Invalid_Opcode
.global Device_Not_Available
.global Double_Fault
.global Coprocessor_Segment_Overrun
.global Invalid_TSS
.global Segment_Not_Present
.global Stack_Segment_Fault
.global General_Protection
.global Page_Fault
.global Floating_Point_Error
.global Alignment_Check
.global Machine_Check
.global Floating_Point_Exception

.global	Keyboard_Interrupt
.global	RTC_Interrupt
.global	System_calls


# for each exception, send its corresponding vector index to 
# a c function Handle_exceptions
Divide_Error:
    pushal
    pushfl
    pushl	$0
    jmp		common_interrupt
        
REVERSED:
    pushal
    pushfl
    pushl	$1
    jmp		common_interrupt

NMI_Interrupt:
    pushal
    pushfl
    pushl	$2
    jmp		common_interrupt

Breakpoint:
    pushal
    pushfl
    pushl	$3
    jmp		common_interrupt

Overflow:
    pushal
    pushfl
    pushl	$4
    jmp		common_interrupt

BOUND_Range_Exceeded:
    pushal
    pushfl
    pushl	$5
    jmp		common_interrupt

Invalid_Opcode:
    pushal
    pushfl
    pushl	$6
    jmp		common_interrupt

Device_Not_Available:
    pushal
    pushfl
    pushl	$7
    jmp		common_interrupt

Double_Fault:
    pushal
    pushfl
    pushl	$8
    jmp		common_interrupt

Coprocessor_Segment_Overrun:
    pushal
    pushfl
    pushl	$9
    jmp		common_interrupt

Invalid_TSS:
    pushal
    pushfl
    pushl	$10
    jmp		common_interrupt

Segment_Not_Present:
    pushal
    pushfl
    pushl	$11
    jmp		common_interrupt

Stack_Segment_Fault:
    pushal
    pushfl
    pushl	$12
    jmp		common_interrupt

General_Protection:
    pushal
    pushfl
    pushl	$13
    jmp		common_interrupt

Page_Fault:
    pushal
    pushfl
    pushl	$14
    jmp		common_interrupt

Floating_Point_Error:
    pushal
    pushfl
    pushl	$16
    jmp		common_interrupt

Alignment_Check:
    pushal
    pushfl
    pushl	$17
    jmp		common_interrupt

Machine_Check:
    pushal
    pushfl
    pushl	$18
    jmp		common_interrupt

Floating_Point_Exception:
    pushal
    pushfl
    pushl	$19
    jmp		common_interrupt


# call the c function Handle_exceptions in idt.c
common_interrupt:
    call	Print_exceptions
    addl	$4, %esp
    popal
    iret
    

# Keyboard_Interrupt assembly linkage
# This function calls the c function Handle_keyboard in keyboard.c
# Inputs   : none
# Outputs  : none
# Registers: save all registers
Keyboard_Interrupt:
#     cli
#     pushal
#     pushfl
#     call	keyboard_interrupt_handler
#     popfl
#     popal
#     sti
#     iret


# RTC_Interrupt assembly linkage
# This function calls the c function Handle_RTC in rtc.c
# Inputs   : none
# Outputs  : none
# Registers: save all registers
RTC_Interrupt:
#     cli
#     pushal
#     pushfl
#     call	Handle_RTC  # TO BE CHANGED---------
#     popfl
#     popal
#     sti
#     iret


# System_calls assembly linkage
# This function calls the c function Handle_system_calls in idt.c
# Inputs   : none
# Outputs  : none
# Registers: save all registers
System_calls:
    cli
    pushal
    pushfl
    call	Print_system_calls
    popfl
    popal
    sti
    iret



