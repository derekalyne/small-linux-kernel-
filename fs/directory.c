#include "directory.h"
#include "fs.h"
#include "../interrupts/syscall_structs.h"

/* 
 * dir_write
 * Would perform a write of buffer data to a file representing a directory
 * on disk. Since it's not needed in this MP, the required behavior 
 * is for the function to...
 * 
 * Return Value: Should always return -1 (failure).
 */
int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes) {
    return -1;
}

/*
 * dir_read
 * Inputs: fd - file descriptor corresponding to file to be read
 *         buf - a pointer to the data buffer that the file data should be read to
 *         nbytes - the size of the buffer to read data into
 * Return Value: The number of bytes read by the function.
 * Side Effects: Overwrites the given buffer.
 */
int32_t dir_read(int32_t fd, void* buf, int32_t nbytes) {
    file_array_t * fda = &PCB[current_process_pid]->file[fd];

    dentry_t dentry;
    int status = read_dentry_by_index(fda->file_position, &dentry);
    if(status != 0) {
        fda->file_position++;
        return 0;
    }

    uint8_t * casted_buf;
    casted_buf = (uint8_t *) buf;  

    /* Get file name read limit */
    int32_t max_length = MAX_FILENAME_LENGTH;
    if (nbytes < MAX_FILENAME_LENGTH) {
        max_length = nbytes;
    }

    /* Copy data from file name to buffer */
    uint8_t i, bytes_read = 0;
    for (i = 0; i < MAX_FILENAME_LENGTH; i++) {
        casted_buf[i] = dentry.file_name[i];

        /* Return early on null termination */
        if(dentry.file_name[i] == '\0') {
            fda->file_position++;
            return bytes_read;
        }

        bytes_read++;
    }

    fda->file_position++;
    return bytes_read;
}

/*
 * dir_open
 * Inputs: filename - the name of the directory we want to open
 * Return Value: A file descriptor corresponding to the opened directory file.
 *               If the named file doesn't exist or no descriptors are free, return -1.
 * Side Effects: Allocates a spot in the file descriptor array for the directory if it can.
 *               Will create a structure corresponding to a directory file descriptor in it.
 */
int32_t dir_open(const uint8_t* filename) {
    // Make sure filename we want to open is of type "directory".
    dentry_t dentry;
    int status = read_dentry_by_name(filename, &dentry);
    if(dentry.file_type != FS_TYPE_DIR || status != 0) {
        return -1;
    }
    return 0;
}

/*
 * dir_close
 * Inputs: fd - file descriptor corresponding to the file descriptor array index of the directory
 * Return Value: On a successful close, return 0.
 *               If trying to free an invalid descriptor (stdin/stdout, non-dir descriptors), return -1.
 * Side Effects: Frees up the fd array entry corresponding to the index given.
 */
int32_t dir_close(int32_t fd) {
    // Everything is handled by the system here - nothing to clean up.
    return 0;
}
