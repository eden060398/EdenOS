#ifndef STDINT_H
#define STDINT_H
	#include <stdint.h>
#endif
#ifndef STDDEF_H
#define STDDEF_H
	#include <stddef.h>
#endif
#ifndef STDBOOL_H
#define STDBOOL_H
	#include <stdbool.h>
#endif
#ifndef SYSTEM_H
#define SYSTEM_H
	#include <system.h>
#endif

#define B_START ((uint16_t*) 0xB8000)
#define B_END ((uint16_t*) 0xB8FA0)

#define BIOS_IO_VEDIO_PORT_PTR 0x0463

#define NUM_OF_ROWS 25
#define NUM_OF_COLUMNS 80

// -----------------------------------------------------------------------------
// Console Module
// --------------
// 
// General	:	The module initializes the console and supplies functions for
//				printing and communicating with the textual console.
//
// Input	:	None
//
// Process	:	Prints the user-interface and initializes the interface for
//				further use.
//
// Output	:	None
//
// -----------------------------------------------------------------------------
// Programmer	:	Eden Frenkel
// -----------------------------------------------------------------------------

static uint16_t *current, *start, *end;
static uint16_t color;
static bool update_cursor_bool;

const char *TITLE = "EdenOS";

// FUNCTION DECLARATIONS
void init_console(void);
void update_crusor(void);
void putc(int c);
void puts(const char *s);
void putc_at(int c, int row, int column);
void puts_at(const char *s, int row, int column);
void set_cursor(int row, int column);
void clear_screen(void);

// -----------------------------------------------------------------------------
// init_console
// ------------
// 
// General		:	The function initializes the console.
//
// Parameters	:	None
//
// Return Value	:	None
//
// -----------------------------------------------------------------------------
void init_console(void)
{	
	color = (LIGHT_GRAY << 8) | BLACK;
	// Set the background color and the foreground color of the console.
	for (current = B_START; current < B_END; *current++ = color)
		;
	// Set the start of the console region at the beginning of line 2.
	start = B_START + NUM_OF_COLUMNS * 2;
	// Set the end of the console region at the end of line 23 (two lines from the end).
	end = B_END - NUM_OF_COLUMNS * 2;
	// Set the current position at the start.
	current = start;
	// Print the title at the top.
	puts_at(TITLE, 0, 0);
	puts_at("--------------------------------------------------------------------------------", 1, 0);
	puts_at("--------------------------------------------------------------------------------", 23, 0);
	// Set the cursor to be updated.
	update_cursor_bool = true;
	// Update the cursor's position.
	update_crusor();
}

// -----------------------------------------------------------------------------
// update_cursor
// -------------
// 
// General		:	The function updates the cursor's position according to the current
//					position.
//
// Parameters	:	None
//
// Return Value	:	None
//
// -----------------------------------------------------------------------------
void update_crusor(void)
{
	uint16_t port, pos;
	
	port = *((uint16_t *) BIOS_IO_VEDIO_PORT_PTR);
	// Calculate the position of the cursor.
	pos = current - B_START;
	// Cursor LOW port to VGA INDEX register
	OUTB(port, 0x0F);
	OUTB(port + 1, (uint8_t)(pos & 0xFF));
	// Cursor HIGH port to VGA INDEX register
	OUTB(port, 0x0E);
	OUTB(port + 1, (uint8_t)((pos >> 8) & 0xFF));
}

// -----------------------------------------------------------------------------
// putc
// ----
// 
// General		:	The function prints a character at the current position,
//					and adjusts the cursor's position accordingly.
//
// Parameters	:
//		c	-	A char to be printed (In)
//
// Return Value	:	None
//
// -----------------------------------------------------------------------------
void putc(int c)
{
	int n;
	
	switch (c)
	{
		// If newline
		case '\n':
			n = NUM_OF_COLUMNS - (current - start) % NUM_OF_COLUMNS;
			while (n && current < end)
			{
				*current++ = color;
				n--;
			}
			break;
		// If tab
		case '\t':
			n = TAB - (current - start) % TAB;
			while (n && current < end)
			{
				*current++ = color | ' ';
				n--;
			}
			break;
		// If backspace
		case '\b':
			if (current > start)
			{
				*--current = color;
				while (*(current - 1) == color && current > start && (current - start) % NUM_OF_COLUMNS)
					current--;
			}
			break;
		// Else - a regular char
		default:
			if (c < 256)
				*current++ = (uint16_t) c | color;
			break;
	}
	
	if (current < start)
		current = start;
	else if (current >= end)
	{
		uint16_t *p1, *p2;
		p1 = start;
		p2 = (current - end) % NUM_OF_COLUMNS ? p1 + (current - end) - (current - end) % NUM_OF_COLUMNS + NUM_OF_COLUMNS : p1 + (current - end) + NUM_OF_COLUMNS;
		while (p2 < end)
		{
			*p1++ = *p2++;
		}
		current = p1;
		while (p1 < end)
		{
			*p1++ = 0;
		}
	}
	if (update_cursor_bool)
		update_crusor();
}

// -----------------------------------------------------------------------------
// puts
// ----
// 
// General		:	The function prints a string.
//
// Parameters	:
//		s	-	A pointer to a string terminated with a null character (In)
//
// Return Value	:	None
//
// -----------------------------------------------------------------------------
void puts(const char *s)
{
	bool update_crusor_bool_copy;
	
	update_crusor_bool_copy = update_cursor_bool;
	update_cursor_bool = false;
	while (*s)
		putc(*s++);
	if (update_crusor_bool_copy)
		update_crusor();
	update_cursor_bool = update_crusor_bool_copy;
}

// -----------------------------------------------------------------------------
// putc_at
// -------
// 
// General		:	The function prints a character at a specified position.
//
// Parameters	:
//		c		-	A char to be printed (In)
//		row		-	An int that specifies the row the char shall be printed at
//					(In)
//		column	-	An int that specifies the column the char shall be printed
//					at (In)
//
// Return Value	:	None
//
// -----------------------------------------------------------------------------
void putc_at(int c, int row, int column)
{
	if (row >= 0 && row < NUM_OF_ROWS && column >= 0 && column < NUM_OF_COLUMNS)
	{
		*(B_START + row * NUM_OF_COLUMNS + column) = color | (uint16_t) c;
	}
}

// -----------------------------------------------------------------------------
// puts_at
// -------
// 
// General		:	The function prints a string at a specified position.
//
// Parameters	:
//		c		-	A pointer to a string terminated with a null character (In)
//		row		-	An int that specifies the row the string shall be printed at
//					(In)
//		column	-	An int that specifies the column the string shall be printed
//					at (In)
//
// Return Value	:	None
//
// -----------------------------------------------------------------------------
void puts_at(const char *s, int row, int column)
{
	uint16_t *prev_start, *prev_end, *prev_current;
	
	prev_start = start;
	prev_end = end;
	prev_current = current;
	
	start = B_START;
	end = B_END;
	current = B_START + row * NUM_OF_COLUMNS + column;
	update_cursor_bool = false;
	puts(s);
	
	start = prev_start;
	end = prev_end;
	current = prev_current;
	update_cursor_bool = true;
}

// -----------------------------------------------------------------------------
// set_cursor
// ----------
// 
// General		:	The function sets the position of the cursor.
//
// Parameters	:
//		row		-	An int that specifies the row the cursor shall be put at (In)
//		column	-	An int that specifies the column the cursor shall be put at
//					(In)
//
// Return Value	:	None
//
// -----------------------------------------------------------------------------
void set_cursor(int row, int column)
{
	if (row >= 0 && row < NUM_OF_ROWS && column >= 0 && column < NUM_OF_COLUMNS)
	{
		current = B_START + row * NUM_OF_COLUMNS + column;
	}
	update_crusor();
}

// -----------------------------------------------------------------------------
// clear_screen
// ------------
// 
// General		:	The function clears the console.
//
// Parameters	:	None
//
// Return Value	:	None
//
// -----------------------------------------------------------------------------
void clear_screen(void)
{
	current = start;
	while (current < end)
		*current++ = color;
	current = start;
	update_crusor();
}

uint32_t get_current(void)
{
	return current - B_START;
}

void set_color(uint8_t fore, uint8_t back)
{
	color = (fore << 8) | back;
}