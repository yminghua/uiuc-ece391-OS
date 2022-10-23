#if !defined(E391TERMINAL_H)
#define E391TERMINAL_H
#include "types.h"
//create by drush8
//extern volatile int if9pressed;     //only used in cp1:drush8

/******terminalFunc******/


//keep the form that syscall used.
extern int32_t terminal_read(void *buf, int32_t nbytes);
extern int32_t terminal_write(const void* buf, int32_t nbytes);
extern int32_t terminal_open(void);
extern int32_t terminal_close(void);





/*******inline*********/


#endif /* E391INTREXCEHANDER_H */