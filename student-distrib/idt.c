/* idt.c - Defines for the IDT initialization and usage
 * created by YMH on 2022/10/15
*/

#include "idt.h"

/*
 *  Init_IDT:
 *    DESCRIPTION: initialize the IDT.
 *    INPUTS: none
 *    OUTPUTS: none
 *    RETURN VALUE: none
 *    SIDE EFFECTS: the IDT is initialized
 */
void Init_IDT(void){
    int i;

    for (i = 0; i < 32; i++)    // initialize the exceptions part (vector 0-31)
    {
        idt[i].seg_selector = KERNEL_CS;
        idt[i].reserved4 = 0;
        idt[i].reserved3 = 1;   // according to Figure 5-2. IDT Gate Descriptors: Trap Gate in the ISA Reference Manual
        idt[i].reserved2 = 1;
        idt[i].reserved1 = 1;
        idt[i].size = 1;    // Size of gate: 1 = 32 bits
        idt[i].reserved0 = 0;
        idt[i].dpl = 0;     // Descriptor Privilege Level for kernel is 0
        idt[i].present = 1;
    }

    for (i = 32; i < NUM_VEC; i++)  // initialize the remainig part (vector 32-255)
    {
        idt[i].seg_selector = KERNEL_CS;
        idt[i].reserved4 = 0;
        idt[i].reserved3 = 0;   // according to Figure 5-2. IDT Gate Descriptors: intrrupt Gate in the ISA Reference Manual
        idt[i].reserved2 = 1;
        idt[i].reserved1 = 1;
        idt[i].size = 1;    // Size of gate: 1 = 32 bits
        idt[i].reserved0 = 0;
        idt[i].dpl = 3;     // Descriptor Privilege Level for kernel is 3
        idt[i].present = 0;
    }

    /* put exceptions table into the IDT*/
    SET_IDT_ENTRY(idt[0], Divide_Error);
    SET_IDT_ENTRY(idt[1], REVERSED);
    SET_IDT_ENTRY(idt[2], NMI_Interrupt);
    SET_IDT_ENTRY(idt[3], Breakpoint);
    SET_IDT_ENTRY(idt[4], Overflow);
    SET_IDT_ENTRY(idt[5], BOUND_Range_Exceeded);
    SET_IDT_ENTRY(idt[6], Invalid_Opcode);
    SET_IDT_ENTRY(idt[7], Device_Not_Available);
    SET_IDT_ENTRY(idt[8], Double_Fault);
    SET_IDT_ENTRY(idt[9], Coprocessor_Segment_Overrun);
    SET_IDT_ENTRY(idt[10], Invalid_TSS);
    SET_IDT_ENTRY(idt[11], Segment_Not_Present);
    SET_IDT_ENTRY(idt[12], Stack_Segment_Fault);
    SET_IDT_ENTRY(idt[13], General_Protection);
    SET_IDT_ENTRY(idt[14], Page_Fault);
    SET_IDT_ENTRY(idt[16], Floating_Point_Error);
    SET_IDT_ENTRY(idt[17], Alignment_Check);
    SET_IDT_ENTRY(idt[18], Machine_Check);
    SET_IDT_ENTRY(idt[19], Floating_Point_Exception);

    /* vector index for keyboard interrupt is 0x21 */
    SET_IDT_ENTRY(idt[0x28], rtc_handler_linkage);
    /* vector index for RTC interrupt is 0x28 */
    SET_IDT_ENTRY(idt[0x21], keyboard_handler_linkage);
    /* vector index for system calls is 0x80 */
    //may only used in cp1.
    SET_IDT_ENTRY(idt[0x80], System_calls);

}

