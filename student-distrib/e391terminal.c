#include "types.h"
#include "e391terminal.h"
#include "e391keyboard.h"
#include "lib.h"

//create by drush8

/*   terminal_read
 *   Function: syscall-like device driver 
 *   Inputs: none
 *   Return Value: none
 */
int32_t terminal_read(int32_t fd, void *buf, int32_t nbytes){
    if (buf == NULL) return 1;  //case: invalid input
    int flags,i,num=0;
    uint8_t line_num = 0;
    int c=0;
    cli_and_save(flags);    // now, we open the critical section.

    kbstatus.terminalreading = 1;

    kb_setoffset(get_screen_x());  //before read, we set the base pos x of the screen(for the '\b' functionality.)
    //
    //while(ln==0){
    //    sti();
    //    for(i =0;i<5;i++);      //spend some time wait for the \n input... . drush8'sflag: can be sleeplock in future.
    //    cli();
    //    ln = kbbuf.linenum; //volatile in struct seems useless...
    //}
    //above realization is not so  pleased, fix it. drush8:11.6


    //printf("finish\n");
    //now there is at least one line in kb_buf
    while(num<nbytes){

        while(line_num == 0){
            sti();
            for(i =0;i<5;i++);      //spend some time wait for the \n input... . drush8'sflag: can be sleeplock in future.
            cli();
            line_num = kbbuf.linenum;
        }       //drush8: check is there is a complete line...

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

/*   terminal_write
 *   Function: syscall-like device driver 
 *   Inputs: none
 *   Return Value: none
 */
int32_t terminal_write(int32_t fd,const void* buf, int32_t nbytes){
    //should always success...
    //for \0 we terminal_write donnot print it to the screen
    //printf("writebegin\n");
    int i,total = 0;
    if (buf == NULL) return 1;  //case: invalid input

    for(i=0;i<nbytes;i++){
        //if(*((uint8_t *)buf)=='\0') break;       //meet null,we stop.
        if(*((uint8_t *)buf)!='\b'){ 
            if(*((uint8_t *)buf)!='\0') putc(*((uint8_t *)buf));
        }
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



/*   
 *   
 *   below two are special in func meaning.
 *   
 */




/*   terminal_open
 *   Function: syscall-like device driver 
 *   Inputs: none
 *   Return Value: none
 */
int32_t terminal_open(const uint8_t* filename){
    kb_init();
    clear();
    //just initialize the whole buffer in keyboard and clean the screen
    return 0;
}


/*   terminal_close
 *   Function: syscall-like device driver 
 *   Inputs: none
 *   Return Value: none
 */
int32_t terminal_close(int32_t fd){
    kb_init();
    clear();
    //we shouldn't close it! even if we do so, we just regard it as a reinitializing act.
    return 0;
}
