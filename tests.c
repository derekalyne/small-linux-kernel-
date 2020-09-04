#include "tests.h"
#include "x86_desc.h"
#include "lib.h"

#include "devices/rtc.h"
#include "devices/terminal.h"
#include "paging/page_structs.h"
#include "fs/file.h"
#include "fs/directory.h"
#include "fs/fs.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}

/* Checkpoint 1 tests */

/* divide_zero_error_test
 *
 * Checks if kernel is able to perform a divide by zero exception
 * Inputs: None
 * Outputs: NONE
 * Side Effects: creates a divide by zero exception
 * Files: idt.c/x86_desc.h
 */
int divide_zero_error_test() {
	TEST_HEADER;
	int error;
	int a = 1;
	int b = 0;
	error = a/b;
	return FAIL;
}

/* IDT Test - Example
 *
 * Asserts that first 20 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 20; ++i){
		if ((idt[i].offset_15_00 == NULL) &&
			(idt[i].offset_31_16 == NULL) && i!=15) {
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

/* null_deref_test
 *
 * Checks if kernel throws an exception when trying to deference null
 * Inputs: None
 * Outputs: NONE
 * Side Effects: creates a page fault exception
 * Files: idt.c/x86_desc.h
 */
int null_deref_test(){
	TEST_HEADER;
	int* a = NULL;
	int b;
    b = *(a);
	return FAIL;
}

/* Access Valid Page Test
 *
 * Asserts that we can cleanly access the memory contained from 4 MB - 8 MB, as well
 * as the video memory at 0x000B8000 - 0x000B9000
 * Inputs: None
 * Outputs: PASS
 * Side Effects: May cause an exception, which would kill the kernel
 * Coverage: Ability to correctly access allocated pages
 * Files: paging.h/S
 */
int access_valid_page_test() {
	TEST_HEADER;
	uint32_t * kernel_ptr = (uint32_t *) 0x00400001;
	uint32_t tmp = *kernel_ptr;  // Attempt to dereference

	uint32_t * video_ptr = (uint32_t *) 0x000B8001;
	tmp = *video_ptr;

	int result = PASS;
	return result;
}

/* Access Non-Present Memory Test
 *
 * Asserts that we throw a page fault when memory marked not present is accessed.
 * Inputs: None
 * Outputs: FAIL
 * Side Effects: Should cause an exception to occur, killing the kernel
 * Coverage: Ability to detect access of non-allocated pages
 * Files: paging.h/S
 */
int cannot_access_nonpresent_memory_test() {
	TEST_HEADER;

	// Check address just outside of kernel memory
	uint32_t * bunk_ptr = (uint32_t *) 0x00800001;
	uint32_t tmp;
	tmp = *bunk_ptr;
	return FAIL;
}

/* Validate Page Directory and Page Table Contents
 *
 * Asserts that the page directory contains
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Correct initialization of page dir and table
 * Files: page_structs.h/C
 */
int validate_page_struct_contents() {
	// All entries will have their dirty and accessed bits enabled by the processor, so account for
	// that when checking values

	// Check if kernel page is correctly referenced in PDE
	if(page_directory[1] != 0x004001F3) {
		return FAIL;
	}

	// Check if entry for page table is present
	if(!(page_directory[1] & 0x1)) {
		return FAIL;
	}

	// Check if video memory PTE is correctly set
	if(page_table[184] != 0x000B8063) {
		return FAIL;
	}

	return PASS;
}

/* Checkpoint 2 tests */

/*
read file with long name / all cases (verylargetextwithverylongname.t should not work, verylargetextwithverylongname.tx should work, verylargetextwithverylongname.txt should not work)
read small file (frame0.txt)
read large file (verylargetextwithverylongname.tx)
read executable (grep)
test lookup non-existent file
test lookup for invalid inode index
test read w/ invalid inode number
test different ways of read like reading across blocks, reading past end of file etc.
test everything in directory.c
test everything in file.c
*/

int fs_read_file_with_large_name_test() {
	TEST_HEADER;
	dentry_t ret;

	char *buf = "verylargetextwithverylongname.t";
	// this should not work
	if(read_dentry_by_name((uint8_t *) buf, &ret) == 0) {
		return FAIL;
	}

	buf = "verylargetextwithverylongname.txt";
	// this should also not work
	if(read_dentry_by_name((uint8_t *) buf, &ret) == 0) {
		return FAIL;
	}

	buf = "verylargetextwithverylongname.tx";
	// this should work
	if(read_dentry_by_name((uint8_t *) buf, &ret) != 0) {
		return FAIL;
	}

	return PASS;
}

int fs_read_file_with_standard_file_test() {
	TEST_HEADER;
	dentry_t ret;
	char *buf = "ls";
	if(read_dentry_by_name((uint8_t *) buf, &ret) != 0) {
		return FAIL;
	}

	return PASS;
}

int fs_read_dentry_non_existent_file_test() {
	TEST_HEADER;
	dentry_t ret;
	char *buf = "thisfiledoesnotexist";
	// this should not work
	if(read_dentry_by_name((uint8_t *) buf, &ret) == 0)
		return FAIL;
	return PASS;
}

int fs_read_dentry_invalid_dentry_test() {
	TEST_HEADER;
	dentry_t ret;
	// first test a bad idx
	// upper bound
	if(read_dentry_by_index(100, &ret) == 0)
		return FAIL;
	// lower bound
	if(read_dentry_by_index(-1, &ret) == 0)
		return FAIL;
	// test good idx
	if(read_dentry_by_index(1, &ret) == -1)
		return FAIL;
	return PASS;
}

int fs_read_data_invalid_inode_test() {
	TEST_HEADER;
	char buf[10];
	uint32_t offset = 0;
	// first test a bad inode number
	// upper bound
	if(read_data(100, offset, (uint8_t *) buf, 10) == 0)
		return FAIL;
	// lower bound
	if(read_data(-1, offset, (uint8_t *) buf, 10) == 0)
		return FAIL;
	// test good inode number
	if(read_data(1, offset, (uint8_t *) buf, 10) == 0)
		return PASS;
	return FAIL;
}

int fs_read_data_test() {
	TEST_HEADER;
	dentry_t dentry;
	char* filename = "verylargetextwithverylongname.tx";
	read_dentry_by_name((uint8_t *) filename, &dentry);
	char buf[5277];

	// Read the entire file
	if(read_data(dentry.inode_num, 0, (uint8_t *) buf, 5277) != 5277)
		return FAIL;
	// Read part of file with offset
	if(read_data(dentry.inode_num, 100, (uint8_t *) buf, 100) != 100)
		return FAIL;
	// Read file data across blocks
	if(read_data(dentry.inode_num, 5000, (uint8_t *) buf, 100) != 100)
		return FAIL;
	// Read exactly up to end of file
	if(read_data(dentry.inode_num, 5177, (uint8_t *) buf, 100) != 100)
		return FAIL;
	// Read over the end of the file
	if(read_data(dentry.inode_num, 5200, (uint8_t *) buf, 100) != 77)
		return FAIL;
	// Read with offset greater than file size
	if(read_data(dentry.inode_num, 5300, (uint8_t *) buf, 100) != 0)
		return FAIL;

	return PASS;
}

int fs_read_small_file_test() {
	int32_t fd = file_open((uint8_t *) "frame1.txt");
	if(fd == -1) {
		return FAIL;
	}

	/* Read file */
	uint8_t buf[301];
	int32_t bytes_read = file_read(fd, (void *) buf, 300);
	if(bytes_read == 0) {
		return FAIL;
	}
	buf[bytes_read] = '\0';
	printf("%s", buf);

	int32_t status = file_close(fd);
	if(status != 0) {
		return FAIL;
	}

	return PASS;
}

int fs_read_large_file_test() {
	// file_read verylargetextwithverylongname.tx
	// should be 5277 bytes, putc each byte
	return PASS;
}

int fs_read_executable_test() {
	// file_read grep
	// idk how many bytes
	return PASS;
}

int fs_file_open_close_test() {
	TEST_HEADER;
	int32_t fd = file_open((uint8_t *) "ls");
	if(fd == -1) {
		return FAIL;
	}

	int32_t status = file_close(fd);
	if(status != 0) {
		return FAIL;
	}

	return PASS;
}

int fs_file_open_invalid_file_test() {
	TEST_HEADER;
	int32_t fd = file_open((uint8_t *) ".");
	if(fd == -1) {
		return PASS;
	}

	file_close(fd);
	return FAIL;
}

int fs_read_directory_test() {
	TEST_HEADER;
	uint8_t * dir_name = (uint8_t *) ".";
	uint8_t i, successful_reads = 0;
	int32_t fd = dir_open(dir_name);
	if(fd == -1) {
		return FAIL;
	}

	uint8_t buf[34];
	for(i = 0; i < 63; i++) {
		int32_t bytes_read = dir_read(fd, (void*) buf, 32);
		buf[bytes_read] = '\n';
		buf[bytes_read + 1] = '\0';
		if(bytes_read != 0) {
			successful_reads++;
		}
	}

	int32_t status = dir_close(fd);
	// Should be 16 files/directories in the file system (1 RTC file for total of 17 files)
	if(status == -1 || successful_reads != 16) {
		return FAIL;
	}
	return PASS;
}

// Attempt to read file using dir_open, should not be able to do this
int fs_open_invalid_dir_test() {
	TEST_HEADER;
	int32_t fd = dir_open((uint8_t *) "frame0.txt");
	if(fd == -1) {
		return PASS;
	}

	dir_close(fd);
	return FAIL;
}

int test_terminal(){
	int idx;
	int to_write;
	char arr[2];
	arr[0] = 'a';
	arr[1] = 'b';
	//Buffer that I will write to screen
	char basic_test_write[2300];
	char read_buffer_test[2300];
 	char new_line[1];
	new_line[0] = '\n';
	terminal_open(0);

	//Filling up the buffer with random letters
	for(idx = 0; idx < 2300; idx++){
		if(idx > 2000){
			basic_test_write[idx] = 'a';
		}
		if(idx > 10){
			basic_test_write[idx] = 'b';
		} else{
			basic_test_write[idx] = 'f';
		}
	}

	//This is a simple test to write to the screen with the same n count
  //terminal_write(0, basic_test_write, 2300);
	//This is a simple test to write to the screen with a negative n count
  //terminal_write(0, basic_test_write, -5);
	//This is a simple test to write to screen with a lower n count
  //terminal_write(0, basic_test_write, 5);
	//performing read and write to and from terminal
  //terminal_read(0, read_buffer_test, 5);
  //terminal_write(0, read_buffer_test, 5);
  while(1){
		for(idx = 0; idx < 2300; idx++){
			read_buffer_test[idx] = '\0';
			basic_test_write[idx] = '\0';
		}
		to_write = 0;
		read_buffer_test[0] = 'E';
		read_buffer_test[1] = 'C';
		read_buffer_test[2] = 'E';
		read_buffer_test[3] = '3';
		read_buffer_test[4] = '9';
		read_buffer_test[5] = '1';
		read_buffer_test[6] = '>';
		terminal_write(0, read_buffer_test, 7);
  	to_write = terminal_read(0, basic_test_write, CHAR_BUFFER_SIZE);
		terminal_write(0, basic_test_write, to_write);
		terminal_write(0, new_line, 1);
  }
  //test for closing terminals
  //terminal_close(0);
	//return PASS;
}
int rtc_open_write_read_close_test() {
	TEST_HEADER;
	clear();
	// no need for filename parameter
	// verify that frequency is 2
	rtc_open(NULL);
	uint32_t MAX_FREQ = 512;
	uint32_t START_FREQ = 1;
	// how many seconds do we want to stay on each frequency for?
	uint8_t num_seconds_wait = 1;
	uint32_t current_freq, num_interrupts_received;
	for(current_freq = START_FREQ; current_freq <= MAX_FREQ; current_freq *= 2) {
		clear();
		void *ptr = &current_freq;
		// parameters fd and nbytes not needed
		rtc_write(0, ptr, 0);
		for(num_interrupts_received = 0; num_interrupts_received < current_freq * num_seconds_wait; num_interrupts_received++) {
			// we don't care about any of the parameters
			rtc_read(0, NULL, 0);
		}
	}
	putc('\n');
	// we don't care about the parameter
	rtc_close(0);
	return PASS;
}

// int rtc_open_resets_frequency_to_2Hz() {
// 	TEST_HEADER;
// 	// don't care about parameter
// 	rtc_open(NULL);
// 	// freq should be set to two
// 	if(read_rtc_frequency() == 2)
// 		return PASS;
// 	// don't care
// 	rtc_close(0);
// 	return FAIL;
// }

/*int rtc_write_accepts_appropriate_values() {
	TEST_HEADER;
	// don't care about parameter
	rtc_open(NULL);
	// not a power of two
	uint32_t freq = 14;
	// we don't care about other parameters
	if(rtc_write(0, &freq, 0) == 0)
		return FAIL;
	// outside of range that users are allowed to set RTC to run at
	freq = 2048;
	// we don't care about other parameters
	if(rtc_write(0, &freq, 0) == 0)
		return FAIL;
	// good value for freq
	freq = 32;
	if(rtc_write(0, &freq, 0) == -1)
		return FAIL;
	// don't care
	rtc_close(0);
	return PASS;
}*/

/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */

/* Test suite entry point */
void launch_tests() {
	// For CP 1
	// Should all PASS
	//TEST_OUTPUT("idt_test", idt_test());
	//TEST_OUTPUT("access_valid_page_test", access_valid_page_test());
	//TEST_OUTPUT("validate_page_struct_contents", validate_page_struct_contents());

	// Should cause exceptions
	TEST_OUTPUT("divide_zero_error_test", divide_zero_error_test());
	// TEST_OUTPUT("null_deref_test", null_deref_test());
	// TEST_OUTPUT("cannot_access_nonpresent_memory_test", cannot_access_nonpresent_memory_test());

	// For CP 2
	//TEST_OUTPUT("change_rtc_freq_test", change_rtc_freq_test());
	//TEST_OUTPUT("Test reading files with a long name", fs_read_file_with_large_name_test());
	//TEST_OUTPUT("Test accessing dentry with bad index", fs_read_dentry_invalid_dentry_test());
	//TEST_OUTPUT("Test accessing inode with bad index", fs_read_data_invalid_inode_test());
	//TEST_OUTPUT("Test read_data", fs_read_data_test());
	// TEST_OUTPUT("fs_read_directory_test", fs_read_directory_test());
	// TEST_OUTPUT("fs_open_invalid_dir_test", fs_open_invalid_dir_test());
	// TEST_OUTPUT("fs_read_file_nonexistent_file_test", fs_read_file_nonexistent_file_test());
	// TEST_OUTPUT("fs_read_file_with_standard_file_test", fs_read_file_with_standard_file_test());
	// TEST_OUTPUT("fs_read_small_file_test", fs_read_small_file_test());
	// TEST_OUTPUT("fs_file_open_close_test", fs_file_open_close_test());
	// TEST_OUTPUT("fs_file_open_invalid_file_test", fs_file_open_invalid_file_test());
	// test_terminal();
}
