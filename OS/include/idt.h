#ifndef IDT_H
#define IDT_H

#include <system.h>

// DEFINITIONS

#define CODE_SEL 0x08
#define FLAGS 0x8E

// STRUCTURES

struct idt_entry {
    uint16_t base_l;
    uint16_t selector;
    uint8_t zero;
    uint8_t flags;
    uint16_t base_h;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

// EXTERNAL FUNCTION DECLARATIONS OF IDT ENTRIES

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

#endif /* IDT_H */

