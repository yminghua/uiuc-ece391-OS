#include "types.h"
#include "e391terminal.h"
#include "e391keyboard.h"
#include "lib.h"

int32_t terminal_read(void *buf, int32_t nbytes){
    if (buf == NULL) return 1;  //case: invalid input
    int flags,i,num=0;
    uint8_t ln = 0;
    int c=0;
    cli_and_save(flags);    // now, we open the critical section.

    kbstatus.terminalreading = 1;
    while(ln==0){
        sti();
        for(i =0;i<5;i++);      //spend some time wait for the \n input... . drush8'sflag: can be sleeplock in future.
        cli();
        ln = kbbuf.linenum; //volatile in struct seems useless...
    }
    //printf("finish\n");
    //now there is at least one line in kb_buf
    while(num<nbytes){
        c = kbbufconsume();
        if(c == '\n'){
            kbstatus.terminalreading = 0;
            restore_flags(flags);
            *((char *)buf++) = '\n';

            if(num+1<nbytes) *((char *)buf) = '\0'; //if there is spare room, we kindly add a \0 for it.
            return num+1;
        }
        num++;
        *((char *)buf) = (char)c;
        buf++;
    }
    kbstatus.terminalreading = 0;
    restore_flags(flags);
    //now num is equal to the nbytes..
    //BUG:if the buffer is full, we do not add a \0 anymore
    //*((char *)buf) = '\0';
    return num;
}

int32_t terminal_write(const void* buf, int32_t nbytes){
    //should always success...
    //printf("writebegin\n");
    int i,total = 0;
    if (buf == NULL) return 1;  //case: invalid input

    for(i=0;i<nbytes;i++){
        //if(*((uint8_t *)buf)=='\0') break;       //meet null,we stop.
        if(*((uint8_t *)buf)!='\b') putc(*((uint8_t *)buf));
        else {
            clearwithcursor(1,0);                //most situation(even in all situation), this divagence won't happen
            if(total > 0 ) total--;
        }
        buf++;
        total++;
    }
    //printf("writeend\n");
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
