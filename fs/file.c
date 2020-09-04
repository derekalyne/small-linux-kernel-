#include "file.h"
#include "fs.h"
#include "../interrupts/syscall_structs.h"

/* 
 * file_write
 * Perform a write of buffer data to a file on disk.
 * Inputs: fd - file descriptor corresponding to file to be written to
 *         buf - a pointer to the data buffer to be written to the file system
 *         nbytes - the number of bytes we want to write
 * 
 * Return Value: The number of bytes written by the function
 * Side Effects: Modifies the file system data contents.
 */
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes) {
    file_array_t * fda = PCB[current_process_pid]->file;
    int32_t bytes_written = write_data(fda[fd].inode, fda[fd].file_position, (uint8_t *) buf, nbytes);
    fda[fd].file_position += bytes_written;
    return bytes_written;
}

/*
 * file_read
 * Inputs: fd - file descriptor corresponding to file to be read
 *         buf - a pointer to the data buffer that the file data should be read to
 *         nbytes - the size of the buffer to read data into
 * Return Value: The number of bytes read by the function.
 * Side Effects: Overwrites the given buffer.
 */
int32_t file_read(int32_t fd, void* buf, int32_t nbytes) {
    // get right file descriptor array
    file_array_t * fda = PCB[current_process_pid]->file;
    int32_t bytes_read = read_data(fda[fd].inode, fda[fd].file_position, (uint8_t *) buf, nbytes);
    // update file pos data
    fda[fd].file_position += bytes_read;
    return bytes_read;
}

/*
 * file_open
 * Inputs: filename - the name of the file we want to open
 * Return Value: A file descriptor corresponding to the opened file.
 *               If the named file doesn't exist or no descriptors are free, return -1.
 * Side Effects: Allocates a spot in the file descriptor array for the file if it can.
 *               Will create a structure corresponding to a 'file' file descriptor in it.
 */
int32_t file_open(const uint8_t* filename) {
    // Validate dentry corresponding to filename
    dentry_t file_dentry;
    int32_t status = read_dentry_by_name(filename, &file_dentry);
    if(status != 0 || file_dentry.file_type != FS_TYPE_FILE) {
        return -1;
    }

    return 0;
}

/*
 * dir_close
 * Inputs: fd - file descriptor corresponding to the file descriptor array index of the file
 * Return Value: On a successful close, return 0.
 *               If trying to free an invalid descriptor (stdin/stdout, non-dir descriptors), return -1.
 * Side Effects: Frees up the fd array entry corresponding to the index given.
 */
int32_t file_close(int32_t fd) {
    // Everything is handled by the system here - nothing to clean up.
    return 0;
}
