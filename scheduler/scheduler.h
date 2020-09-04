#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#ifndef ASM

#include "../interrupts/syscall_structs.h"
#include "../devices/keyboard.h"
#include "../i8259.h"

// for the 3 shells we are running
#define MAX_MULTIPROCESS_NUM 3

typedef struct {
    // the PCB block for each process
    PCB_BLOCK_t* PCB;
    // saved kernel esp
    uint32_t saved_esp;
    // saved kernel ebp
    uint32_t saved_ebp;
    // which terminal is process running on
    uint8_t active_terminal_idx;
} process_t;

/* init stuff relating to scheduler */
extern void scheduler_init();

/* called after 1 time-slice, i.e after PIT interrupt is triggered */
extern void scheduler_step();

/* performs a context switch between kernel stacks of two different processes */
extern void process_context_switch(uint8_t process_from, uint8_t process_to);

// this is where all the process_t structs are stored
extern process_t all_process[MAX_PROCESSES];

// this is where the indexes of the processes im currently scheduling are stored
extern uint8_t multi_process_idx[MAX_MULTIPROCESS_NUM];

// which idx in multi_process_idx is currently scheduled
extern uint8_t current_multiprocess;

#endif

#endif /* _SCHEDULER_H */
