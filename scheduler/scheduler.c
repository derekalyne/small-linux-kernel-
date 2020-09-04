#include "scheduler.h"
#include "../interrupts/syscalls.h"
#include "../x86_desc.h"
#include "../lib.h"
#include "../debug.h"
#include "../paging/multi_terminals.h"
#include "../devices/pit.h"

uint8_t num_multiprocess;
uint8_t current_multiprocess;
uint8_t on_default_term;
uint8_t keyboard_disabled;
uint8_t multi_process_idx[MAX_MULTIPROCESS_NUM];
process_t all_process[MAX_PROCESSES];


/* void scheduler_init()
 * Inputs: None
 * Return Value: None
 * Inits all local variables for scheduler */
void scheduler_init() {
    num_multiprocess = 0;
    current_multiprocess = 2;
    on_default_term = 0;
    keyboard_disabled = 0;
}

/* void process_context_switch(uint8_t process_from, uint8_t process_to)
 * Inputs: process_from - pid of process we are switching from
           process_to - pid of process we are switching to
 * Return Value: None
 * performs a context switch between kernel stacks of two different processes */
void process_context_switch(uint8_t process_from, uint8_t process_to) {
    /* save esp/ebp */
    asm ("movl %%esp, %0;\n"
            "movl %%ebp, %1;\n"
            : "=g"(all_process[multi_process_idx[process_from]].saved_esp), "=g"(all_process[multi_process_idx[process_from]].saved_ebp)
            :
            : "memory", "cc"
            );
    current_process_pid = multi_process_idx[process_to];

    uint32_t addr = VIDEO_MEM_FULL_ADDR >> THREE_BYTE_SIZE;
    unsigned int page_dir = (unsigned int) PROGRAM_IMAGE_END_ADDRESS >> BITSHIFT_PAGE_OFFSET;
    if(current_term == process_to) {
        // we also want to map vidmap page -> 0xB8000
        page_table_vid[0] = (VIDEO_MEM_FULL_ADDR) | USER_READ_WRITE_PRESENT_ENABLE;			
        page_directory[page_dir] = (unsigned int)(page_table_vid);
        page_directory[page_dir] |= USER_READ_WRITE_PRESENT_ENABLE;
    } else {
        // we also want to map vidmap page -> physical backup buffers
        page_table_vid[0] = (physical_terminal_addresses[process_to]) | USER_READ_WRITE_PRESENT_ENABLE;			
        page_directory[page_dir] = (unsigned int)(page_table_vid);
        page_directory[page_dir] |= USER_READ_WRITE_PRESENT_ENABLE;
    }

    /* switch process paging */
    uint32_t four_mb_page_start = PCB_KERNEL_PHYSICAL_ADDRESS + all_process[multi_process_idx[process_to]].PCB->pid * PROCESS_USER_PHYSICAL_OFFSET;
    // page is 4MB aligned
    uint32_t page_dir_entry = four_mb_page_start | RW_USER_PRESENT_4MB_MASK;
    page_directory[PROCESS_VIRTUAL_ADDRESS_START >> BITSHIFT_PAGE_OFFSET] = page_dir_entry;

    // if current_term == currently scheduled process, changed video memory to be mapped to actual video memory
    // else change video memory to be mapped to the actual physical memory buffer

    // flush tlb
    asm volatile ("movl %cr3,%eax; movl %eax,%cr3");

    uint32_t kernel_stack_base_ptr = PCB_KERNEL_PHYSICAL_ADDRESS - sizeof(int) - (all_process[multi_process_idx[process_to]].PCB->pid * PCB_KERNEL_PHYSICAL_OFFSET);
    tss.esp0 = kernel_stack_base_ptr;

    // only allow typing on active terminal (this also has the super-nice side effect of making sure
    // that anything involving typing such as running commands or scrolling works properly on its own
    // terminal)
    if(process_to != current_term && !keyboard_disabled) {
        disable_irq(1);
        keyboard_disabled = 1;
    }
    else if(process_to == current_term && keyboard_disabled) {
        enable_irq(1);
        keyboard_disabled = 0;
    }

    send_eoi(PIT_IRQ);

    /* restore esp/ebp */
    asm ("movl %0, %%esp;\n"
        "movl %1, %%ebp;\n"
        :
        : "g"(all_process[multi_process_idx[process_to]].saved_esp), "g"(all_process[multi_process_idx[process_to]].saved_ebp)
        : "memory", "cc"
        );
}

/* void scheduler_step()
 * Inputs: None
 * Return Value: None
 * called after 1 time-slice, i.e after PIT interrupt is triggered */
void scheduler_step() {

    if(num_multiprocess != MAX_MULTIPROCESS_NUM) {
        if(num_multiprocess > 0) {
            // since we have a esp/ebp to save, let's save it
            // save our current esp, ebp into its multiprocess struct
            asm ("movl %%esp, %0;\n"
                "movl %%ebp, %1;\n"
                : "=g"(all_process[multi_process_idx[num_multiprocess - 1]].saved_esp), "=g"(all_process[multi_process_idx[num_multiprocess - 1]].saved_ebp)
                :
                : "memory", "cc"
                );
        }

        // setup new process struct
        process_t process;
        process.PCB = PCB[num_multiprocess];
        process.active_terminal_idx = num_multiprocess;
        
        current_process_pid = num_multiprocess;

        multi_process_idx[num_multiprocess] = num_multiprocess;
        all_process[multi_process_idx[num_multiprocess]] = process;
        terminal_switch(num_multiprocess, 0);
        num_multiprocess++;
        // allow for another PIT interrupt
        send_eoi(PIT_IRQ);
        sys_execute((uint8_t *) "shell");
    } else {
        // if we aren't on the default terminal yet, switch to it
        if(!on_default_term) {
            terminal_switch(0, 1);
            on_default_term = 1;
            draw_status_bar();
        }

        uint8_t old_process = current_multiprocess;
        current_multiprocess = (current_multiprocess + 1) % num_multiprocess;
        process_context_switch(old_process, current_multiprocess);
        
    }
    return;
}
