#include "../i8259.h"
#include "pit.h"
#include "../scheduler/scheduler.h"
#include "rtc.h"

/* void pit_init(void);
 * Inputs: none
 * Return Value: none
 * Function: Initialize PIT device to generate interrupts at a rate of 60hz */
void pit_init() {
    NMI_disable();
    enable_irq(PIT_IRQ);

    /* Calculate divisor that achieves 60hz */
    int divisor = PIT_FREQ / 100;

    /* Send command word enabling square wave generation on PIT channel 0 */
    outb(SQ_WAVE_CHZ_CMD, PIT_MODECMD_PORT);

    /* Write divisor to PIT Channel 0 to set square wave frequency.
     * This command word requires writing the lower, then upper byte 
     * of the divisor. The command could be modified if only a single 
     * write was desired. */
    outb(PIT_CHZ_PORT, divisor & 0xFF);
    outb(PIT_CHZ_PORT, divisor >> 8);

    NMI_enable();
}

/* void pit_handler(void);
 * Inputs: none
 * Return Value: none
 * Function: Handle IRQ0 PIT interrupts to trigger scheduling step */
void pit_handler() {
    scheduler_step();
}
