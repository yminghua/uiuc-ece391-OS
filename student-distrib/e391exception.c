#include "lib.h"
#include "e391exception.h"
#include "e391keyboard.h"
/*
 *  Handle_exceptions:
 *    DESCRIPTION: deal with all exception handlers.
 *    INPUTS: vec_id: vector index in the IDT
 *    OUTPUTS: none
 *    RETURN VALUE: none
 *    SIDE EFFECTS: current exception is handled
 */
void Print_exceptions(int vec_id){
    //maynot be correct after checkpoint1, we forbid the interrupt here. For page fault, we will add more feature in future.
    set_video_mem(nowterminalno);
    cli();
    //clear();
    register uint32_t save_cr2 ;
    //register uint32_t save_cr2 asm("cr2");
    switch (vec_id)
    {
    case 0:
        printf("Divide_Error!!!\n");
        break;

    case 1:
        printf("REVERSED!!!\n");
        break;
    
    case 2:
        printf("NMI_Interrupt!!!\n");
        break;

    case 3:
        printf("Breakpoint!!!\n");
        break;

    case 4:
        printf("Overflow!!!\n");
        break;

    case 5:
        printf("BOUND_Range_Exceeded!!!\n");
        break;

    case 6:
        printf("Invalid_Opcode!!!\n");
        break;

    case 7:
        printf("Device_Not_Available!!!\n");
        break;

    case 8:
        printf("Double_Fault!!!\n");
        break;

    case 9:
        printf("Coprocessor_Segment_Overrun!!!\n");
        break;

    case 10:
        printf("Invalid_TSS!!!\n");
        break;

    case 11:
        printf("Segment_Not_Present!!!\n");
        break;

    case 12:
        printf("Stack_Segment_Fault!!!\n");
        break;

    case 13:
        printf("General_Protection!!!\n");
        break;

    case 14:
        printf("Page_Fault!!! \n");
            asm volatile(
                "movl %%cr2, %%ebx;"
				"movl %%ebx, %0;"
				: "=a" (save_cr2)
				: 
				: "ebx"
	);
        printf("0x%#x",save_cr2);
        break;

    case 16:
        printf("Floating_Point_Error!!!\n");
        break;

    case 17:
        printf("Alignment_Check!!!\n");
        break;

    case 18:
        printf("Machine_Check!!!\n");
        break;

    case 19:
        printf("Floating_Point_Exception!!!\n");
        break;

    default:
        break;
    }
    while(1){}  //the blue screen forever
}

//below only used in cp1:drush8

/*
 *  Handle_system_calls:
 *    DESCRIPTION: deal with a system call.
 *    INPUTS: nonr
 *    OUTPUTS: none
 *    RETURN VALUE: none
 *    SIDE EFFECTS: print that a system call was 
 *                  called when the handler associated 
 *                  with 0x80 is executed
 */
void Print_system_calls(void){
    printf("A system call was called!\n");
}
