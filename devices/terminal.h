#include "../lib.h"
#include "keyboard.h"
#include "terminal_structs.h"
#include "../scheduler/scheduler.h"
#include "../paging/multi_terminals.h"

//file must be constant and read-only
//the function initializes terminal
void terminal_init();
//opens file in terminal
int32_t terminal_open(const uint8_t* file);
//closes the terminal
int32_t terminal_close(int fd);
//copies number of bytes to buffer
int32_t terminal_read(int fd,  void *char_buffer, int bytes);
//writes an amount of characters to terminal
int32_t terminal_write(int fd, const void *char_buffer, int bytes);
