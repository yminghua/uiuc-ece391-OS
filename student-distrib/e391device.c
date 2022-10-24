#include "lib.h"
#include "e391device.h"
#include "e391synchronize.h"
#include "i8259.h"
#include "intrexcenum.h"
#include "types.h"

//below is the macro for the RTC
#define RTC_TIMES 20        //no lock added yet, don't be bigger than 100 (will enter dead loop!)
#define RTC_SELECTOR 0x70
#define RTC_RW 0x71
#define RTC_RA 0x8A
#define RTC_RB 0x8B



/****
 * 
 * 
 * 
 * 
 * 
 * 
 * line:above is keyboard, below is the rtc.
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
*/


/*   rtc_handler
 *   Function: rtc_handler. dealed called by the interrupt.
 *   Inputs: none
 *   Return Value: none
 */
void rtc_handler(void){   //we wait for at least RTC_TIMES intrrputs. **We consider it is a 1 core machine so decide this simple code.
                      //if it is a muti-core, we need to have  independent structures for every cpu.
    static volatile int times = 0;
    times ++;
    send_eoi(RTC_IRQ);

    outb(0x0C,RTC_SELECTOR); //here we choose the RC. Why not begin at 0x8C? I don't know yet   0x0c just the RC address
    inb(RTC_RW);              //similar to the keyboard, you need to read the Register to show that your acception.

    if((times)!=1){
      return;         //ok, not the first one , you don't need to goto the while
    }
    sti();
    while(times<RTC_TIMES){// maybe when the loop break, the times is not exactly RTC_TIMES. (may be a little bigger?)
    }
    cli();      //re enable the critical section to set the time argument.
    times =0;
    ///
    if(if9pressed == 1) test_interrupts();
    /// @drush8: warning, this part of code is only used in the cp1.
    sti();//enable the intrrupts as soon as possible.here cp1 test printing should before the interrupt enable.
    return;
}

/*   rtc_init
 *   Function: init the keyboard input
 *   Inputs: none
 *   Return Value: none
 */
//warning: we need to call it in critical section
void rtc_init(void){
    //first step: set the frequency correctly.
    // for RA: UIP| DV2 1 0 | RS 3 2 1 0   PC set the divide-control bits 010 by default.
    //for the rate , we choose 32768 / (2^6) = 512 so rate is : 0b0111

    //warning: every op on RA&B(whatever r/w), we need reset the port.
    outb(RTC_RA, RTC_SELECTOR); 
    char va = inb(RTC_RW);
    outb(RTC_RA, RTC_SELECTOR); 
    outb((va | 0x07 | 0x20), RTC_RW); //0b010 and 0b0111

    //second step: open the interrupt enable bit for RegisterB
    //the 6th bit of RB is interrupt enable so it is 0b100,000 ->0x40s
    outb(RTC_RB, RTC_SELECTOR); 
    char vb = inb(RTC_RW);
    outb(RTC_RB, RTC_SELECTOR); 
    outb((vb | 0x40), RTC_RW); // 

    //final step: enable the irq.
    enable_irq(RTC_IRQ);  //unmask the irq of the rtc.
    return;
}

//create by drush8
