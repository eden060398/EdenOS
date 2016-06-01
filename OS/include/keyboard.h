#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <system.h>

// DEFINITIONS

#define DATA_PORT 0x60
#define STATUS_REG 0x64
#define COMMAND_REG 0x64
#define MASTER_MASK 0x21

#define KEYS_COUNT 84

// FUNCTION DECLARATIONS

void init_keyboard(void);
int get_scancode(void);
int get_scancode(void);
int get_scancode(void);
int get_scancode(void);

#endif /* KEYBOARD_H */

