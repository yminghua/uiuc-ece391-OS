#include "types.h"
#include "e391terminal.h"
#include "e391keyboard.h"
#include "lib.h"

int32_t terminal_read(void *buf, int32_t nbytes){
    if (buf == NULL) return 1;  //case: invalid input
    int flags,i,num=0;
    int c=0;
    cli_and_save(flags);    // now, we open the critical section.
    kbstatus.terminalreading = 1;
    while(kbbuf.linenum==0){
        sti();
        for(i =0;i<5;i++);      //spend some time wait for the \n input... . drush8'sflag: can be sleeplock in future.
        cli();
    }
    //now there is at least one line in kb_buf
    while(num<nbytes){
        c = kbbufconsume();
        if(c == '\n'){
            *((char *)buf++) = '\n';
            *((char *)buf) = '\0';
            return num++;
        }
        num++;
        *((char *)buf) = (char)c;
        (char *)buf++;
    }
    //now num is equal to the nbytes..
    *((char *)buf) = '\0';
    return num;
}

int32_t terminal_write(const void* buf, int32_t nbytes){
    //should always success...
    int i,j;
    if (buf == NULL) return 1;  //case: invalid input

    for(i=0;i<nbytes;i++){
        if(*((uint8_t *)buf)=='\0') break;       //meet null,we stop.
        if(*((uint8_t *)buf)!='\b') putc(*((uint8_t *)buf));
        else {
            clearwithcursor(1,0);                //most situation(even in all situation), this divagence won't happen
            i-=2;
        }
    }
    return i;               //always write 'nbytes' successfully
}
int32_t terminal_open(void){
    kb_init();
    clear();
    //just initialize the whole buffer in keyboard and clean the screen
    return 0;
}



int32_t terminal_close(void){
    kb_init();
    clear();
    //we shouldn't close it! even if we do so, we just regard it as a reinitializing act.
    return 0;
}