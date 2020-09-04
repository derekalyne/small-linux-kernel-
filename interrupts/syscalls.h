#include "../x86_desc.h"
#include "../lib.h"
#include "../debug.h"

#include "../paging/page_structs.h"
#include "../devices/rtc.h"
#include "../devices/terminal.h"
#include "../devices/mouse.h"
#include "../fs/fs.h"
#include "../fs/file.h"
#include "../fs/directory.h"
#include "../scheduler/scheduler.h"
#include "syscall_structs.h"

#ifndef _SYSCALLS_H
#define _SYSCALLS_H

// end of kernel
#define PCB_KERNEL_PHYSICAL_ADDRESS 0x800000
// size of PCB
#define PCB_KERNEL_PHYSICAL_OFFSET 0x2000
// start of user programs in physical mem
#define PROCESS_USER_PHYSICAL_OFFSET 0x400000
// start of user program in virtual mem
#define PROCESS_VIRTUAL_ADDRESS_START 0x08000000
// start of entry address in virtual mem
#define PROGRAM_VIRTUAL_ADDRESS_START 0x08048000

#define EXCEPTION_CODE 80
#define HALT_EXCEPTION 256

#define ENTRY_ADDRESS_BYTE_1 24
#define ENTRY_ADDRESS_BYTE_2 25
#define ENTRY_ADDRESS_BYTE_3 26
#define ENTRY_ADDRESS_BYTE_4 27
#define BITSHIFT_3_BYTES 24
#define BITSHIFT_2_BYTES 16
#define BITSHIFT_1_BYTES 8
#define BITSHIFT_PAGE_OFFSET 22
#define FILE_HEADER_SIZE 4

#define FLAG_SET 1
#define FLAG_UNSET 0
#define RETURN_FAIL -1
#define RETURN_PASS 0

// start of user program in virtual mem
#define PROGRAM_IMAGE_START_ADDRESS 0x8000000
// end of user program in virtual mem
#define PROGRAM_IMAGE_END_ADDRESS 0x8400000

void init_PCBs();

extern int sys_halt_wrapper(uint32_t status);

extern int32_t sys_halt(uint8_t status);

extern int32_t sys_execute(const uint8_t* command);

extern int32_t sys_read(int32_t fd, void* buf, int32_t nbytes);

extern int32_t sys_write(int32_t fd, const void* buf, int32_t nbytes);

extern int32_t sys_open(const uint8_t* filename);

extern int32_t sys_close(int32_t fd);

extern int32_t sys_getargs(uint8_t* buf, int32_t nbytes);

extern int32_t sys_vidmap(uint8_t** screen_start);

extern int32_t sys_set_handler(int32_t signum, void* handler_address);

extern int32_t sys_sigreturn(void);

#endif
