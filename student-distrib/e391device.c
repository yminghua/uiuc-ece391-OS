#include "lib.h"
#include "e391device.h"
#include "e391synchronize.h"
#include "i8259.h"
#include "intrexcenum.h"
#include "types.h"

//below is the macro for the RTC
#define RTC_TIMES 1>>3        //youcan choose the 1, 10 ,100, 1000 ,100000....
#define RTC_SELECTOR 0x70
#define RTC_RW 0x71
#define RTC_RA 0x8A
#define RTC_RB 0x8B


//below is the macro for the KB

//the standard scan code of the keyboard to ascii reference, we only complete part of them (including cahr and number)
//the translation is referenced to linux0.11:KBD_US, we do't consider the correcness or compability after '/'
uint8_t std_scancodetoascii[70]= {
        0,  27,
        '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 
        '\b','\t',                                                 
        'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 
        '\n', 0,                                         
        'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', 
        '`',   0,                                      
        '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',    //below is not sure/or not right maybe.
        0, '*', 0, ' ',  
        0,  'Z',  'X',   'C',   0,   0,   0,   0,   0,   0,
        0 
    };
volatile int if9pressed = 0;         //warning, this is just for cp1 testing, 9 means open test of the RTC handler. We do not add lock, because it will only use twice.


/*   keyboard_init
 *   Function: init the keyboard input
 *   Inputs: none
 *   Return Value: none
 */
void keyboard_init(void) {
    enable_irq(KEYBOARD_IRQ); 
}

/*
 * drush8: handler
 * we reference the linux 0.11 source code, to generate the code of us.
 * 
 * 
 */
void keyboard_handler(void){
    static int e0=0, e1=0;  //state despriptor, we now won't use it yet.
    //by default, we think e0 stands for 1 following 
    cli();//for safety...
    int scancode;
    if((inb(0x64)&1)==1){    //8042a protocol. That status bit means there is scan code available in 0x60 port.
      scancode = inb(0x60);//get the scancode.
      if(scancode != 0xE0 && scancode != 0xE1){//situation not be e0 or e1
          if(e0!=0) e0--;
          if(e1!=0) e1--; //now we doesn't deal with special key yet.
          if(e0==0 && e1==0){//the situation that we need to put out a char to the screen
          putc(std_scancodetoascii[scancode]);

          ////////
          if (std_scancodetoascii[scancode] == '9') {
            if(if9pressed == 0)if9pressed=1;
            else if9pressed =0;
          }
          if (std_scancodetoascii[scancode] == '8') if9pressed = -2; 
          if (std_scancodetoascii[scancode] == '7') if9pressed = -1;
          ////////warning: the code here is created only for cp1.
          }
        }

      if(scancode == 0xE0){
         e0+=1;
      }

      if(scancode == 0xE1){
          e1+=2;
      }                       //set the state machine properly.

      //below is the code for 8255A dealing specially. I am not sure if it is necessary
      //for our qemu machine. but for safety, we add it //drush8
      //char status = kbsget();
      //int i = 0; // set the for loop to wait for the device;
      //status |= 0x80; //set the bite to forbid the keyboard working.
      //for(i =0;i<5;i++){
      //}   //wait
      //kbssend(status); //let the keyboard to stop working
      //status &= 0x7F; //reset
      //for(i =0;i<3;i++){
      //}   //wait   
      //kbssend(status);
      //we are not use 8255a chip, but 8042a instead. so....
    }

    send_eoi(KEYBOARD_IRQ);
    sti();//enable the intrrupts as soon as possible.

}


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
void rtc_handler(void){   //we wait for at least RTC_TIMES intrrputs. **We consider it is a 1 core machine so decide this simple code.
                      //if it is a muti-core, we need to have  independent structures for every cpu.
    static volatile int times = 0;
    times ++;
    send_eoi(RTC_IRQ);
    if((times-1)==1){
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
