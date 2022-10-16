/* idt.h - Defines for the IDT initialization and usage
 * created by YMH on 2022/10/15
*/

#ifndef _ITD_H
#define _ITD_H

#include "x86_desc.h"
#include "lib.h"

/* Initialize the IDT */
void Init_IDT(void);
/* Handle all exceptions */
void Print_exceptions(int vec_id);
/* Handle system calls */
void Print_system_calls(void);

/* ---Exceptions Table--- */
void Divide_Error(void);
void REVERSED(void);
void NMI_Interrupt(void);
void Breakpoint(void);
void Overflow(void);
void BOUND_Range_Exceeded(void);
void Invalid_Opcode(void);
void Device_Not_Available(void);
void Double_Fault(void);
void Coprocessor_Segment_Overrun(void);
void Invalid_TSS(void);
void Segment_Not_Present(void);
void Stack_Segment_Fault(void);
void General_Protection(void);
void Page_Fault(void);
void Floating_Point_Error(void);
void Alignment_Check(void);
void Machine_Check(void);
void Floating_Point_Exception(void);

/* Interrupts from keyboard, RTC, and system_calls */
void Keyboard_Interrupt(void);
void RTC_Interrupt(void);
void System_calls(void);


#endif /* _ITD_H */
