#include "../types.h"

#ifndef _FS_DIR_H
#define _FS_DIR_H

/* 
 * dir_write
 * Would perform a write of buffer data to a file representing a directory
 * on disk. Since it's not needed in this MP, the required behavior 
 * is for the function to...
 * 
 * Return Value: Should always return -1 (failure).
 */
int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes);

/*
 * dir_read
 * Inputs: fd - file descriptor corresponding to file to be read
 *         buf - a pointer to the data buffer that the file data should be read to
 *         nbytes - the size of the buffer to read data into
 * Return Value: The number of bytes read by the function.
 * Side Effects: Overwrites the given buffer.
 */
int32_t dir_read(int32_t fd, void* buf, int32_t nbytes);

/*
 * dir_open
 * Inputs: filename - the name of the directory we want to open
 * Return Value: A file descriptor corresponding to the opened directory file.
 *               If the named file doesn't exist or no descriptors are free, return -1.
 * Side Effects: Allocates a spot in the file descriptor array for the directory if it can.
 *               Will create a structure corresponding to a directory file descriptor in it.
 */
int32_t dir_open(const uint8_t* filename);

/*
 * dir_close
 * Inputs: fd - file descriptor corresponding to the file descriptor array index of the directory
 * Return Value: On a successful close, return 0.
 *               If trying to free an invalid descriptor (stdin/stdout, non-dir descriptors), return -1.
 * Side Effects: Frees up the fd array entry corresponding to the index given.
 */
int32_t dir_close(int32_t fd);

#endif /* _FS_DIR_H */
