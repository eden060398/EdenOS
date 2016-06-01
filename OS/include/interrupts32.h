#ifndef INTERRUPTS32_H
#define INTERRUPTS32_H

#include <system.h>

// DEFINITIONS

#define IRQ_KEYSTROKE 0x01
#define IRQ_RTC   0x08
#define MASTER_CONTROL 0x20
#define MASTER_MASK  0x21
#define SLAVE_CONTROL 0xA0
#define SLAVE_MASK  0xA1
#define EOI    0x20
#define ICW1_ICW4  0x01
#define ICW1_SINGLE  0x02
#define ICW1_INTERVAL4 0x04
#define ICW1_LEVEL  0x08
#define ICW1_INIT  0x10
#define ICW4_8086  0x01
#define ICW4_AUTO  0x02
#define ICW4_BUF_SLAVE 0x08
#define ICW4_BUF_MASTER 0x0C
#define ICW4_SFNM  0x10
#define MASTER_OFFSET 0x20
#define SLAVE_OFFSET 0x28

// FUNCTION DECLARATIONS

void init_interrupts(void);
void exception_handler(uint32_t irq_num);
void reserved_handler(uint32_t irq_num);
void irq_handler(uint32_t irq_num);
void send_eoi(uint32_t irq_num);
void irq_remap(void);

#endif /* INTERRUPTS32_H */

