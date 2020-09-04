#include "fs.h"

#define NOT_FOUND -1
#define FOUND 0

#define FILENAME_MATCH 0
#define FILENAME_NOT_EQUAL -1

// Global variables
boot_block_t *fs_boot_block;

/*
 * init_fs
 * Inputs: addr - The address of where the boot block starts
 * Return Value: None
 * Side Effects: Sets a variable to the boot block address location
 */
void fs_init(unsigned int addr) {
    fs_boot_block = (boot_block_t *) addr;

    // Data block and inode 0 are reserved
    data_block_bitmap[0] = 1;
    inode_bitmap[0] = 1;

    /* Populate bitmap data for file creation */
    int i, j;
    for(i = 0; i < fs_boot_block->num_dir_entries; i++) {
        dentry_t * dir_entry;
        read_dentry_by_index(i, dir_entry);

        // Mark inode as claimed for dentry, then get data block indices
        // Directory entry must exist for this to occur
        if (dir_entry->file_name[0] != '\0' && dir_entry->inode_num != 0) {
            inode_bitmap[dir_entry->inode_num] = 1;
            inode_t * inode = get_inode_by_idx(dir_entry->inode_num);

            // Get claimed data block indices from inode structure
            for(j = 0; j < MAX_BLOCKS_IN_INODE; j++) {
                // Reached end of claimed blocks
                if (inode->data_blocks_idx[j] == 0)
                    break;

                data_block_bitmap[inode->data_blocks_idx[j]] = 1;
            }
        }
    }
}

/*
 * read_dentry_by_name
 * Inputs: fname - The filename corresponding to the dentry we want to access
 *         dentry - the dentry data structure to populate using dentry data
 * Return Value: If successful, return 0.
 *               If a file doesn't exist with the given name, return -1.
 * Side Effects: None
 */
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry) {
    uint8_t dentry_idx, ch_idx;
    int8_t status_flag = FILENAME_MATCH;  // Used to support file names that exceed 32B

    for(dentry_idx = 0; dentry_idx < fs_boot_block->num_dir_entries; dentry_idx++) {
        // Assume we have a match until proven otherwise
        status_flag = FILENAME_MATCH;
        
        uint8_t *dentry_file_name = fs_boot_block->directory_entries[dentry_idx].file_name;
        
        /* Check each char in the string is the same.
         * Completing this loop means the file name matched up to our max length of 32 chars. */
        for(ch_idx = 0; ch_idx < MAX_FILENAME_LENGTH; ch_idx++) {
            if(fname[ch_idx] != dentry_file_name[ch_idx]) {
                status_flag = FILENAME_NOT_EQUAL;  // Found a mismatch
                break;
            }

            /* Check if we've reached the end of the name early */
            if(fname[ch_idx] == '\0') {
                break;  // Full match! 
            } else if(ch_idx == MAX_FILENAME_LENGTH - 1 && fname[MAX_FILENAME_LENGTH] != '\0') {
                // we matched until 32 chars but fname still has chars
                status_flag = FILENAME_NOT_EQUAL;
                break;
            }
        }
        
        /* If our full match assumption was correct, update dentry. */
        if(status_flag != FILENAME_NOT_EQUAL) {
            memcpy(dentry, &fs_boot_block->directory_entries[dentry_idx], sizeof(dentry_t));
            return FOUND;
        }
    }

    return NOT_FOUND;
}

/*
 * read_dentry_by_index
 * Inputs: index - The index of the dentry we want to access in the boot block
 *         dentry - the dentry data structure to populate using dentry data
 * Return Value: If successful, return 0.
 *               If no dentry exists at the given index, return -1.
 * Side Effects: None
 */
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry) {
    /* Check that the dentry index is within the valid range. */
    uint32_t dir_count = fs_boot_block->num_dir_entries;
    if(index > dir_count - 1 || index < 0) {  // Account for zero indexing
        return NOT_FOUND;
    }
    
    memcpy(dentry, &fs_boot_block->directory_entries[index], sizeof(dentry_t));
    
    return FOUND;
}

/*
 * read_data
 * Inputs: inode - the numeric ID of the inode we want to read data from
 *         offset - the position to start reading data from relative to the beginning of the file data
 *         buf - the buffer to read data into
 *         length - the number of bytes to read into the buffer
 * Return Value: If we've reached the end of the file, return 0.
 *               If no inode exists at the given index, return -1.
 *               Otherwise, return the number of bytes read and placed into the buffer.
 */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length) {
    /* Check that the inode index is within the valid range. */
    uint32_t inode_count = fs_boot_block->num_inodes;
    if(inode > inode_count - 1 || inode < 0) {  // Account for zero indexing
        return NOT_FOUND;
    }

    /* Get a pointer to the inode. Pointer arithmetic works in terms of sizeof(original pointer)
     * so if we add to fs_boot_block, each "byte" we add is actually 4KB (the sizeof(fs_boot_block_t))
     * We want to skip the boot block and go to the inode'th block
     */
    inode_t *inode_ptr = (inode_t *) (fs_boot_block + inode + 1);

    if (offset >= inode_ptr->file_length) {
        return 0;  // EOF, offset is farther than or equal to file length
    }

    /* Determine limit of data to read max(length, file length) */
    uint32_t max_length = length;
    if(inode_ptr->file_length < length + offset) {
        max_length = inode_ptr->file_length - offset;  // new length
    }

    /* Set up local vars for use in data reading and validation */
    uint32_t curr_offset = offset % FS_BLK_SIZE;  // Find starting position within the current data block
    uint32_t curr_idx_offset = offset / FS_BLK_SIZE;
    uint32_t bytes_read = 0;
    uint32_t bytes_remaining = max_length;
    uint32_t inode_data_block_idx = 0;
    uint32_t bytes_to_copy = 0;
    uint32_t datablk_idx = inode_ptr->data_blocks_idx[inode_data_block_idx + curr_idx_offset];  

    uint32_t data_block_count = fs_boot_block->num_data_blocks;
    if (datablk_idx > data_block_count - 1 || datablk_idx < 0) {  // Account for zero indexing
        return -1;
    }

    /* Retrieve file data from data blocks */
    uint8_t *data = (uint8_t *) (fs_boot_block + 1 + inode_count + datablk_idx);
    
    while (bytes_remaining) {

        // copy bytes from offset to end of the block
        bytes_to_copy = FS_BLK_SIZE - curr_offset;
        if(bytes_remaining < bytes_to_copy) {
            // we don't want to copy the entire block, just copy remaining bytes instead
            bytes_to_copy = bytes_remaining;
        }

        // use memcpy because its fast
        memcpy((buf + bytes_read), (data + curr_offset), bytes_to_copy);

        bytes_read += bytes_to_copy;
        bytes_remaining -= bytes_to_copy;
        
        // prepare next block for copying
        datablk_idx = inode_ptr->data_blocks_idx[++inode_data_block_idx];
        data = (uint8_t *) (fs_boot_block + 1 + inode_count + datablk_idx);

        curr_offset = 0;

        /* Validate new block idx */
        if (datablk_idx > data_block_count - 1 || datablk_idx < 0) {  // Account for zero indexing
            return bytes_read;  // Something went wrong, return the bytes we've already read into the buf
        }
    }

    return bytes_read;
}

/*
 * write_data
 * Inputs: inode - the numeric ID of the inode we want to use to index into data blocks
 *         offset - the position to start writing data to relative to the beginning of the file data
 *         buf - the buffer to write data from
 *         length - the number of bytes to read from the buffer
 * Return Value: If no inode exists at the given index, return -1.
 *               Otherwise, return the number of bytes written to the file. This number
 *               may not always be length, as the file system may be unable to claim free blocks.
 * Side Effects: Modifies the file system data block contents.
 */
int32_t write_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length) {
    /* Check that the inode index is within the valid range. */
    uint32_t inode_count = fs_boot_block->num_inodes;
    if(inode > inode_count - 1 || inode < 0) {  // Account for zero indexing
        return NOT_FOUND;
    }

    /* Get a pointer to the inode. Pointer arithmetic works in terms of sizeof(original pointer)
     * so if we add to fs_boot_block, each "byte" we add is actually 4KB (the sizeof(fs_boot_block_t))
     * We want to skip the boot block and go to the inode'th block
     */
    inode_t *inode_ptr = (inode_t *) (fs_boot_block + inode + 1);

    
    uint16_t blocks_curr_claimed = inode_ptr->file_length / 4096 + 1;  // each block is 4kB
    uint32_t max_write_offset = blocks_curr_claimed * 4096;  // Each file can only write in data blocks they own

    // Allocate new data blocks if needed
    if (offset >= inode_ptr->file_length) {
        // Check if we need to claim a new data block to write to this file.
        uint32_t new_length = offset + length;
        uint16_t blocks_required = new_length / 4096 + 1;
        uint16_t block_diff = blocks_required - blocks_curr_claimed;
        uint16_t new_blocks_claimed = 0;
        int i;
        if (block_diff > 0 && blocks_curr_claimed < MAX_BLOCKS_IN_INODE) {
            for (i = 0; i < block_diff; i++) {
                int32_t block_idx = claim_free_data_block();
                if(block_idx == -1) {
                    // Can't perform write, unclaim blocks
                    while (new_blocks_claimed) {
                        int32_t unclaim_idx = inode_ptr->data_blocks_idx[blocks_curr_claimed - new_blocks_claimed];
                        data_block_bitmap[unclaim_idx] = 0;
                        new_blocks_claimed--;
                    }

                    return -1;
                }

                new_blocks_claimed++;

                // Update inode data block indices
                inode_ptr->data_blocks_idx[blocks_curr_claimed] = block_idx;
            }

            max_write_offset += 4096 * new_blocks_claimed;
        }
        
        inode_ptr->file_length = new_length;
    }

    /* We now know we have enough space to perform the write. 
     * Set up local vars for use in data reading and validation */
    uint32_t curr_offset = offset % FS_BLK_SIZE;  // Find starting position within the current data block
    uint32_t curr_idx_offset = offset / FS_BLK_SIZE;
    uint32_t bytes_written = 0;
    uint32_t bytes_remaining = length;
    uint32_t inode_data_block_idx = 0;
    uint32_t bytes_to_copy = 0;
    uint32_t datablk_idx = inode_ptr->data_blocks_idx[inode_data_block_idx + curr_idx_offset];  

    uint32_t data_block_count = fs_boot_block->num_data_blocks;
    if (datablk_idx > data_block_count - 1 || datablk_idx < 0) {  // Account for zero indexing
        return -1;
    }

    /* Retrieve file data from data blocks */
    uint8_t *data = (uint8_t *) (fs_boot_block + 1 + inode_count + datablk_idx);
    
    while (bytes_remaining) {

        // copy bytes from offset to end of the block
        bytes_to_copy = FS_BLK_SIZE - curr_offset;
        if(bytes_remaining < bytes_to_copy) {
            // we don't want to copy the entire block, just copy remaining bytes instead
            bytes_to_copy = bytes_remaining;
        }

        // use memcpy because its fast
        memcpy((data + curr_offset), (buf + bytes_written), bytes_to_copy);

        bytes_written += bytes_to_copy;
        bytes_remaining -= bytes_to_copy;
        
        // prepare next block for copying
        datablk_idx = inode_ptr->data_blocks_idx[++inode_data_block_idx];
        data = (uint8_t *) (fs_boot_block + 1 + inode_count + datablk_idx);

        curr_offset = 0;

        /* Validate new block idx */
        if (datablk_idx > data_block_count - 1 || datablk_idx < 0) {  // Account for zero indexing
            return bytes_written;  // Something went wrong, return the bytes we've already read into the buf
        }
    }

    return bytes_written;
}

/*
 * get_file_size
 * Inputs: entry - the dentry for the file we want to get the size of
 * Return Value: returns length of file if valid dentry, else -1
 */
uint32_t get_file_size(dentry_t* entry) {
    uint32_t inode = entry->inode_num;
    uint32_t inode_count = fs_boot_block->num_inodes;
    if(inode > inode_count - 1 || inode < 0) {  // Account for zero indexing
        return NOT_FOUND;
    }

    inode_t *inode_ptr = (inode_t *) (fs_boot_block + inode + 1);
    return inode_ptr->file_length;
}

/*
 * get_inode_by_idx
 * Inputs: idx - the index of the inode we want to retrieve
 * Return value - a pointer to the inode structure if it exists, else -1
 */
inode_t * get_inode_by_idx(uint32_t idx) {
    if(idx > MAX_INODES - 1 || idx < 0) {  // Account for zero indexing
        return NULL;
    }

    // Skip past first 4 kb of memory, which contains boot block
    return (inode_t *) ((uint32_t) fs_boot_block + idx + 0x1000);
}

/*
 * claim_free_inode_block
 * Return value - the index of the first free data block, or -1 if no blocks are free
 * Side effects: Marks whatever data block was claimed as such
 */
int32_t claim_free_data_block() {
    int i;
    for(i = 0; i < MAX_DATA_BLOCKS; i++) {
        if (data_block_bitmap[i] == 0) {
            data_block_bitmap[i] = 1;
            return i;
        }
    }

    return -1;
}

/*
 * find_free_data_block
 * Return value - the index of the first free inode block, or -1 if no blocks are free
 * Side effects: Marks whatever inode block was claimed as such
 */
int32_t claim_free_inode_block() {
    int i;
    for(i = 0; i < MAX_DATA_BLOCKS; i++) {
        if (inode_bitmap[i] == 0) {
            inode_bitmap[i] = 1;
            return i;
        }
    }

    return -1;
}

/*
 * create_new_file
 * Creates a new directory entry and claims a free inode/data block if possible.
 * 
 * Return value - A pointer to our new dentry, or NULL if we couldn't create one.
 * Side effects: Will update the data block and inode bitmaps, as well as take up directory entry space.
 */
dentry_t * create_new_file(const uint8_t * fname) {
    // Check if there are any free directory entries
    if (fs_boot_block->num_dir_entries >= NUM_DENTRIES) {
        return NULL;
    }

    // Try to claim an inode for our new file
    int32_t inode_idx = claim_free_inode_block();
    if (inode_idx == -1) {
        return NULL;
    }

    // Try to claim a data block for our new file
    int32_t data_block_idx = claim_free_data_block();
    if (data_block_idx == -1) {
        // Free the inode that we claimed
        inode_bitmap[inode_idx] = 0;
        return NULL;
    }

    /* At this point, our dentry and data are considered valid.
     * Populate the dentry with the file name that we got and update metadata. */
    dentry_t * new_dentry = &fs_boot_block->directory_entries[fs_boot_block->num_dir_entries];
    uint8_t valid_chars[MAX_FILENAME_LENGTH];
    memset((void *) valid_chars, 0, MAX_FILENAME_LENGTH);
    int i;
    for (i = 0; i < MAX_FILENAME_LENGTH; i++) {
        if (fname[i] == '\0')
            break;

        valid_chars[i] = fname[i];
    }
    strcpy((int8_t *) new_dentry->file_name, (int8_t *) valid_chars);
    new_dentry->file_type = FS_TYPE_FILE;
    new_dentry->inode_num = inode_idx;

    /* Update inode data block indices */
    inode_t * inode = get_inode_by_idx(inode_idx);
    inode->file_length = 0;
    inode->data_blocks_idx[0] = data_block_idx;

    fs_boot_block->num_dir_entries++;

    return new_dentry;
}
