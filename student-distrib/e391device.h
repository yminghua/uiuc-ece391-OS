#if !defined(E391INTREXCEHANDER_H)
#define E391INTREXCEHANDER_H
//create by drush8



/******interrupt******/
extern void rtc_handler();
extern void keyboard_handler(); //2022.10.16. Now only two interrupt.



/******exception******/







/*******inline*********/
//here for simplify, we use own funcs not ones in lib.h...
extern inline char kbcget()
    {
        register int __out ;
        __asm__ __volatile__ ( "inb $0x60, %%al\n\t"
        :"=a" (__out)::"mem"
        );
        return (char)(__out & 0xFF);
    }

extern inline char kbsget()
    {
        register int __out ;
        __asm__ __volatile__ ( "inb $0x61, %%al\n\t"
        :"=a" (__out)::"mem"
        );
        return (char)(__out & 0xFF);
    }

extern inline void kbssend(char in)
    {
        __asm__ __volatile__ ( "outb %%al, $0x61\n\t"
        ::"a"(in):"mem"
        );
        return;
    }

#endif /* E391INTREXCEHANDER_H */
