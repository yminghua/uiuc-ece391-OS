#ifndef     SIGNAL_H
#define     SIGNAL_H

#include "types.h"
#include "idt.h"

#define     SIGNAL_DIV_ZERO     0
#define     SIGNAL_SEGFAULT     1 
#define     SIGNAL_INTERRUPT    2 
#define     SIGNAL_ALARM        3 
#define     SIGNAL_USER1        4 
#define     MAX_SIGNAL_NUM      5

#define     MASK_ALL_SIGNALS    0xFFFFFFFF
#define     ALARM_TIME          10000   // 10s = 10000ms

typedef int32_t (*signal_handler)(void);

typedef struct signal {
    uint32_t pending_signal;
    uint32_t mask;
    uint32_t prev_mask;
    uint32_t alarm_time;
    signal_handler signal_handler[MAX_SIGNAL_NUM]; // Store the signal handle
} signal_t;


extern int32_t set_handler(int32_t signum, void *handler_address);

void init_signal_default_handler();
int32_t signal_init(signal_t* signal_struct);
int32_t signal_send(int32_t signum); 
int32_t signal_mask(int32_t signum);
int32_t signal_unmask(int32_t signum);
extern void restore_signal_mask();

int32_t sig_div_zero_default_handler();
int32_t sig_seg_default_handler();
int32_t sig_interrupt_default_handler();
int32_t sig_alarm_default_handler();
int32_t sig_user1_default_handler();

#endif  /* SIGNAL_H */
