#include "lib.h"
#include "e391device.h"
#include "e391synchronize.h"
#include "i8259.h"
#include "intrexcenum.h"

#define RTC_TIMES 1>>3        //youcan choose the 1, 10 ,100, 1000 ,100000....

char std_scancodetoascii[70];//the standard scan code of the keyboard to ascii reference, we only complete part of them (including cahr and number)


/*
 * drush8: handler
 * we reference the linux 0.11 source code, to generate the code of us.
 * 
 * 
 */
void keyboard_handler(){
    static int e0=0, e1=0;  //state despriptor, we now won't use it yet.
    //by default, we think e0 stands for 1 following 
    char scancode;
    scancode = kbcget();//get the scancode.
    if(scancode != 0xE0 && scancode != 0xE1){//situation not be e0 or e1
        if(e0!=0) e0--;
        if(e1!=0) e1--; //now we doesn't deal with special key yet.
        if(e0==0 && e1==0){//the situation that we need to put out a char to the screen
        putc(std_scancodetoascii[scancode]);}
    }

    if(scancode == 0xE0){
        e0+=1;
    }

    if(scancode == 0xE1){
        e1+=2;
    }                       //set the state machine properly.

    //below is the code for 8255A dealing specially. I am not sure if it is necessary
    //for our qemu machine. but for safety, we add it //drush8
    char status = kbsget();
    int i = 0; // set the for loop to wait for the device;
    status |= 0x80; //set the bite to forbid the keyboard working.
    for(i =0;i<5;i++){
    }   //wait
    kbssend(status); //let the keyboard to stop working
    status &= 0x7F; //reset
    for(i =0;i<3;i++){
    }   //wait   
    kbssend(status);
    send_eoi(KEYBOARD_IRQ);
    sti();//enable the intrrupts as soon as possible.

}


void rtc_handler(){   //we wait for at least RTC_TIMES intrrputs. **We consider it is a 1 core machine so decide this simple code.
                      //if it is a muti-core, we need to have  independent structures for every cpu.
    static volatile times = 0;
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
    sti();//enable the intrrupts as soon as possible.
    
    return;
}
//create by drush8


//the translation is referenced to linux0.11:KBD_US, we do't consider the correcness or compability after '/'
char std_scancodetoascii[70]= {
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