#ifndef STDINT_H
#define STDINT_H
	#include <stdint.h>
#endif
#ifndef SYSTEM_H
#define SYSTEM_H
	#include <system.h>
#endif

#define IRQ_KEYSTROKE	0x01
#define IRQ_RTC 		0x08
#define MASTER_CONTROL	0x20
#define MASTER_MASK		0x21
#define SLAVE_CONTROL	0xA0
#define SLAVE_MASK		0xA1
#define EOI				0x20
#define ICW1_ICW4		0x01
#define ICW1_SINGLE		0x02
#define ICW1_INTERVAL4	0x04
#define ICW1_LEVEL		0x08
#define ICW1_INIT		0x10
#define ICW4_8086		0x01
#define ICW4_AUTO		0x02
#define ICW4_BUF_SLAVE	0x08
#define ICW4_BUF_MASTER	0x0C
#define ICW4_SFNM		0x10
#define MASTER_OFFSET	0x20
#define SLAVE_OFFSET	0x28

// -----------------------------------------------------------------------------
// Interrupts Module
// -----------------
// 
// General	:	The module re-maps the interrupt IRQ Numbers and handles interrupts.
//
// Input	:	None
//
// Process	:	Re-maps the IRQ Numbers and handles interrupts when they occur.
//
// Output	:	None
//
// -----------------------------------------------------------------------------
// Programmer	:	Eden Frenkel
// -----------------------------------------------------------------------------

// FUNCTION DECLARATIONS
void init_interrupts(void);
void exception_handler(uint32_t irq_num);
void reserved_handler(uint32_t irq_num);
void irq_handler(uint32_t irq_num);
void send_eoi(uint32_t irq_num);
void irq_remap(void);

// -----------------------------------------------------------------------------
// init_interrupts
// ---------------
// 
// General		:	The function re-maps the interrupt IRQ Numbers and initiates
//					the interrupts.
//
// Parameters	:	None
//
// Return Value	:	None
//
// -----------------------------------------------------------------------------
void init_interrupts(void)
{
	irq_remap();
	SET_INTS();
}

// -----------------------------------------------------------------------------
// exception_handler
// ----------------
// 
// General		:	The functions handles an Interrupt Request that is at the
//					Exceptions area.
//
// Parameters	:
//		irq_num	-	the number of the Interrupt Request (In)
//
// Return Value	:	None
//
// -----------------------------------------------------------------------------
void exception_handler(uint32_t irq_num)
{
	char *buff = malloc(5);
	
	puts("Exception ");
	puts(itoa(irq_num, buff, 10));
	putc('\n');
	
	free(buff);
	while (1);	
}

// -----------------------------------------------------------------------------
// reserved_handler
// ----------------
// 
// General		:	The functions handles an Interrupt Request that is at the
//					Reserved area (generally, this kind of interrupt shouldn't
//					occur).
//
// Parameters	:
//		irq_num	-	the number of the Interrupt Request (In)
//
// Return Value	:	None
//
// -----------------------------------------------------------------------------
void reserved_handler(uint32_t irq_num)
{
	char *buff = malloc(5);
	
	puts("Reserved ");
	puts(itoa(irq_num, buff, 10));
	putc('\n');
	
	free(buff);
}

// -----------------------------------------------------------------------------
// irq_handler
// -----------
// 
// General		:	The functions handles an Interrupt Request.
//
// Parameters	:
//		irq_num	-	the number of the Interrupt Request (In)
//
// Return Value	:	None
//
// -----------------------------------------------------------------------------
void irq_handler(uint32_t irq_num)
{
	char *buff;
	
	send_eoi(irq_num);
	switch (irq_num)
	{
		case IRQ_KEYSTROKE:
			handle_keystroke();
			break;
		case IRQ_RTC:
			handle_tick();
			break;
		default:
			buff = malloc(5);
			puts("IRQ ");
			puts(itoa(irq_num, buff, 10));
			putc('\n');
			free(buff);
			break;
	}
}

// -----------------------------------------------------------------------------
// send_eio
// --------
// 
// General		:	The functions sends an End Of Interrupt signal to the PICs.
//
// Parameters	:
//		irq_num	-	the number of the Interrupt Request (In)
//
// Return Value	:	None
//
// -----------------------------------------------------------------------------
void send_eoi(uint32_t irq_num)
{
	if(irq_num >= 8)
		// If it is an interrupt from the Slave PIC, send an EOI to it.
		OUTB(SLAVE_CONTROL, EOI);
	// Anyhow, send an EOI to the Master PIC.
	OUTB(MASTER_CONTROL, EOI);
}

// -----------------------------------------------------------------------------
// irq_remap
// ---------
// 
// General		:	The functions re-maps the interrupt IRQ Numbers.
//
// Parameters	:	None
//
// Return Value	:	None
//
// -----------------------------------------------------------------------------
void irq_remap(void)
{
    OUTB(MASTER_CONTROL, ICW1_INIT + ICW1_ICW4);
    OUTB(SLAVE_CONTROL, ICW1_INIT + ICW1_ICW4);
    OUTB(MASTER_MASK, MASTER_OFFSET);
    OUTB(SLAVE_MASK, SLAVE_OFFSET);
    OUTB(MASTER_MASK, 0x04);
    OUTB(SLAVE_MASK, 0x02);
    OUTB(MASTER_MASK, ICW4_8086);
    OUTB(SLAVE_MASK, ICW4_8086);
	// Disable all but the Slave-PIC
	OUTB(MASTER_MASK, 0xFB);
	OUTB(SLAVE_MASK, 0xFF);
}