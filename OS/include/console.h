#ifndef CONSOLE_H
#define CONSOLE_H

#include <system.h>

// DEFINITIONS

#define B_START ((uint16_t*) 0xB8000)
#define B_END ((uint16_t*) 0xB8FA0)

#define BIOS_IO_VEDIO_PORT_PTR 0x0463

#define NUM_OF_ROWS 25
#define NUM_OF_COLUMNS 80

// FUNCTION DECLARATIONS

void init_console(void);
void update_crusor(void);
void putc(int c);
void puts(const char *s);
void putc_at(int c, int row, int column);
void puts_at(const char *s, int row, int column);
void set_cursor(int row, int column);
void clear_screen(void);

#endif /* CONSOLE_H */
