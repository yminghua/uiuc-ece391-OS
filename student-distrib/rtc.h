#ifndef _RTC_H
#define RTC_H

//create by drush8


/******interrupt******/
extern void rtc_handler(void);
extern void rtc_init(void);


volatile int  rtc_interrupt_occur;



/*******inline*********/
int rtc_open(const unsigned char* filename);

int rtc_read(int fd, void* buf, int nbytes);

int rtc_write(int fd, const void* buf, int nbytes);

int rtc_close(int fd);

int rtc_reset_freq(int freq);

#endif /* E391INTREXCEHANDER_H */

