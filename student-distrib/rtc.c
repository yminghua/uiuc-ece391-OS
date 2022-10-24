#include "lib.h"
#include "rtc.h"
#include "e391synchronize.h"
#include "i8259.h"
#include "intrexcenum.h"
#include "types.h"

//below is the macro for the RTC
#define RTC_TIMES 20        //no lock added yet, don't be bigger than 100 (will enter dead loop!)
#define RTC_SELECTOR 0x70
#define RTC_RW 0x71
#define RTC_RA 0x8A
#define RTC_RB 0x8B         //ABOVE ARE all ports num


/*   rtc_handler
 *   Function: rtc_handler. dealed called by the interrupt.
 *   Inputs: none
 *   Return Value: none
 */
void rtc_handler(void){   //we wait for at least RTC_TIMES intrrputs. **We consider it is a 1 core machine so decide this simple code.
                      //if it is a muti-core, we need to have  independent structures for every cpu.
    
    send_eoi(RTC_IRQ);

    rtc_interrupt_occur=1; // for rtc_read

    outb(0x0C,RTC_SELECTOR); //here we choose the RC. Why not begin at 0x8C? I don't know yet   0x0c just the RC address
    inb(RTC_RW);              //similar to the keyboard, you need to read the Register to show that your acception.

    
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


    rtc_interrupt_occur=0;

    return;
}

/* rtc_open
 * Description: Open the RTC, set f to 2Hz
 * Inputs:      none
 * Outputs:     0 success, -1 fail
 */
int rtc_open(const unsigned char* filename){

    // just set the f to 2 Hz
    return rtc_reset_freq(2);
}

/* rtc_read
 * Description: Read frequency of RTC
 * Inputs:      fd, buf， nbytes
 * Outputs:  0 success
 * Function:  Read once interrupt occur
 */
int rtc_read(int fd, void* buf, int nbytes){

    rtc_interrupt_occur=0;
    // wait

    while(!rtc_interrupt_occur){}

    // reset the rtc_interrupt_occur
    rtc_interrupt_occur=0;

    return 0; // return success
}

/* rtc_write
 * Description: set rtc frequency
 * Inputs:      buf, where frequency to be set. nbytes only 4 to work
 * Outputs:     0 success, -1 fail
 */
int rtc_write(int fd, const void* buf, int nbytes){

    //check the input
    if( buf==NULL || nbytes!=4 ){     
        return -1;                  //Failure
    }

    //set the freq by using buf
    return rtc_reset_freq(*(int32_t*)buf);
}

/* rtc_close
 * Description: close rtc
 * Inputs:      fd, file descriptor
 * Outputs:     none
 */
int rtc_close(int fd){
    return 0;
}


/* rtc_reset_freq
 * Description: reset freq to register A, used for open and write
 * Inputs: freq(HZ)
 * Outputs: 0 success, -1 fail
 */
int rtc_reset_freq(int freq){

    // find the correct input for A register
    
    int judge;
    int inputa=-1;
    int judge2=1;
    // max is 1024, 2^10
    for(judge=1;judge<=10;judge++){
        judge2 *=2; // to get to power of 2
        if(judge2==freq){

            // inputa = 16 - log2(freq)
            inputa=16-judge;
            
            break;
        }
    }

    // check not 2^n data, print wrong
    if(inputa==-1){
        //printf("\n illegal input freq, must be the power of 2 and <1024 \n");
        return -1;
    }

    // set the freq
    outb( RTC_RA , RTC_SELECTOR);
    outb( (uint8_t)(inputa) , RTC_RW);
    return 0;   //Success

}


