#ifndef _RTC_H
#define RTC_H

//create by drush8


/******interrupt******/
extern void rtc_handler(void);
extern void rtc_init(void);


volatile int  rtc_interrupt_occur;



/*******inline*********/
int rtc_open(const uint8_t* filename);

int rtc_read(int32_t fd, void* buf, int32_t nbytes);

int rtc_write(int32_t fd, const void* buf, int32_t nbytes);

int rtc_close(int32_t fd);

int rtc_reset_freq(int freq);

#endif /* E391INTREXCEHANDER_H */

