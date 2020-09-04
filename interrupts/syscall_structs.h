#include "../types.h"

#ifndef __SYSCALL_STRUCT_H
#define __SYSCALL_STRUCT_H

#define MAX_PROCESSES 6
#define DISABLE_LAST_BIT_MASK 0xFFFFFFFE

// Number of characters the terminal supports + null-terminator 
#define ARG_BUF_SIZE 122

#define STDIN_FD 0
#define STDOUT_FD 1
#define FIRST_READABLE_FILE 2 //file 0 and file 1 are stdin and stdout 
#define FILE_DESCRIPTOR_SIZE  8 

#define PRESENT_BITMASK 0x1

#define GET_PID_MASK 0x1e000
#define GET_PID_BITSHIFT 13
#define GET_PID_OFFSET 15

// struct for file operations table
typedef struct {
    int32_t (*open) (const unsigned char*);
    int32_t (*close) (int32_t); 
    int32_t (*read) (int, void*, int); 
    int32_t (*write) (int, const void*, int);
} driver_t; 

// struct for file descriptor entry
typedef struct {
    driver_t operation_table; 
    int32_t inode; 
    int32_t file_position; 
    // 0th bit of flags determines whether file is open or closed
    int32_t flags;
} file_array_t; 

// struct for PCB block
typedef struct PCB_BLOCK_t{
    file_array_t file[8];
    uint8_t running;  
    uint32_t esp; 
    uint32_t ebp;
    uint8_t pid; 
    uint8_t flags;
    uint8_t args[ARG_BUF_SIZE]; 
    uint8_t cmd_name[ARG_BUF_SIZE];

    struct PCB_BLOCK_t* parent;
} PCB_BLOCK_t; 

// global variables
extern uint8_t current_process_pid;
extern PCB_BLOCK_t* PCB[MAX_PROCESSES];

// func for getting ebp
extern uint32_t get_current_ebp();

#endif // __SYSCALL_STRUCT_H
