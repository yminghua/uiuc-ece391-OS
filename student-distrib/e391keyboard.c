#include "lib.h"
#include "e391synchronize.h"
#include "i8259.h"
#include "intrexcenum.h"
#include "types.h"
#include "e391keyboard.h"
#include "PCB.h"
//#include "e391device.h"   used in cp1, now has been abandoned

//create by drush8

//MP3.2code:
//below is the struct for the KB
#define POS(x)  ((x)%128)
#define DIFF(x,y) (x+128-y)%128
/*****status of the key board, used for the state divides*****/
kbstatus_t kbstatus={0,0,0,0,0,0,0,0,0,0};
kb_buf_t kbbuf;

//mp3.5 additional:
//we need that we will use the 
kbstatus_t kbstatus_for_multiterminal[NUMTERMINAL];
kb_buf_t kbbuf_for_multiterminal[NUMTERMINAL];
kbstatus_t *kbstatusp;
kb_buf_t *kbbufp;

int nowterminalno; //it will be just used in this file scope. 

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
volatile int if9pressed = -5; //Warning, this is just for cp1 testing, 9 means open test of the RTC handler. We do not add lock, because it will only use twice.
                              //set -5 means we won't use it anymore..
                              
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
  kbstatusp->altpressed = 0;
  kbstatusp->caplockon = 0;
  kbstatusp->flag = 0;
  kbstatusp->shiftpressed = 0;
  kbstatusp->controlpressed = 0;
  kbstatusp->terminalreading = 0;
  kbstatusp->multiscanmode = 0;
  kbstatusp->echo = 1; //echo to screen by default.
  return 0;
}  
        
//init the kbbuffer structure.
//return 0 always
uint32_t kb_Binit(){
  kbbufp->biteBP = 0;
  kbbufp->biteEP = 0;
  kbbufp->bitenum  =0;

  kbbufp->lineBP = 0;
  kbbufp->lineEP = 0;
  kbbufp->linenum = 0;      //line is combined with 
  return 0;
}


//combination of above two func
uint32_t kb_init(){
  kb_Sinit();kb_Binit();
  kbstatusp->setoffset = 0;
  kbstatusp->kbalready = 1;
  return 0;
}

//setting the offset.
//(may used in cp3, make sure backspace won't clean the prompt of the shell)
uint32_t kb_setoffset(int n){
  kbstatusp->setoffset = n;
  return 0;
}

//assistance func: return the last line length in th ebuf.
int lengthbetween(int index){
  int num=0;
  while(index != kbbufp->biteBP && kbbufp->bitebuf[DIFF(index,1)]!='\n'){
    index = DIFF(index,1);
    if(kbbufp->bitebuf[index] != '\0'){
      if(kbbufp->bitebuf[index] == '\t') num+=4;
      else num+=1;
    } 
  }
  return num;
}

//pop out the highest one char in the queue.(used by the backspace)
uint32_t kbbufpop(){
  if(kbbufp->bitenum==0) return 1;   //no char anymore.
  kbbufp->biteEP = POS(kbbufp->biteEP-1);
  kbbufp->bitenum--;
  uint8_t dchar = kbbufp->bitebuf[POS(kbbufp->biteEP)];
  if (dchar == '\n'){
    kbbufp->lineEP = POS(kbbufp->lineEP-1);
    kbbufp->linenum--;
  }//if the delete char is \n, we need to fix the lineposition correctly.
  if (kbstatusp->echo == 0) return 0; //if the echo is closed, don't care about the screen.

  if(dchar == '\n'){
    int c;
    //if (kbbufp->linenum!=0) c = DIFF(kbbufp->biteEP, kbbufp->linelocbuf[kbbufp->lineEP-1])-1;
    //else c = DIFF(kbbufp->biteEP, kbbufp->biteBP)+kbstatup->setoffset;   //here for example, offset is num char of shell's prompt 
    //it is very sad that above method is useless on special situation...
    c = lengthbetween(kbbufp->biteEP);
    if(kbbufp->linenum==0) c+=kbstatusp->setoffset; //if it is the final line,we should consider the prompt..
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
  //if(kbbufp->bitenum == 127 && bite !='\n') return 1; //buffer is almost full, we only accept '\n' for nearly full buffer.
  //fixed by drush8: we do not force that the final one must be the \n.
  if(kbbufp->bitenum == 128) return 2; //totally full... return 2
  if(bite == '\n'){
    kbbufp->linelocbuf[kbbufp->lineEP] = kbbufp->biteEP;
    kbbufp->lineEP = POS(kbbufp->lineEP+1);
    kbbufp->linenum++;
  }
  kbbufp->bitebuf[kbbufp->biteEP] = bite;
  kbbufp->biteEP = POS(kbbufp->biteEP+1);
  kbbufp->bitenum++;
  if (kbstatusp->echo == 0) return 0; //if the echo is closed, don't care about the screen.
  if(bite != '\0' ) putc(bite); //safety check: drush8
  //well echo is 1 , we need to put it on the screen
  return 0;
}

//consume by terminal fetching. the direction is from the head to the tail. the consume num is 1 default in this func.
uint32_t kbbufconsume(){    
  if(kbbufp->bitenum==0) return 1; //no more bites
  char c = kbbufp->bitebuf[kbbufp->biteBP];
  if(c == '\n'){
    kbbufp->lineBP = POS(kbbufp->lineBP+1);
    kbbufp->linenum--;
  }
  kbbufp->bitenum--;
  kbbufp->biteBP=POS(kbbufp->biteBP+1);
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
  kbstatusp->flag = 0;
  return 0;
}
uint32_t kbsetbusy(){
  kbstatusp->flag = 1;
  return 0;
}


//below two will used by terminal reading. related to the crtl+l functionality.
uint32_t kbsetreading(){
  kbstatusp->terminalreading = 1;
    return 0;
}

uint32_t kbunsetreading(){
  kbstatusp->terminalreading = 0;
    return 0;
}


//below three is assistance func. they will be  all used by the inter_handler.


void savexyposition(){
  kbstatusp->cx = get_screen_x();
  kbstatusp->cy = get_screen_y();
}

uint32_t kb_status_ptr_set(int i){
  if(i>=NUMTERMINAL) return 1;
  kbstatusp = &kbstatus_for_multiterminal[i];
  kbbufp = &kbbuf_for_multiterminal[i];
  return 0;
}

void kbstatusswitch(int terno){ 
  //terno should be from 0 to max-1
  //terno is the no. of the kbstatus you want to switch:
  kb_status_ptr_set(terno);
  set_screen_xy(kbstatusp->cx,kbstatusp->cy);
  set_video_mem(terno+1);
}


//ctrl+l functionality.
void ctrllfunc(){
  if(kbstatusp->terminalreading == 0){
    kb_init();
  }
  clear();
}

//ctrl+F1,2,3 functionality.
void altfnfunc(int no){
  //for safety, we store position again.
  savexyposition();
  int now, previous;
  previous = nowterminalno;
  now = no;
  nowterminalno = now;    //we will use now&previous in the following code, and here we give nowteminalno right value.

  //0.0: now's kbstatus have the real video addr, but previous has only its back buffer one...
  kb_status_ptr_set(now);
  //set_screen_xy(kbstatusp->cx,kbstatusp->cy);//in lib.c we switch the position...
  kbstatusp->cur_videoaddr = BVIDEO(0);
  kb_status_ptr_set(previous);
  kbstatusp->cur_videoaddr = BVIDEO(previous+1);

  //0. 

  //then 1. save the real video stuff to the previous' back video space
  memmove(BVIDEO(previous+1),BVIDEO(0),4*1024);  //here we have to know that: 0 real, 1,2,3 is the coresponding addr. so need to add 1.
  //2. copy the now's back video space stuff to the real video mapping.
  memmove(BVIDEO(0),BVIDEO(now+1),4*1024);
  //3. drush8'sflag: call the scheduler to switch to the corresponding terminal... now we don't realize it

  return;
}

//do scancode to ascii code translation
char asciitranslate(int c){
  //we give out the priority: first is the alt, then shift, finall caplock
  if(kbstatusp->altpressed!=0) return alt_scancodetoascii[c];
  if(kbstatusp->shiftpressed!=0) return shift_scancodetoascii[c];
  if(kbstatusp->caplockon!=0) return caplock_scancodetoascii[c];
  return std_scancodetoascii[c];
}

//this func only used for testing in cp3.1 & 3.2 now...
//set the is9pressedset according to the keyvoard pressing...
//9,8,7 and 0,-1,-2 are used to control the tests. free to use them to build your continuous tests!!! drush8
void is9pressedset(int b){
            ////////
  return; //we will abandon this test usage func..
  if (b == '9') {                        
    if(if9pressed == 0)if9pressed=1; 
    else if9pressed =0;
  }

  if (b == '8') {
    if(if9pressed == 0)if9pressed=-2; 
    else if9pressed =0;}
  if (b == '7') if9pressed = -1;         
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
    int i;
    for(i=0;i<NUMTERMINAL;i++){
      kb_status_ptr_set(i);
      kb_init();
      kbstatusp->cur_videoaddr = BVIDEO(i+1);
    }
    kb_status_ptr_set(0);
    nowterminalno = 0;
    kbstatusp->cur_videoaddr = BVIDEO(0);
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
    send_eoi(KEYBOARD_IRQ);
    //if(kbstatusp->kbalready == 0) return; //by default this situation won't happen.
    int scancode;
    char asciicode;
    if((inb(0x64)&1)==1){    //8042a protocol. That status bit means there is scan code available in 0x60 port.
      scancode = inb(0x60);//get the scancode.
      if(kbstatusp->multiscanmode==0){    //well, there is no multiscancode comes here
        if(scancode == 0xE0 || scancode == 0xE1){
          kbstatusp->multiscanmode = scancode + 1 -0xE0;
          return;   //set the mode then return
        }
        if(scancode<0x80){// well, that is the key being pressed now.
          switch(scancode) {
          case CAPSLOCK_P:
            kbstatusp->caplockon = 1-kbstatusp->caplockon;
            break;
          case LCTRL_P: 
            kbstatusp->controlpressed++;
            break;
          case LSHIFT_P: 
            kbstatusp->shiftpressed++;
            break;
          case LALT_P: 
            kbstatusp->altpressed++;
            break;
          case RSHIFT_P: 
            kbstatusp->shiftpressed++;
            break;
          case BACKSPACE_P:
            if(kbstatusp->kbalready == 1) kbbufpop();
            break;
          case F1_P:
          case F2_P:
          case F3_P:
            if(kbstatusp->altpressed>0) {altfnfunc(scancode-F1_P);break;}
          case L_P:   //is l pressed?
            if(kbstatusp->controlpressed>0) {ctrllfunc();break;}    //here we clean the screen, but not the buf of keyboard.
          default:
            asciicode = asciitranslate(scancode);
            if(asciicode == '\0') break;                          //useless or the situation we dont consider, as if it is untyped by the kb.
            if(kbstatusp->kbalready ==1) kbbufpush(asciicode);
            is9pressedset(asciicode); //only used for the testing argument setting.
          }
        }
        else{     //the situation of the releaseed key press.
          scancode -=0x80;
                    switch(scancode) {
          case LCTRL_P: 
            if(kbstatusp->controlpressed>0) kbstatusp->controlpressed--;
            break;
          case LSHIFT_P: 
            if(kbstatusp->shiftpressed>0) kbstatusp->shiftpressed--;
            break;
          case LALT_P: 
            if(kbstatusp->altpressed>0) kbstatusp->altpressed--;
            break;
          case RSHIFT_P: 
            if(kbstatusp->shiftpressed>0) kbstatusp->shiftpressed--;
            break;
          default:
            break;
          }
        }
        //putc(std_scancodetoascii[scancode]);
      }

      else{   //now is in the multiscancode situation
          kbstatusp->multiscanmode--;
          //if(scancode == LCTRL_P) kbstatusp->controlpressed ++;  //well this means the right control is pressed now.
          //if(scancode == LCTRL_P+0x80 && kbstatusp->controlpressed>0) kbstatusp->controlpressed --;//it is released!!
      }
    }    

//    send_eoi(KEYBOARD_IRQ);//move up to the start
    //mp3.5: we need to reset the x,y video position everytime there is a key pressed.
    savexyposition();
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
