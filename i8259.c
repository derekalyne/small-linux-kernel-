/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask = FULL_MASK; /* IRQs 0-7  */
uint8_t slave_mask = FULL_MASK;  /* IRQs 8-15 */

/* Initialize the 8259 PIC */
void i8259_init(void) {
    
    // mask out everything
    outb(master_mask, MASTER_8259_PORT_DATA);
    outb(slave_mask, SLAVE_8259_PORT_DATA);

    // send init icw to both pics
    outb(ICW1, MASTER_8259_PORT_CMD);
    outb(ICW1, SLAVE_8259_PORT_CMD);

    // specify offsets for both pics
    outb(ICW2_MASTER, MASTER_8259_PORT_DATA);
    outb(ICW2_SLAVE, SLAVE_8259_PORT_DATA);

    // specify master/slave config
    outb(ICW3_MASTER, MASTER_8259_PORT_DATA);
    outb(ICW3_SLAVE, SLAVE_8259_PORT_DATA);

    // send icw4 to both pics
    outb(ICW4, MASTER_8259_PORT_DATA);
    outb(ICW4, SLAVE_8259_PORT_DATA);

    // enable slave PIC
    enable_irq(SLAVE_IRQ);

}

/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num) {
    if(irq_num < NUM_IRQS) {
        // set bit at index irq_num
        master_mask &= ~(1 << irq_num);
        outb(master_mask, MASTER_8259_PORT_DATA);
    } else {
        // set irq num to start at 0
        irq_num -= NUM_IRQS;
        // set bit at index irq_num
        slave_mask &= ~(1 << irq_num);
        outb(slave_mask, SLAVE_8259_PORT_DATA);
    }
}

/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num) {
    if(irq_num < NUM_IRQS) {
        // clear bit at index irq_num
        master_mask |= (1 << irq_num);
        outb(master_mask, MASTER_8259_PORT_DATA);
    } else {
        // set irq num to start at 0
        irq_num -= NUM_IRQS;
        // clear bit at index irq_num
        slave_mask |= (1 << irq_num);
        outb(slave_mask, SLAVE_8259_PORT_DATA);
    }
}


/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {
    if(irq_num >= NUM_IRQS) {
        // OR EOI with irq, -8 to offset slave to index 0
        outb((EOI | (irq_num - NUM_IRQS)), SLAVE_8259_PORT_CMD);
        // send EOI to master PIC
        outb((EOI | SLAVE_IRQ), MASTER_8259_PORT_CMD);
    } else {
        // OR EOI with irq
        outb((EOI | irq_num), MASTER_8259_PORT_CMD);
    }
}
