# paging.S - initialization for processor paging support
# Enabling paging (and 4MB pages) requires the Paging and
# Page Size Extension flags to be set.

#define ASM 1

#include "paging.h"
#include "../x86_desc.h"

.data                       # section declaration
    # Useful constants for PG, PGE, PSE flags.
    X86_PG_FLAG  = 0x80000000  # Bit 31 of CR0 (Paging Enabled)
    X86_PSE_FLAG = 0x00000010  # Bit 4 of CR4 (Page Size Extension)
    X86_PGE_FLAG = 0x00000080  # Bit 7 of CR4 (Page Global Enabled)

.text                       # section declaration

# Export function symbol names
.globl paging_init

# void paging_init(void);
#
# Interface: Stack-based arguments (C-style)
#    Inputs: uint32_t *pg_dir_ptr - Pointer to page directory base address
#   Outputs: None
# Registers: %ecx - Temp data store
#            %cr0 - Processor control reg; Used to enable paging
#            %cr4 - Processor control reg; Used to enable page size extension for 4MB pages & global pages
paging_init:
    pushl %ebp
    movl %esp, %ebp

    # Update Page Directory Base Register (Control Regsister 3) with physical addr of page dir
    # THIS NEEDS TO HAPPEN BEFORE PAGING IS ENABLED (as per IA-32 docs)
    movl 8(%ebp), %ecx  # First arg on stack is PD base addr
    movl %ecx, %cr3  

    # Read and update CR4 value to enable page size extension & global pages
    movl %cr4, %ecx
    orl $X86_PSE_FLAG, %ecx
    orl $X86_PGE_FLAG, %ecx
    movl %ecx, %cr4

    # Read and update CR0 value to enable paging
    movl %cr0, %ecx
    orl $X86_PG_FLAG, %ecx
    movl %ecx, %cr0

    leave
    ret
