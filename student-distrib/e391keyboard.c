#include "lib.h"
#include "e391synchronize.h"
#include "i8259.h"
#include "intrexcenum.h"
#include "types.h"
#include "e391keyboard.h"
//#include "e391device.h"   used in cp1, now has been abandoned
//MP3.2code:
//below is the struct for the KB

#define POS(x)  ((x)%128)
#define DIFF(x,y) (x+128-y)%128
/*****status of the key board, used for the state divides*****/
kbstatus_t kbstatus={0,0,0,0,0,0,0,0,0,0};
kb_buf_t kbbuf;

//the standard scan code of the keyboard to ascii reference, we only complete part of them (including cahr and number)
//the translation is referenced to linux0.11:KBD_US, we do't consider the correcness or compability after '/'
uint8_t std_scancodetoascii[89]= {
        0,  27,                                                    //27 is escaped
        '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 
        '\b','\t',                                                 //backspace & tab
        'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 
        '\n', 0,                                                   //1C, 1D:left control
        'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', 
        '`',   0,                                                  //2A:left shift
        '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',    //below is not sure/or not right maybe.
        0, '*', 0, ' ',  
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,            //3A-49     :3A is th capslock
        '-',0,0,0,'+',                               //region of keypad number, reserve. no corresponding ascii.
        0,0,0,0,0,0,0,                               //4F-55    4F-53 is keypad region
        0,0,0                                        //56,57,58
    };
volatile int if9pressed = 0; //Warning, this is just for cp1 testing, 9 means open test of the RTC handler. We do not add lock, because it will only use twice.

uint8_t shift_scancodetoascii[89]= {
        0,  27,
        '!', '@', '#', '$', '%', '^', '&', '*','(', ')', '_', '+',
        '\b', '\t',                 //backspace & tab
        'Q', 'W', 'E', 'R','T', 'Y', 'U', 'I', 'O', 'P', '{', '}',
        '\n', 0,                  //left control
        'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"',
        '~',   0,                   //left shift
        '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 
          0,  '*', 0, ' ',  //36 right shift. 38 left Alt  39 space
        //below is the  same as std mode. (maybe)
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,            //3A-49
        '-',0,0,0,'+',                               //region of keypad number, reserve. no corresponding ascii.
        0,0,0,0,0,0,0,                               //4F-55    4F-53 is keypad region
        0,0,0                                        //56,57,58
    };

uint8_t caplock_scancodetoascii[89]= {
        0,  27,                                                    //27 is escaped
        '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 
        '\b','\t',                                                 //backspace & tab
        'Q', 'W', 'E', 'R','T', 'Y', 'U', 'I', 'O', 'P', '[', ']', 
        '\n', 0,                                                   //1C, 1D:left control
        'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', 
        '`',   0,                                                  //2A:left shift
        '\\', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/',    //below is not sure/or not right maybe.
        0, '*', 0, ' ',  
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,            //3A-49     :3A is th capslock
        '-',0,0,0,'+',                               //region of keypad number, reserve. no corresponding ascii.
        0,0,0,0,0,0,0,                               //4F-55    4F-53 is keypad region
        0,0,0                                        //56,57,58
    };


//drush8: reference to the linux 0.11 USKB, seems... there is no need additional print for the alt?
uint8_t alt_scancodetoascii[89]= {   //0x59
        0,  0,                                                    //27 is escaped
        '\0', '@', '\0', '$', '\0', '\0', '{', '[', ']', '}', '\\', '\0', 
        0,0,                                                 //backspace & tab
        0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, '~', 
        '\n', 0,                                                   //1C, 1D:left control
        0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 
        0, 0,                                                  //2A:left shift
        0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0,    //below is not sure/or not right maybe.
        0, 0, 0, 0,  
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,            //3A-49     :3A is th capslock
        0,0,0,0,0,                               //region of keypad number, reserve. no corresponding ascii.
        0,0,0,0,0,0,0,                               //4F-55    4F-53 is keypad region
        0,0,0                                        //56,57,58
    };
/**
 * 
 * 
 * warning: the code here all used cli/sti to mantain the correctness of the data sturcture considering the single core
 * for improvement, we will use the spinlock at the suitable position to improve the performance in the future.
 * 2022.10.22
 *   drush8'sflag
 */


/**
 * 
 *          B U F F E R          O P E R A T I O N
 *          (with one Status init)
 */

//init the kbstatus structure.
//return 0 always
uint32_t kb_Sinit(){
  kbstatus.altpressed = 0;
  kbstatus.caplockon = 0;
  kbstatus.flag = 0;
  kbstatus.shiftpressed = 0;
  kbstatus.controlpressed = 0;
  kbstatus.terminalreading = 0;
  kbstatus.multiscanmode = 0;
  kbstatus.echo = 1; //echo to screen by default.
  return 0;
}  
        
//init the kbbuffer structure.
//return 0 always
uint32_t kb_Binit(){
  kbbuf.biteBP = 0;
  kbbuf.biteEP = 0;
  kbbuf.bitenum  =0;

  kbbuf.lineBP = 0;
  kbbuf.lineEP = 0;
  kbbuf.linenum = 0;      //line is combined with 
  return 0;
}


//combination of above two func
uint32_t kb_init(){
  kb_Sinit();kb_Binit();
  kbstatus.setoffset = 0;
  kbstatus.kbalready = 1;
  return 0;
}

uint32_t kb_setoffset(int n){
  kbstatus.setoffset = n;
  return 0;
}

int lengthbetween(int index){
  int num=0;
  while(index != kbbuf.biteBP && kbbuf.bitebuf[DIFF(index,1)]!='\n'){
    index = DIFF(index,1);
    if(kbbuf.bitebuf[index] != '\0'){
      if(kbbuf.bitebuf[index] == '\t') num+=4;
      else num+=1;
    } 
  }
  return num;
}

//pop out the highest one char in the queue.(used by the backspace)
uint32_t kbbufpop(){
  if(kbbuf.bitenum==0) return 1;   //no char anymore.
  kbbuf.biteEP = POS(kbbuf.biteEP-1);
  kbbuf.bitenum--;
  uint8_t dchar = kbbuf.bitebuf[POS(kbbuf.biteEP)];
  if (dchar == '\n'){
    kbbuf.lineEP = POS(kbbuf.lineEP-1);
    kbbuf.linenum--;
  }//if the delete char is \n, we need to fix the lineposition correctly.
  if (kbstatus.echo == 0) return 0; //if the echo is closed, don't care about the screen.

  if(dchar == '\n'){
    int c;
    //if (kbbuf.linenum!=0) c = DIFF(kbbuf.biteEP, kbbuf.linelocbuf[kbbuf.lineEP-1])-1;
    //else c = DIFF(kbbuf.biteEP, kbbuf.biteBP)+kbstatus.setoffset;   //here for example, offset is num char of shell's prompt 
    //it is very sad that above method is useless on special situation...
    c = lengthbetween(kbbuf.biteEP);
    clearwithcursor(0,c);
  }
  if(dchar == '\t'){
    clearwithcursor(4,0);
  }
  if(dchar != '\t' && dchar != '\n' && dchar != '\0'){      //safety check for the \0
    clearwithcursor(1,0);
  }

  return 0;

}

//push one char into the buffer. 
uint32_t kbbufpush(uint8_t bite){
  if(kbbuf.bitenum >= 127 && bite !='\n') return 1; //buffer is almost full, we only accept '\n' for nearly full buffer.
  if(bite == '\n'){
    kbbuf.linelocbuf[kbbuf.lineEP] = kbbuf.biteEP;
    kbbuf.lineEP = POS(kbbuf.lineEP+1);
    kbbuf.linenum++;
  }
  kbbuf.bitebuf[kbbuf.biteEP] = bite;
  kbbuf.biteEP = POS(kbbuf.biteEP+1);
  kbbuf.bitenum++;
  if (kbstatus.echo == 0) return 0; //if the echo is closed, don't care about the screen.
  if(bite != '\0' ) putc(bite); //safety check: drush8
  //well echo is 1 , we need to put it on the screen
  return 0;
}

//consume by terminal fetching. the direction is from the head to the tail. the consume num is 1 default in this func.
uint32_t kbbufconsume(){    
  if(kbbuf.bitenum==0) return 1; //no more bites
  char c = kbbuf.bitebuf[kbbuf.biteBP];
  if(c == '\n'){
    kbbuf.lineBP = POS(kbbuf.lineBP+1);
    kbbuf.linenum--;
  }
  kbbuf.bitenum--;
  kbbuf.biteBP=POS(kbbuf.biteBP+1);
  return c;
}

//below three func should be called in critical section. thay are used to protecting the structure.
//can be inplemented by:
//1. just spin_lock
//2. well, it is just a 1-core machine. We just make sure that terminal fetch with if =0.
uint32_t kbwaituntilfree(){
  //won't used, until we try to do the improvement..
  return 0;
}
uint32_t kbsetfree(){
  kbstatus.flag = 0;
  return 0;
}
uint32_t kbsetbusy(){
  kbstatus.flag = 1;
  return 0;
}


//below two will used by terminal reading. related to the crtl+l functionality.
uint32_t kbsetreading(){
  kbstatus.terminalreading = 1;
    return 0;
}

uint32_t kbunsetreading(){
  kbstatus.terminalreading = 0;
    return 0;
}


//below three is assistance func. they will be  all used by the inter_handler.

//ctrl+l functionality.
void ctrllfunc(){
  if(kbstatus.terminalreading == 0){
    kb_init();
  }
  clear();
}

//do scancode to ascii code translation
char asciitranslate(int c){
  //we give out the priority: first is the alt, then shift, finall caplock
  if(kbstatus.altpressed!=0) return alt_scancodetoascii[c];
  if(kbstatus.shiftpressed!=0) return shift_scancodetoascii[c];
  if(kbstatus.caplockon!=0) return caplock_scancodetoascii[c];
  return std_scancodetoascii[c];
}

//this func only used for testing in cp3.1 & 3.2 now...
//set the is9pressedset according to the keyvoard pressing...
void is9pressedset(int b){
            ////////
  if (b == '9') {                        //9 stands for the RTB mode open
    if(if9pressed == 0)if9pressed=1; 
    else if9pressed =0;
  }

  if (b == '8') {
    if(if9pressed == 0)if9pressed=-2; 
    else if9pressed =0;}
  if (b == '7') if9pressed = -1;         //7 opens the final PF test.
    ////////warning: the code here is created only for cp1.
}




/**
 *                          DRUSH8
 *          K E Y B O A R D         O P E R A T I O N
 * 
 */



/*   keyboard_init
 *   Function: init the keyboard input
 *   Inputs: none
 *   Return Value: none
 */
void keyboard_init(void) {
    kb_init();
    enable_irq(KEYBOARD_IRQ); 
}


/*   keyboard_handler
 *   Function: the KBUS handler
 *   Inputs: none
 *   Return Value: none
 *   (we reference the linux 0.11 source code, to generate the code of us.)
 */
void keyboard_handler(void){
  //warning: drush'sflag. keyboard intr should not be interrupted by others. so, we don't protect the kb structs.
    cli();//for safety...
    //if(kbstatus.kbalready == 0) return; //by default this situation won't happen.
    int scancode;
    char asciicode;
    if((inb(0x64)&1)==1){    //8042a protocol. That status bit means there is scan code available in 0x60 port.
      scancode = inb(0x60);//get the scancode.
      if(kbstatus.multiscanmode==0){    //well, there is no multiscancode comes here
        if(scancode == 0xE0 || scancode == 0xE1){
          kbstatus.multiscanmode = scancode + 1 -0xE0;
          return;   //set the mode then return
        }
        if(scancode<0x80){// well, that is the key being pressed now.
          switch(scancode) {
          case CAPSLOCK_P:
            kbstatus.caplockon = 1-kbstatus.caplockon;
            break;
          case LCTRL_P: 
            kbstatus.controlpressed++;
            break;
          case LSHIFT_P: 
            kbstatus.shiftpressed++;
            break;
          case LALT_P: 
            kbstatus.altpressed++;
            break;
          case RSHIFT_P: 
            kbstatus.shiftpressed++;
            break;
          case BACKSPACE_P:
            if(kbstatus.kbalready == 1) kbbufpop();
            break;
          case L_P:   //is l pressed?
            if(kbstatus.controlpressed>0) {ctrllfunc();break;}    //here we clean the screen, but not the buf of keyboard.
          default:
            asciicode = asciitranslate(scancode);
            if(asciicode == '\0') break;                          //useless or the situation we dont consider, as if it is untyped by the kb.
            if(kbstatus.kbalready ==1) kbbufpush(asciicode);
            is9pressedset(asciicode); //only used for the testing argument setting.
          }
        }
        else{     //the situation of the releaseed key press.
          scancode -=0x80;
                    switch(scancode) {
          case LCTRL_P: 
            if(kbstatus.controlpressed>0) kbstatus.controlpressed--;
            break;
          case LSHIFT_P: 
            if(kbstatus.shiftpressed>0) kbstatus.shiftpressed--;
            break;
          case LALT_P: 
            if(kbstatus.altpressed>0) kbstatus.altpressed--;
            break;
          case RSHIFT_P: 
            if(kbstatus.shiftpressed>0) kbstatus.shiftpressed--;
            break;
          default:
            break;
          }
        }
        //putc(std_scancodetoascii[scancode]);
      }

      else{   //now is in the multiscancode situation
          kbstatus.multiscanmode--;
          //if(scancode == LCTRL_P) kbstatus.controlpressed ++;  //well this means the right control is pressed now.
          //if(scancode == LCTRL_P+0x80 && kbstatus.controlpressed>0) kbstatus.controlpressed --;//it is released!!
      }
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
 */

//create by drush8
