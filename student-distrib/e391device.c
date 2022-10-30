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

/*   keyboard_handler
 *   Function: the KBUS handler
 *   Inputs: none
 *   Return Value: none
 */
void keyboard_handler(void){
    static int e0=0, e1=0;  //state despriptor, we now won't use it yet.
    //by default, we think e0 stands for 1 following 
    cli();//for safety...
    int scancode;
    if((inb(0x64)&1)==1){    //8042a protocol. That status bit means there is scan code available in 0x60 port.
      scancode = inb(0x60);//get the scancode.
      if(scancode != 0xE0 && scancode != 0xE1){//situation not be e0 or e1. e0 and e1 stands for the different Scan coede set
          if(e0!=0) e0--;
          if(e1!=0) e1--; //now we doesn't deal with special key yet.
          if(e0==0 && e1==0){//the situation that we need to put out a char to the screen
          putc(std_scancodetoascii[scancode]);

          ////////
          if (std_scancodetoascii[scancode] == '9') {                        //9 stands for the RTB mode open
            if(if9pressed == 0)if9pressed=1; 
            else if9pressed =0;
          }
          if (std_scancodetoascii[scancode] == '8') if9pressed = -2;         //8 stands for the test phase going
          if (std_scancodetoascii[scancode] == '7') if9pressed = -1;         //7 opens the final PF test.
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






//create by drush8
