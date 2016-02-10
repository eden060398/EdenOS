#ifndef SYSTEM_H
#define SYSTEM_H
	#include <system.h>
#endif
#ifndef STDINT_H
#define STDINT_H
	#include <stdint.h>
#endif
#ifndef STDBOOL_H
#define STDBOOL_H
	#include <stdbool.h>
#endif

#define DATA_PORT	0x60
#define STATUS_REG	0x64
#define COMMAND_REG	0x64
#define MASTER_MASK 0x21

#define KEYS_COUNT	84

// -----------------------------------------------------------------------------
// Keyboard (Input) Module
// -----------------
// 
// General	:	The module communicates with the keyboard to get input.
//
// Input	:	Keystrokes.
//
// Process	:	Getting scancodes of keystrokes and handling them.
//
// Output	:	None
//
// -----------------------------------------------------------------------------
// Programmer	:	Eden Frenkel
// -----------------------------------------------------------------------------

static bool caps, shift, kbhit;

static int last, last_sc;

static bool kb_status[KEYS_COUNT];
static int scancode_to_ascii[KEYS_COUNT] =
{
	0, 0, '1', '2', '3', '4', '5', '6', '7', '8',
	'9', '0', '-', '=', '\b', '\t', 'q', 'w', 'e', 'r',
	't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
	'\'', '`', 0, '\\', 'z', 'x', 'c', 'v', 'b', 'n',
	'm', ',', '.', '/', 0, 0, 0, ' ', 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, '-', 0, 0, 0, '+', 0,
	0, 0, 0, 0
};
static int scancode_to_ascii_shift[KEYS_COUNT] =
{
	0, 0, '!', '@', '#', '$', '%', '^', '&', '*',
	'(', ')', '_', '+', '\b', '\t', 'Q', 'W', 'E', 'R',
	'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,
	'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',
	'\"', '~', 0, '|', 'Z', 'X', 'C', 'V', 'B', 'N',
	'M', '<', '>', '?', 0, 0, 0, ' ', 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, '-', 0, 0, 0, '+', 0,
	0, 0, 0, 0
};
static int scancode_to_ascii_caps[KEYS_COUNT] =
{
	0, 0, '1', '2', '3', '4', '5', '6', '7', '8',
	'9', '0', '-', '=', '\b', '\t', 'Q', 'W', 'E', 'R',
	'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\n', 0,
	'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';',
	'\'', '`', 0, '\\', 'Z', 'X', 'C', 'V', 'B', 'N',
	'M', ',', '.', '/', 0, 0, 0, ' ', 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, '-', 0, 0, 0, '+', 0,
	0, 0, 0, 0
};
static int scancode_to_ascii_caps_shift[KEYS_COUNT] =
{
	0, 0, '!', '@', '#', '$', '%', '^', '&', '*',
	'(', ')', '_', '+', '\b', '\t', 'q', 'w', 'e', 'r',
	't', 'y', 'u', 'i', 'o', 'p', '{', '}', '\n', 0,
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ':',
	'\"', '~', 0, '|', 'z', 'x', 'c', 'v', 'b', 'n',
	'm', '<', '>', '?', 0, 0, 0, ' ', 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, '-', 0, 0, 0, '+', 0,
	0, 0, 0, 0
};

// -----------------------------------------------------------------------------
// init_keyboard
// -------------
// 
// General		:	The function initializes the communication with the keyboard.
//
// Parameters	:	None
//
// Return Value	:	None
//
// -----------------------------------------------------------------------------
void init_keyboard(void)
{
	int i;
	uint8_t prev;
	
	caps = shift = kbhit = false;
	for (i = 0; i < KEYS_COUNT; i++)
	{
		kb_status[i] = false;
	}
	
	// Get the previos mask.
	INB(prev, MASTER_MASK);
	// Enable IRQ number 2.
	OUTB(MASTER_MASK, prev & ~2);
}

// -----------------------------------------------------------------------------
// get_scancode
// ------------
// 
// General		:	The function gets the last scancode.
//
// Parameters	:	None
//
// Return Value	:	The scancode
//
// -----------------------------------------------------------------------------
int get_scancode(void)
{
	uint8_t ret;
	INB(ret, DATA_PORT);
	return (int) ret;
}

// -----------------------------------------------------------------------------
// handle_keystroke
// ----------------
// 
// General		:	The function handles a keystroke interrupt.
//
// Parameters	:	None
//
// Return Value	:	None
//
// -----------------------------------------------------------------------------
void handle_keystroke(void)
{
	last_sc = get_scancode();
	if (last_sc < KEYS_COUNT)
	{
		if (caps)
		{
			if (shift)
				last = scancode_to_ascii_caps_shift[last_sc];
			else
				last = scancode_to_ascii_caps[last_sc];
		}
		else
		{
			if (shift)
				last = scancode_to_ascii_shift[last_sc];
			else
				last = scancode_to_ascii[last_sc];
		}
		if (!last)
			last = last_sc << 8;
		if (last_sc == 58)
			caps = !caps;
		kb_status[last_sc] = kbhit = true;
	}
	else if (last_sc <= 211)
	{
		kb_status[last_sc - 128] = false;
	}
	shift = kb_status[42] || kb_status[54];
}

// -----------------------------------------------------------------------------
// getc
// ----
// 
// General		:	The function gets a char of the next keystroke.
//
// Parameters	:	None
//
// Return Value	:	The char of the key pressed.
//
// -----------------------------------------------------------------------------
int getc(void)
{
	while (1)
	{
		while (!kbhit)
			HALT();
		kbhit = false;
		if (last)
			return last;
		return 256 + last_sc;
	}
}

// -----------------------------------------------------------------------------
// getc
// ----
// 
// General		:	The function gets a string from keyboard keystrokes (until a newline).
//
// Parameters	:	None
//
// Return Value	:	The string (terminated with a null character).
//
// -----------------------------------------------------------------------------
char *gets(char *buff, int max)
{
	char *s;
	int c, n;
	 
	s = buff;
	max -= 1;
	while ((c = getc()) != '\n' && s - buff < max)
	{
		switch (c)
		{
			case '\b':
				if (s > buff)
				{
					*--s = 0;
					putc(c);
				}
				break;
			case '\t':
				n = TAB - (s - buff) % TAB;
				while (n && s - buff < max)
				{
					*s++ = ' ';
					n--;
				}
				putc(c);
				break;
			default:
				if (c < 256)
				{
					*s++ = (char) c;
					putc(c);
				}
				break;
		}
	}
	putc('\n');
	*s = 0;
	
	return buff;
 }