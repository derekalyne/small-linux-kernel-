#include "syscalls.h"

// .ELF header
int8_t executable_magic_numbers[FILE_HEADER_SIZE] = {0x7F, 0x45, 0x4C, 0x46};

static int32_t error_return_value = FLAG_UNSET;

// file op tables
driver_t terminal;
driver_t rtc;
driver_t files;
driver_t directories;
driver_t mouse;

/* init_PCBs
 * 
 * Description: initializes PCBs, creates one at the top of each possible processes kernel stack, 
 * and stores pointers in an array. It also setups the file operation tables
 * Inputs: None
 * Outputs: None
 * Side Effects: writes to kernel memory
 */
void init_PCBs() {
    int idx;
    for(idx = 0; idx < MAX_PROCESSES; idx++) {
        PCB[idx] = (PCB_BLOCK_t *) (PCB_KERNEL_PHYSICAL_ADDRESS - PCB_KERNEL_PHYSICAL_OFFSET - (idx * PCB_KERNEL_PHYSICAL_OFFSET));
		PCB[idx]->running = 0;
    }

	terminal.read = &terminal_read;
	terminal.write = &terminal_write;

	rtc.open = &rtc_open;
	rtc.close = &rtc_close;
	rtc.read = &rtc_read;
	rtc.write = &rtc_write;

	files.open = &file_open;
	files.close = &file_close;
	files.read = &file_read;
	files.write = &file_write;

	directories.open = &dir_open;
	directories.close = &dir_close;
	directories.read = &dir_read;
	directories.write = &dir_write;

	mouse.open = &mouse_open;
	mouse.close = &mouse_close;
	mouse.read = &mouse_read;
	mouse.write = &mouse_write;
}

/* sys_halt_wrapper
 * 
 * Description: wrapper for halt syscall, takes in a 32-bit status and checks if 
 * an exception was caused and based on that, modifies a flag, and calls halt syscall with 8-bit status
 * Inputs: uint32_t status -- 32 bit status
 * Outputs: return value from halt syscall
 * Side Effects: None
 */
int sys_halt_wrapper(uint32_t status) {
	if(status == HALT_EXCEPTION) {
		error_return_value = HALT_EXCEPTION;
		return sys_halt(0);
	}
	return sys_halt((uint8_t) status & 0xFF);
}

/* sys_halt
 * 
 * Description: System call for halt, takes in status code, clears PCB for current process,
 * sets up parent process PCB, and makes call to execute
 * Inputs: uint8_t status -- 8 bit status
 * Outputs: return value from halt syscall
 * Side Effects: Jumps to execute system call
 */
int sys_halt(uint8_t status) {
	// get pointer to current PCB
	// printf("Halting from pid: %d", current_process_pid);
	PCB_BLOCK_t* current_PCB = PCB[current_process_pid];
	
	// check to see if we are halting root process
	if(current_PCB == current_PCB->parent || !current_PCB->parent) {
		// 31 is size of error msg
    	sys_write(1, (void*) "Cannot halt from root process!\n", 31);

		current_PCB->running = FLAG_UNSET;
		int i;
		for(i = FIRST_READABLE_FILE; i < FILE_DESCRIPTOR_SIZE; i++) {
			if(current_PCB->file[i].flags & PRESENT_BITMASK) {
				current_PCB->file[i].flags &= DISABLE_LAST_BIT_MASK;
				current_PCB->file[i].operation_table.close(i);
			}
		}

		current_process_pid = 0;

		error_return_value = status;

		// spawn a new shell
		sys_execute((uint8_t *) "shell");
	}

	// get link to parent PCB, shouldn't be null at this point
	PCB_BLOCK_t* parent_PCB = current_PCB->parent;
	// clear current PCB and close all fd
	current_PCB->running = FLAG_UNSET;
	int i;
	for(i = FIRST_READABLE_FILE; i < FILE_DESCRIPTOR_SIZE; i++) {
		if(current_PCB->file[i].flags & PRESENT_BITMASK) {
			current_PCB->file[i].flags &= DISABLE_LAST_BIT_MASK;
			current_PCB->file[i].operation_table.close(i);
		}
	}


	// we want to replace the old process_t with its parent process_t
	// we want to avoid synchronization issues so cli, sti
	uint32_t flags;
	cli_and_save(flags);

	// swap paging back to parent's paging
	uint32_t four_mb_page_start = PCB_KERNEL_PHYSICAL_ADDRESS + parent_PCB->pid * PROCESS_USER_PHYSICAL_OFFSET;
	// page is 4MB aligned
	uint32_t page_dir_entry = four_mb_page_start | RW_USER_PRESENT_4MB_MASK;
	page_directory[PROCESS_VIRTUAL_ADDRESS_START >> BITSHIFT_PAGE_OFFSET] = page_dir_entry;

	// flush TLB
	asm volatile ("movl %cr3,%eax; movl %eax,%cr3");

	// get parents kernel stack base address
	uint32_t parent_kernel_stack_base_address = PCB_KERNEL_PHYSICAL_ADDRESS - sizeof(int) - (parent_PCB->pid * PCB_KERNEL_PHYSICAL_OFFSET);
	tss.esp0 = parent_kernel_stack_base_address;

	// modify the multiprocessing structs
	if(current_process_pid >= MAX_MULTIPROCESS_NUM) {
		// iterate through multi_process_idx to find the child idx and replace with parent idx
		int i;
		for(i = 0; i < MAX_MULTIPROCESS_NUM; i++) {
			if(multi_process_idx[i] == current_process_pid) {
				multi_process_idx[i] = parent_PCB->pid;
				break;
			}
		}
	}

	draw_status_bar();

	// set current pid to parent's pid
	current_process_pid = parent_PCB->pid;

	// jump to return in sys_execute
	if(error_return_value != HALT_EXCEPTION) {
		error_return_value = status;
	}

	// restore our esp, ebp
	asm ("movl %0, %%esp;\n"
		 "movl %1, %%ebp;\n"
		  :
		  : "g"(current_PCB->esp), "g"(current_PCB->ebp)
		  : "memory", "cc"
		);

	restore_flags(flags);

	asm ("jmp return_from_halt");
	return RETURN_PASS;
}

/* sys_execute
 * 
 * Description: System call for execute, takes in char buffer for command, sets up PCB for process,
 * setups up TSS, stores registers, sets up IRET context and calls IRET
 * Inputs: uint8_t* command - a char buffer for the command
 * Outputs: return value from halt syscall
 * Side Effects: Modifies paging, flushes TLB, calls IRET, modifies esp/ebp
 */
int32_t sys_execute(const uint8_t* command) {
	
	// Reenable interrupts, as user programs should always be interruptable.
	sti();

	if(command[0] == '\0') {
		return RETURN_FAIL;
	}

	uint8_t command_name[ARG_BUF_SIZE];
	uint8_t argument_name[ARG_BUF_SIZE];
	memset(command_name, '\0', ARG_BUF_SIZE);
	memset(argument_name, '\0', ARG_BUF_SIZE);

	uint8_t * trimmed_command = (uint8_t *) strtrimlead((int8_t *) command);
	if(!trimmed_command) {
		return RETURN_FAIL;
	}

	// Find end of first word
	uint8_t * command_end = (uint8_t *) strchr((int8_t *) trimmed_command, ' ');
	// command has stuff added on to end
	if(command_end) {
		// Copy command name to name buf
		memcpy(command_name, trimmed_command, (uint32_t) (command_end - trimmed_command));
		uint8_t * args = (uint8_t *) strtrimlead((int8_t *) command_end);
		// command has arguments
		if(args) {
			// Copy arguments into argument buf
			memcpy(argument_name, args, strlen((int8_t *) args));
		}
	} else {
		memcpy(command_name, trimmed_command, strlen((int8_t *)trimmed_command));
	}
	
	// make sure file is executable
	dentry_t ret;
	int32_t status;
	status = read_dentry_by_name((uint8_t *) command_name, &ret);
	
	// file exist check
	if(status == RETURN_FAIL) {
		return RETURN_FAIL;
	}

	uint32_t file_size = get_file_size(&ret);
	uint8_t buf[FILE_HEADER_SIZE];
	memset(buf, '\0', FILE_HEADER_SIZE);
	read_data(ret.inode_num, 0, buf, FILE_HEADER_SIZE);

	// file executable check
	if(strncmp(executable_magic_numbers, (int8_t* ) buf, FILE_HEADER_SIZE) != 0) {
		return RETURN_FAIL;
	}

	// get a pointer to parent PCB
	PCB_BLOCK_t* parent = PCB[current_process_pid];

	// create a multiprocess struct as well
	// we need to find the ultimate root to determine which shell the process is coming from
	PCB_BLOCK_t* root = parent;
	while(root->parent != NULL) 
		root = root->parent;
	// the terminal will be of idx root->pid
	// TODO: map process to terminal here


	uint8_t found_pid = FLAG_UNSET;
	int idx = 0;
	for(idx = 0; idx < MAX_PROCESSES; idx++) {
		if(!PCB[idx]->running) {
			found_pid = FLAG_SET;
			current_process_pid = idx;
			PCB[idx]->running = FLAG_SET;
			PCB[idx]->pid = idx;
			
			// TODO: this should work fine after we get interrupts working... i think
			// the first 3 processes are supposed to be the root shells which have no parent
			if(idx < MAX_MULTIPROCESS_NUM) {
				PCB[idx]->parent = NULL;
			} else {
				PCB[idx]->parent = parent;
			}

			// setup arguments for PCB
			strcpy((int8_t *) PCB[idx]->args, (int8_t *) argument_name);
			strcpy((int8_t *) PCB[idx]->cmd_name, (int8_t *) command_name);

			// setup file descriptor array for PCB, slots 0 and 1 are terminal driver functions
			PCB[idx]->file[0].operation_table = terminal;
			PCB[idx]->file[1].operation_table = terminal;

			PCB[idx]->file[0].flags |= FLAG_SET;
			PCB[idx]->file[1].flags |= FLAG_SET;

			break;
		}
	}

	if(!found_pid) {
		// 51 is size of error msg
		sys_write(1, (void*)"Cannot create new process, reached maximum amount!\n", 51);
		return RETURN_PASS;
	}

	uint32_t flags;
	cli_and_save(flags);

	// setup paging with the process pid, mapped at virtual address 0x800000
	uint32_t four_mb_page_start = PCB_KERNEL_PHYSICAL_ADDRESS + current_process_pid * PROCESS_USER_PHYSICAL_OFFSET;
	// page is 4MB aligned
	uint32_t page_dir_entry = four_mb_page_start | RW_USER_PRESENT_4MB_MASK;
	page_directory[PROCESS_VIRTUAL_ADDRESS_START >> BITSHIFT_PAGE_OFFSET] = page_dir_entry;

	// flush tlb
	asm volatile ("movl %cr3,%eax; movl %eax,%cr3");

	// load program into memory
	uint8_t* ptr = (uint8_t * ) PROGRAM_VIRTUAL_ADDRESS_START;
	read_data(ret.inode_num, 0, ptr, file_size);

	// get programs entry address stored in bytes 24-27 of executable
	uint32_t entry_address = (uint32_t) ptr[ENTRY_ADDRESS_BYTE_4] << BITSHIFT_3_BYTES | (uint32_t) ptr[ENTRY_ADDRESS_BYTE_3] << BITSHIFT_2_BYTES | (uint32_t) ptr[ENTRY_ADDRESS_BYTE_2] << BITSHIFT_1_BYTES | (uint32_t) ptr[ENTRY_ADDRESS_BYTE_1];

	// subtracting 4 b/c we want the address right above the bottom of kernel stack
	uint32_t kernel_stack_base_ptr = PCB_KERNEL_PHYSICAL_ADDRESS - sizeof(int) - (current_process_pid * PCB_KERNEL_PHYSICAL_OFFSET);
	uint32_t user_stack_base_ptr = PROCESS_VIRTUAL_ADDRESS_START + PROCESS_USER_PHYSICAL_OFFSET;

	tss.esp0 = kernel_stack_base_ptr;

	// save our current esp, ebp into the PCB
	asm ("movl %%esp, %0;\n"
		 "movl %%ebp, %1;\n"
		  : "=g"(PCB[current_process_pid]->esp), "=g"(PCB[current_process_pid]->ebp)
		  :
		  : "memory", "cc"
		);

	// we only want to create new process_t for non-root shell
	// we want to avoid synchronization issues so cli, sti
	
	if(current_process_pid >= MAX_MULTIPROCESS_NUM) {
		process_t process;
		process.PCB = PCB[current_process_pid];
		process.active_terminal_idx = root->pid;

		// iterate through multi_process_idx to find the parent idx and replace with child idx
		int i;
		for(i = 0; i < MAX_MULTIPROCESS_NUM; i++) {
			if(multi_process_idx[i] == parent->pid) {
				multi_process_idx[i] = current_process_pid;
				break;
			}
		}
		all_process[current_process_pid] = process;
	}

	draw_status_bar();

	restore_flags(flags);

	// push user ds, esp, eflags, cs, eip using inline ASM and call IRET
	asm ("pushl %2;\n"
		 "pushl %3;\n"
		//  "sti     ;\n"
		 "pushf   ;\n"
		 "pushl %1;\n"
		 "pushl %0;\n"
		  :        				/* no output */
          : "g"(entry_address), "g"(USER_CS), "g"(USER_DS), "g"(user_stack_base_ptr)
          : "memory", "cc"
        );

	// iret
	asm ("iret;");

	// label for halt to return back to
	asm ("return_from_halt:");
	
	// save return value
	int ret_value = error_return_value;
	// reset return flag
	error_return_value = FLAG_UNSET;
	
	return ret_value;
}

/* sys_read
 * 
 * Description: System call for read, calls the read function from appropriate file descriptor entry
 * Inputs: int32_t fd -- file descriptor index
 *		   void* buf -- pointer to char buffer to store result in
 * 		   int32_t nbytes -- how many bytes to read
 * Outputs: return -1 for fail, 0 for success
 * Side Effects: Calls from file operations table
 */
int sys_read(int32_t fd, void* buf, int32_t nbytes) {
	if(fd >= FILE_DESCRIPTOR_SIZE || fd < 0 || fd == STDOUT_FD) {
		return RETURN_FAIL;
	} else if((PCB[current_process_pid]->file[fd].flags & PRESENT_BITMASK) == FLAG_UNSET) {
		return RETURN_FAIL;
	} else {
		return PCB[current_process_pid]->file[fd].operation_table.read(fd, buf, nbytes);
	}
	return RETURN_PASS;
}

/* sys_write
 * 
 * Description: System call for write, calls the write function from appropriate file descriptor entry
 * Inputs: int32_t fd -- file descriptor index
 *		   void* buf -- pointer to char buffer with the data to write
 * 		   int32_t nbytes -- how many bytes to write
 * Outputs: return -1 for fail, 0 for success
 * Side Effects: Calls from file operations table
 */
int sys_write(int32_t fd, const void* buf, int32_t nbytes) {
	if(fd >= FILE_DESCRIPTOR_SIZE || fd < 0 || fd == STDIN_FD) {
		return RETURN_FAIL;
	} else if((PCB[current_process_pid]->file[fd].flags & PRESENT_BITMASK) == FLAG_UNSET) {
		return RETURN_FAIL;
	} else {
		return PCB[current_process_pid]->file[fd].operation_table.write(fd, buf, nbytes);
	}
	return RETURN_PASS;
}

/* sys_open
 * 
 * Description: System call for open, based on the filename, figure out whether to use predefined 
 * 				file descriptor entry, or to setup new entry, and returns the right fd
 * Inputs: uint8_t* filename -- filename for file to open
 * Outputs: return the fd
 * Side Effects: Creates an entry in the file descriptor array
 */
int sys_open(const uint8_t* filename) {
	// set the file descript entry flag bit 0 to be 1, in the first unused file desc entry
	if(filename[0] == '\0') {
		return RETURN_FAIL;
	}

	uint8_t is_mouse_entry_flag = 0;
	if(strncmp((int8_t *) filename, (int8_t *) "mouse", 6) == 0) {
		is_mouse_entry_flag = 1;
	}

	dentry_t entry;
	if(!is_mouse_entry_flag) {
		if(read_dentry_by_name(filename, &entry) == RETURN_FAIL) {
			/* Create a new file if the file requested does not exist. */
			dentry_t * new_dentry = create_new_file(filename);
			if (new_dentry == NULL) {
				return RETURN_FAIL;
			} else {
				entry = *new_dentry;
			}
		}
	}

	uint32_t dir_file_type = entry.file_type;
	uint32_t idx;
	for(idx = FIRST_READABLE_FILE; idx < FILE_DESCRIPTOR_SIZE; idx++) {
		if((PCB[current_process_pid]->file[idx].flags & PRESENT_BITMASK) == FLAG_UNSET) {
			PCB[current_process_pid]->file[idx].inode = entry.inode_num;
			PCB[current_process_pid]->file[idx].file_position = 0;
			PCB[current_process_pid]->file[idx].flags |= FLAG_SET;  // Set present bit
			if(is_mouse_entry_flag) {
				// we need to create a mouse op table
				PCB[current_process_pid]->file[idx].operation_table = mouse;
			}
			else if(dir_file_type == FS_TYPE_RTC) {
				PCB[current_process_pid]->file[idx].flags |= 0x2; // Set this bit to indicate rtc fd
				// Set RTC to default freq
				PCB[current_process_pid]->file[idx].file_position = RTC_MAX_FREQ / DEFAULT_RTC_FREQ;
				PCB[current_process_pid]->file[idx].operation_table = rtc;
			}
			else if(dir_file_type == FS_TYPE_DIR) {
				PCB[current_process_pid]->file[idx].operation_table = directories;
			}
			else if(dir_file_type == FS_TYPE_FILE) {
				PCB[current_process_pid]->file[idx].operation_table = files;
			}
			PCB[current_process_pid]->file[idx].operation_table.open(filename);
			return idx;
		}
	}
	return RETURN_FAIL;
}

/* sys_close
 * 
 * Description: System call for close, based on the fd, calls close from file operations table
 * Inputs: int32_t fd -- the right file descriptor index
 * Outputs: return a status code
 * Side Effects: Calls 
 */
int sys_close(int32_t fd) {
	if(fd < FIRST_READABLE_FILE || fd >= FILE_DESCRIPTOR_SIZE) {
		return RETURN_FAIL;
	} else if((PCB[current_process_pid]->file[fd].flags & PRESENT_BITMASK) == FLAG_SET) {
		PCB[current_process_pid]->file[fd].operation_table.close(fd);
		PCB[current_process_pid]->file[fd].flags &= FLAG_UNSET;  // Clear present bit and flags
		PCB[current_process_pid]->file[fd].inode = 0;
		PCB[current_process_pid]->file[fd].file_position = 0;
		return RETURN_PASS;
	}
	return RETURN_FAIL;
}

/* sys_getargs
 * 
 * Description: System call returning the saved args in the PCB to user
 * Inputs: uint8_t* buf -- buffer to store arguments in
		   int32_t nbytes -- number of bytes to copy
 * Outputs: return a status code
 * Side Effects: Copies from kernel space to user space
 */
int32_t sys_getargs(uint8_t* buf, int32_t nbytes) {
	int8_t * args = (int8_t *) PCB[current_process_pid]->args;

	// Must fit args and null terminator; args must not be empty

	if(args[0] == '\0') {
		return RETURN_FAIL;
	}

	nbytes = (nbytes > strlen(args) + 1) ? strlen(args) + 1 : nbytes;

	// Copy arguments into user buffer
	int32_t i;
	for(i = 0; i < nbytes; i++) {
		buf[i] = args[i];
	}

	return RETURN_PASS;
}

/* sys_set_handler
 * 
 * Description: System call for set_handlers, implement for extra credit
 */
int32_t sys_set_handler(int32_t signum, void* handler_address) {
	return RETURN_FAIL;
}

/* sys_sigreturn
 * 
 * Description: System call for sigreturn, implement for extra credit
 */
int32_t sys_sigreturn(void) {
	return RETURN_FAIL;
}

/* sys_vidmap
 * 
 * Description: System call for setting up video memory address for user space
 * Inputs: uint8_t** screen_start -- double pointer to virtual address for video memory
 * Outputs: return a status code
 * Side Effects: Copies pointer kernel space to user space
 */
int32_t sys_vidmap(uint8_t** screen_start) {
	unsigned int screen_start_address = (unsigned int) screen_start; 
	if(screen_start_address < PROGRAM_IMAGE_START_ADDRESS || screen_start_address > PROGRAM_IMAGE_END_ADDRESS)
		return RETURN_FAIL;
	
	*screen_start = (uint8_t *) PROGRAM_IMAGE_END_ADDRESS;											

	// map virtual 0x8400000 -> physical 0xB8000
	page_table_vid[0] = (VIDEO_MEM_FULL_ADDR) | USER_READ_WRITE_PRESENT_ENABLE;			
	unsigned int page_dir = (unsigned int)*screen_start >> BITSHIFT_PAGE_OFFSET;

	// modify page dir
	page_directory[page_dir] = (unsigned int)(page_table_vid);
	page_directory[page_dir] |= USER_READ_WRITE_PRESENT_ENABLE;

	// flush TLB
	asm volatile ("movl %cr3,%eax; movl %eax,%cr3");

	return RETURN_PASS; 
}
