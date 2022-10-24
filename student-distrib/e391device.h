#if !defined(E391DEVICE_H)
#define E391DEVICE_H
//create by drush8

extern volatile int if9pressed;     //only used in cp1:drush8

/******interrupt******/
extern void rtc_handler(void);
//extern void keyboard_handler(void); //2022.10.16. Now only two interrupt.
//extern void keyboard_init(void);
extern void rtc_init(void);






/*******inline*********/


#endif /* E391INTREXCEHANDER_H */
