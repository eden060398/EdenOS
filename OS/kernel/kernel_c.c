#ifndef SYSTEM_H
#define SYSTEM_H
	#include <system.h>
#endif


void kernel_main(void)
{
	init_console();
	puts("CONSOLE INITIATED.\n");
	puts("INITIATING IDT...\n");
	init_idt();
	puts("INITIATING MEMORY...\n");
	init_palloc();
	puts("INITIATING PAGING...\n");
	init_paging();
	puts("INITIATING DYNAMIC MEMORY...\n");
	init_malloc();
	puts("INITIATING INTERRUPT HANDLING...\n");
	init_interrupts();
	puts("INITIATING TIME...\n");
	init_time();
	puts("INITIATING PROCESSING...\n");
	init_processing();
	puts("INITIATING KEYBOARD...\n");
	init_keyboard();
	puts("INITIATING COMMAND PROMPT...\n");
	init_command_prompt();
}
