#ifndef _PIT_H
#define _PIT_H

#define PIT_IRQ 0
#define PIT_MODECMD_PORT 0x43
#define PIT_CHZ_PORT 0x40
#define PIT_FREQ 1193180

#define SQ_WAVE_CHZ_CMD 0x36

void pit_init(void);

void pit_handler(void);

#endif /* _PIT_H */
