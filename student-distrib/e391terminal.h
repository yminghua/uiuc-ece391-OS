#if !defined(E391TERMINAL_H)
#define E391TERMINAL_H
#include "types.h"
//create by drush8
//extern volatile int if9pressed;     //only used in cp1:drush8

/******terminalFunc******/


//keep the form that syscall used.
extern int32_t terminal_read(int32_t fd, void *buf, int32_t nbytes);
extern int32_t terminal_write(int32_t fd,const void* buf, int32_t nbytes);
extern int32_t terminal_open(const uint8_t* filename);
extern int32_t terminal_close(int32_t fd);

extern volatile int if9pressed;



/*******inline*********/


#endif /* E391INTREXCEHANDER_H */
