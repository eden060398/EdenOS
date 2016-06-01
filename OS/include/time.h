#ifndef TIME_H
#define TIME_H

#include <system.h>

// DEFINITIONS

#define TICK 0.0549254

#define SLAVE_MASK 0xA1

#define RTC_PORT1 0x70
#define RTC_PORT2 0x71
#define SELECT_B 0x0B
#define SELECT_C 0x0C
#define DISABLE_NMI 0x80

// FUNCTION DECLARATIONS

void init_time(void);
void handle_tick(void);
void advance_clock(void);
void init_switch(unsigned int count);
void wait_ticks(int n);

#endif /* TIME_H */

