#ifndef STDINT_H
#define STDINT_H
	#include <stdint.h>
#endif
#ifndef SYSTEM_H
#define SYSTEM_H
	#include <system.h>
#endif

#define CODE_SEL 0x08
#define FLAGS 0x8E

// -----------------------------------------------------------------------------
// Interrupt Descpitor Table Module
// --------------------------------
// 
// General	:	The module initializes the Interrupt Descpitor Table.
//
// Input	:	None
//
// Process	:	Initializes the Interrupt Descpitor Table.
//
// Output	:	None
//
// -----------------------------------------------------------------------------
// Programmer	:	Eden Frenkel
// -----------------------------------------------------------------------------

struct idt_entry
{
	uint16_t base_l;
	uint16_t selector;
	uint8_t zero;
	uint8_t flags;
	uint16_t base_h;
} __attribute__((packed));

struct idt_ptr
{
	uint16_t limit;
	uint32_t base;
} __attribute__((packed));

struct idt_entry idt[256];
struct idt_ptr idtp;

extern void load_idtr(void);

extern void exception0(void);
extern void exception1(void);
extern void exception2(void);
extern void exception3(void);
extern void exception4(void);
extern void exception5(void);
extern void exception6(void);
extern void exception7(void);
extern void exception8(void);
extern void exception9(void);
extern void exception10(void);
extern void exception11(void);
extern void exception12(void);
extern void exception13(void);
extern void exception14(void);
extern void exception15(void);
extern void exception16(void);
extern void exception17(void);
extern void exception18(void);

extern void reserved19(void);
extern void reserved20(void);
extern void reserved21(void);
extern void reserved22(void);
extern void reserved23(void);
extern void reserved24(void);
extern void reserved25(void);
extern void reserved26(void);
extern void reserved27(void);
extern void reserved28(void);
extern void reserved29(void);
extern void reserved30(void);
extern void reserved31(void);

extern void irq0(void);
extern void irq1(void);
extern void irq2(void);
extern void irq3(void);
extern void irq4(void);
extern void irq5(void);
extern void irq6(void);
extern void irq7(void);
extern void irq8(void);
extern void irq9(void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);
extern void irq14(void);
extern void irq15(void);

// FUNCTION DELARATIONS
void init_idt(void);
void set_idt_entry(uint8_t index, uint32_t base, uint16_t selector, uint8_t flags);

// -----------------------------------------------------------------------------
// init_idt
// --------
// 
// General		:	The function initializes the Interrupt Descpitor Table.
//
// Parameters	:	None
//
// Return Value	:	None
//
// -----------------------------------------------------------------------------
void init_idt(void)
{
	idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
	idtp.base = (uint32_t) &idt;
	
	memset(&idt, 0, sizeof(struct idt_entry) * 256);
	
	set_idt_entry(0, (uint32_t) &exception0, CODE_SEL, FLAGS);
	set_idt_entry(1, (uint32_t) &exception1, CODE_SEL, FLAGS);
	set_idt_entry(2, (uint32_t) &exception2, CODE_SEL, FLAGS);
	set_idt_entry(3, (uint32_t) &exception3, CODE_SEL, FLAGS);
	set_idt_entry(4, (uint32_t) &exception4, CODE_SEL, FLAGS);
	set_idt_entry(5, (uint32_t) &exception5, CODE_SEL, FLAGS);
	set_idt_entry(6, (uint32_t) &exception6, CODE_SEL, FLAGS);
	set_idt_entry(7, (uint32_t) &exception7, CODE_SEL, FLAGS);
	set_idt_entry(8, (uint32_t) &exception8, CODE_SEL, FLAGS);
	set_idt_entry(9, (uint32_t) &exception9, CODE_SEL, FLAGS);
	set_idt_entry(10, (uint32_t) &exception10, CODE_SEL, FLAGS);
	set_idt_entry(11, (uint32_t) &exception11, CODE_SEL, FLAGS);
	set_idt_entry(12, (uint32_t) &exception12, CODE_SEL, FLAGS);
	set_idt_entry(13, (uint32_t) &exception13, CODE_SEL, FLAGS);
	set_idt_entry(14, (uint32_t) &exception14, CODE_SEL, FLAGS);
	set_idt_entry(15, (uint32_t) &exception15, CODE_SEL, FLAGS);
	set_idt_entry(16, (uint32_t) &exception16, CODE_SEL, FLAGS);
	set_idt_entry(17, (uint32_t) &exception17, CODE_SEL, FLAGS);
	set_idt_entry(18, (uint32_t) &exception18, CODE_SEL, FLAGS);
	
	set_idt_entry(19, (uint32_t) &reserved19, CODE_SEL, FLAGS);
	set_idt_entry(20, (uint32_t) &reserved20, CODE_SEL, FLAGS);
	set_idt_entry(21, (uint32_t) &reserved21, CODE_SEL, FLAGS);
	set_idt_entry(22, (uint32_t) &reserved22, CODE_SEL, FLAGS);
	set_idt_entry(23, (uint32_t) &reserved23, CODE_SEL, FLAGS);
	set_idt_entry(24, (uint32_t) &reserved24, CODE_SEL, FLAGS);
	set_idt_entry(25, (uint32_t) &reserved25, CODE_SEL, FLAGS);
	set_idt_entry(26, (uint32_t) &reserved26, CODE_SEL, FLAGS);
	set_idt_entry(27, (uint32_t) &reserved27, CODE_SEL, FLAGS);
	set_idt_entry(28, (uint32_t) &reserved28, CODE_SEL, FLAGS);
	set_idt_entry(29, (uint32_t) &reserved29, CODE_SEL, FLAGS);
	set_idt_entry(30, (uint32_t) &reserved30, CODE_SEL, FLAGS);
	set_idt_entry(31, (uint32_t) &reserved31, CODE_SEL, FLAGS);
	
	set_idt_entry(32, (uint32_t) &irq0, CODE_SEL, FLAGS);
	set_idt_entry(33, (uint32_t) &irq1, CODE_SEL, FLAGS);
	set_idt_entry(34, (uint32_t) &irq2, CODE_SEL, FLAGS);
	set_idt_entry(35, (uint32_t) &irq3, CODE_SEL, FLAGS);
	set_idt_entry(36, (uint32_t) &irq4, CODE_SEL, FLAGS);
	set_idt_entry(37, (uint32_t) &irq5, CODE_SEL, FLAGS);
	set_idt_entry(38, (uint32_t) &irq6, CODE_SEL, FLAGS);
	set_idt_entry(39, (uint32_t) &irq7, CODE_SEL, FLAGS);
	set_idt_entry(40, (uint32_t) &irq8, CODE_SEL, FLAGS);
	set_idt_entry(41, (uint32_t) &irq9, CODE_SEL, FLAGS);
	set_idt_entry(42, (uint32_t) &irq10, CODE_SEL, FLAGS);
	set_idt_entry(43, (uint32_t) &irq11, CODE_SEL, FLAGS);
	set_idt_entry(44, (uint32_t) &irq12, CODE_SEL, FLAGS);
	set_idt_entry(45, (uint32_t) &irq13, CODE_SEL, FLAGS);
	set_idt_entry(46, (uint32_t) &irq14, CODE_SEL, FLAGS);
	set_idt_entry(47, (uint32_t) &irq15, CODE_SEL, FLAGS);
	
	load_idtr();
}

// -----------------------------------------------------------------------------
// set_idt_entry
// -------------
// 
// General		:	The function modifies an entry in the Interrupt Descpitor Table.
//
// Parameters	:
//		index		-	The index in the IDT (In)
//		base		-	The address to jump to once the interrupt occurs (In)
//		selector	-	The segmant selector of the address (in the GDT) (In)
//		flags		-	The interrupt's flags (In)
//
// Return Value	:	None
//
// -----------------------------------------------------------------------------
void set_idt_entry(uint8_t index, uint32_t base, uint16_t selector, uint8_t flags)
{
	idt[index].base_l = (uint16_t) (base & 0xFFFF);
	idt[index].selector = selector;
	idt[index].zero = 0;
	idt[index].flags = flags;
	idt[index].base_h = (uint16_t) (base >> 16);
}