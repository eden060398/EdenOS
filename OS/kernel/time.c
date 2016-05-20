#ifndef STDINT_H
#define STDINT_H
	#include <stdint.h>
#endif

#ifndef STDBOOL_H
#define STDBOOL_H
	#include <stdbool.h>
#endif

#ifndef SYSTEM_H
#define SYSTEM_H
	#include <system.h>
#endif

#define TICK 0.0549254

#define SLAVE_MASK 0xA1

#define RTC_PORT1 0x70
#define RTC_PORT2 0x71
#define SELECT_B 0x0B
#define SELECT_C 0x0C
#define DISABLE_NMI 0x80

// -----------------------------------------------------------------------------
// Time Module
// -----------------
// 
// General	:	The module handles the RTC Tick interrupt and measures time.
//
// Input	:	None
//
// Process	:	Handles the tick and incresing the right counter to keep track
//				of time. Also, provides functions to deal with time.
//
// Output	:	None
//
// -----------------------------------------------------------------------------
// Programmer	:	Eden Frenkel
// -----------------------------------------------------------------------------

static uint32_t secs, mins, hrs, ticks;
char current_time[9] = "00:00:00";
static uint8_t status;
static unsigned int switch_counter;
uint32_t init;

// FUNCTION DECLARATIONS
void init_time(void);
void handle_tick(void);
void advance_clock(void);
void init_switch(unsigned int count);

// -----------------------------------------------------------------------------
// init_time
// ---------
// 
// General		:	The function enables the RTC tick interrupt and the
//					time-handling.
//
// Parameters	:	None
//
// Return Value	:	None
//
// -----------------------------------------------------------------------------
void init_time(void)
{
	uint8_t prev;
	
	secs = mins = hrs = 0;
	ticks = 0;
	switch_counter = 0;
	init = 0;
	
	CLEAR_INTS();
	OUTB(RTC_PORT1, SELECT_B | DISABLE_NMI);
	// Save the previous value of the B register.
	INB(prev, RTC_PORT2);
	OUTB(RTC_PORT1, SELECT_B | DISABLE_NMI);
	// Set bit 6 or register B
	OUTB(RTC_PORT2, prev | 0x40);
	// Save the previous mask.
	INB(prev, SLAVE_MASK);
	// Enable the RTC interrupt.
	OUTB(SLAVE_MASK, prev & ~1);
	SET_INTS();
	
	puts_at(current_time, 24, 71);
}

// -----------------------------------------------------------------------------
// handle_tick
// -----------
// 
// General		:	The function handles an RTC Tick interrupt.
//
// Parameters	:	None
//
// Return Value	:	None
//
// -----------------------------------------------------------------------------
void handle_tick(void)
{
	void advance_clock();
	
	ticks += 1;
	
	OUTB(RTC_PORT1, SELECT_C);
	INB(status, RTC_PORT2);
	
	advance_clock();
	
	if (init)
	{
		if (switch_counter > 0)
			switch_counter--;
		else
		{
			switch_counter = switch_proc();
		}
	}
}

// -----------------------------------------------------------------------------
// advance-clock
// -------------
// 
// General		:	The function advances the clock on the screen.
//
// Parameters	:	None
//
// Return Value	:	None
//
// -----------------------------------------------------------------------------
void advance_clock(void)
{
	bool changed;
	
	changed = false;
	if (ticks == 1024)
	{
		secs += 1;
		ticks = 0;
		if (secs == 60)
		{
			mins += 1;
			secs = 0;
			if (mins == 60)
			{
				hrs += 1;
				mins = 0;
				if (hrs == 24)
					current_time[0] = current_time[1] = current_time[3] = current_time[4] = current_time[6] = current_time[7] = '0';
				else
				{
					current_time[0] = '0' + hrs % 100 / 10;
					current_time[1] = '0' + hrs % 10;
					current_time[3] = current_time[4] = current_time[6] = current_time[7] = '0';
				}
				changed = true;
			}
			if (!changed)
			{
				current_time[3] = '0' + mins / 10;
				current_time[4] = '0' + mins % 10;
				current_time[6] = current_time[7] = '0';
				changed = true;
			}
		}
		if (!changed)
		{
			current_time[6] = '0' + secs / 10;
			current_time[7] = '0' + secs % 10;
		}
		puts_at(current_time, 24, 71);
	}
}

// -----------------------------------------------------------------------------
// init_switch
// -----------
// 
// General		:	The function enables context switching (multi-tasking).
//
// Parameters	:
//		count	-	The number of ticks for the first thread to run before the
//					first switch (In)
//
// Return Value	:	None
//
// -----------------------------------------------------------------------------
void init_switch(unsigned int count)
{
	switch_counter = count;
	init = 1;
}

void wait_ticks(int n)
{
	uint32_t prev_ticks, prev_secs, prev_mins, prev_hrs, passed;
	
	while (n)
	{
		prev_ticks = ticks;
		prev_secs = secs;
		prev_mins = mins;
		prev_hrs = hrs;
		while (	prev_ticks == ticks &&
				prev_secs == secs &&
				prev_mins == mins &&
				prev_hrs == hrs)
			HALT();
		passed = (((hrs - prev_hrs) * 60 + mins - prev_mins) * 60 + secs - prev_secs) * 1024 + ticks - prev_ticks;
		if (passed >= n)
			return;
		n -= passed;
	}
}