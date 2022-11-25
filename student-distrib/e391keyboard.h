#if !defined(E391KB_H)
#define E391KB_H
#include "types.h"
//create by drush8

//predefined macro constant

#define NUMTERMINAL 3

#define KBBUSY 1
#define KBFREE 0
//all is the pressed scan code.
#define L_P 0x26
#define LSHIFT_P 0x2A
#define RSHIFT_P 0x36
#define LCTRL_P 0x1D
#define LALT_P 0x38
#define CAPSLOCK_P 0x3A
#define BACKSPACE_P 0x0E

#define F1_P 0x3B
#define F2_P 0x3C
#define F3_P 0x3D

#define TWOSCAN 0xE0
#define THREESCAN 0xE1  //wont used in our kb.
//defined structure
typedef struct kbstatus {
    uint8_t shiftpressed;   //at most 2: 2 means that two shift all pressed
    uint8_t caplockon;      //means caplock mode is on. thus we ignored the release signal.
    uint8_t altpressed;     //same as above.
    uint8_t controlpressed; //is control pressed?
    uint8_t echo;           // the mode that shows what be typed on the screen
    uint8_t kbalready;      //stands for the buffer and kbstatus is ready now.
    uint8_t terminalreading; //now terminal is reading the buffer.
    uint8_t multiscanmode;  //mark as multiscancode will come.... begein with E0 or E1.
    int8_t setoffset;      //for the terminal to use:: used in '\b' situation
    //mp3.5 adding: drush8: for convience, we save the cursor position in this kbstatus(though it is not the property of kb)
    uint8_t cx;              //corresponding cursor position.        
    uint8_t cy;              
    uint32_t cur_videoaddr; // the corresponding video addr.    should be 0 or the corresponding no.

    volatile uint8_t flag;           //means if this structure stable.1 stands busy. Will be changed to locks in the future for the multi_core.
} kbstatus_t;

extern volatile int if9pressed;

//warning:: kb_buf should be pretected by the kbstatus
/*****structure, which is also needed for the terminal*****/
typedef struct kb_buf {
    uint8_t bitebuf[128];             
    uint8_t bitenum;
    uint8_t biteBP;
    uint8_t biteEP;                   //rolling buffer, has the begin pointer and the end pointer
    uint8_t linelocbuf[128];
    volatile uint8_t linenum;
    uint8_t lineBP;
    uint8_t lineEP;
} kb_buf_t;

//         |       |
//         V       V
//    ...  vvvvvvvviiiiii   ... 
//shows that ele pointed py the head pointer is valid, but the one by tail pointer is not.

//extern data.. will be used by the terminal||
extern kbstatus_t kbstatus;
extern kb_buf_t kbbuf;

//mp3.5 fixed to the multi_terminal mode.
extern kbstatus_t kbstatus_for_multiterminal[NUMTERMINAL];
extern kb_buf_t kbbuf_for_multiterminal[NUMTERMINAL];
extern kbstatus_t *kbstatusp;
extern kb_buf_t *kbbufp;


extern uint32_t kb_init();
extern uint32_t kb_Sinit();             //status init             
extern uint32_t kb_Binit();             //init Buf.
extern uint32_t kbbufpop();
extern uint32_t kbbufconsume();         //will be used by the terminal..
extern uint32_t kbbufpush(uint8_t bite);
extern uint32_t kbwaitfree();
extern uint32_t kbsetfree();
extern uint32_t kbsetbusy();
extern uint32_t kb_setoffset(int n);

void keyboard_init();
void keyboard_handler();
uint32_t kb_status_ptr_set(int i);

extern volatile int if9pressed; //drush8: used for testing in cp3.1&3.2
/*******inline*********/



#endif 
