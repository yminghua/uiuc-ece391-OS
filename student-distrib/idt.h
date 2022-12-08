/* idt.h - Defines for the IDT initialization and usage
 * created by YMH on 2022/10/15
*/

#ifndef _ITD_H
#define _ITD_H

#include "x86_desc.h"
#include "lib.h"

typedef struct hw_context {
    int32_t ebx;
    int32_t ecx;
    int32_t edx;
    int32_t esi;
    int32_t edi;
    int32_t ebp;
    int32_t eax;
    uint32_t ds;
    uint32_t es;
    uint32_t fs;
    uint32_t irq_or_excep;  // IRQ num or exception num
    uint32_t err_code_or_dummy;  // error code for exception or dummy
    uint32_t eip;  // return address
    uint16_t cs;
    uint32_t eflags;
    uint32_t esp;
    uint32_t ss;
} hw_context_t;

/* Initialize the IDT */
void Init_IDT(void);
/* Handle all exceptions */
void Print_exceptions(hw_context_t hw);
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
extern void rtc_handler_linkage();
extern void keyboard_handler_linkage(); //2022.10.16. Now only two interrupt.
extern void pit_handler_linkage();      //mp3.5: 3 ..


void SYSTEM_CALL_handler(void);


/* Test functions*/
void test_System_calls(void);
void test_Divide_Error(void);
void test_REVERSED(void);


#endif /* _ITD_H */
