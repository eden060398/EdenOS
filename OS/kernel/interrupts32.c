// -----------------------------------------------------------------------------
// Interrupts Module
// -----------------
// 
// General  :   The module re-maps the interrupt IRQ Numbers and handles
//              interrupts.
//
// Input    :   None
//
// Process  :   Re-maps the IRQ Numbers and handles interrupts when they occur.
//
// Output   :   None
//
// -----------------------------------------------------------------------------
// Programmer   :   Eden Frenkel
// -----------------------------------------------------------------------------


#include <interrupts32.h>


// -----------------------------------------------------------------------------
// init_interrupts
// ---------------
// 
// General      :   The function re-maps the interrupt IRQ Numbers and initiates
//                  the interrupts.
//
// Parameters   :   None
//
// Return Value :   None
//
// -----------------------------------------------------------------------------

void init_interrupts(void) {
    irq_remap();
    SET_INTS();
}

// -----------------------------------------------------------------------------
// exception_handler
// ----------------
// 
// General      :   The functions handles an Interrupt Request that is at the
//                  Exceptions area.
//
// Parameters   :
//              irq_num -   the number of the Interrupt Request (In)
//
// Return Value :   None
//
// -----------------------------------------------------------------------------

void exception_handler(uint32_t irq_num) {
    char *buff;

    buff = malloc(5);

    puts("Exception ");
    puts(itoa(irq_num, buff, 10));
    putc('\n');

    free(buff);

    CLEAR_INTS();
    HALT();
}

// -----------------------------------------------------------------------------
// reserved_handler
// ----------------
// 
// General      :   The functions handles an Interrupt Request that is at the
//                  Reserved area (generally, this kind of interrupt shouldn't
//                  occur).
//
// Parameters   :
//              irq_num -   the number of the Interrupt Request (In)
//
// Return Value :   None
//
// -----------------------------------------------------------------------------

void reserved_handler(uint32_t irq_num) {
    char *buff;

    buff = malloc(5);

    puts("Reserved ");
    puts(itoa(irq_num, buff, 10));
    putc('\n');

    free(buff);
}

// -----------------------------------------------------------------------------
// irq_handler
// -----------
// 
// General      :   The functions handles an Interrupt Request.
//
// Parameters   :
//              irq_num -   the number of the Interrupt Request (In)
//
// Return Value :   None
//
// -----------------------------------------------------------------------------

void irq_handler(uint32_t irq_num) {
    char *buff;

    send_eoi(irq_num);
    switch (irq_num) {
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
// General      :   The functions sends an End Of Interrupt signal to the PICs.
//
// Parameters   :
//              irq_num -   the number of the Interrupt Request (In)
//
// Return Value :   None
//
// -----------------------------------------------------------------------------

void send_eoi(uint32_t irq_num) {
    if (irq_num >= 8)
        // If it is an interrupt from the Slave PIC, send an EOI to it.
        OUTB(SLAVE_CONTROL, EOI);
    // Anyhow, send an EOI to the Master PIC.
    OUTB(MASTER_CONTROL, EOI);
}

// -----------------------------------------------------------------------------
// irq_remap
// ---------
// 
// General      :   The functions re-maps the interrupt IRQ Numbers.
//
// Parameters   :   None
//
// Return Value :   None
//
// -----------------------------------------------------------------------------

void irq_remap(void) {
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