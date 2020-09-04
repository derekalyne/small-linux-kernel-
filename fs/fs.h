#include "../types.h"
#include "../lib.h"

#ifndef _FS_H
#define _FS_H

#define FS_BLK_SIZE 4096

#define FS_INODE_COUNT_OFFSET 0x04
#define FS_DATBLK_COUNT_OFFSET 0x04
#define FS_DIR_ENTRY_OFFSET 0x40
#define FS_BOOT_BLOCK_RESERVED_SIZE 13

#define INODE_DATBLK_OFFSET 0x04

#define DENTRY_SIZE 0x40
#define DENTRY_FILETYPE_OFFSET 0x20
#define DENTRY_INODENUM_OFFSET 0x24
#define DENTRY_RESERVED_SIZE 6

#define MAX_FILENAME_LENGTH 32
#define MAX_BLOCKS_IN_INODE 1023
#define MAX_FILE_SIZE 5277
#define NUM_DENTRIES 63
#define MAX_INODES 64
#define MAX_DATA_BLOCKS 59

#define FS_TYPE_RTC 0
#define FS_TYPE_DIR 1
#define FS_TYPE_FILE 2

typedef struct inode {
    uint32_t file_length;
    uint32_t data_blocks_idx[MAX_BLOCKS_IN_INODE];
} inode_t;

typedef struct dentry {
    uint8_t file_name[MAX_FILENAME_LENGTH];
    uint32_t file_type;
    uint32_t inode_num;
    uint32_t reserved[DENTRY_RESERVED_SIZE];
} dentry_t;

typedef struct boot_block {
    uint32_t num_dir_entries;
    uint32_t num_inodes;
    uint32_t num_data_blocks;
    uint32_t reserved[FS_BOOT_BLOCK_RESERVED_SIZE];
    dentry_t directory_entries[NUM_DENTRIES];
} boot_block_t;

// Bitmaps for checking if a particular inode/data block is in use
uint8_t data_block_bitmap[MAX_DATA_BLOCKS];
uint8_t inode_bitmap[MAX_INODES];

/*
 * init_fs
 * Inputs: addr - The address of where the boot block starts
 * Return Value: None
 * Side Effects: Sets a variable to the boot block address location
 */
void fs_init(unsigned int addr);

/*
 * read_dentry_by_name
 * Inputs: fname - The filename corresponding to the dentry we want to access
 *         dentry - the dentry data structure to populate using dentry data
 * Return Value: If successful, return 0.
 *               If a file doesn't exist with the given name, return -1.
 * Side Effects: None
 */
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);

/*
 * read_dentry_by_index
 * Inputs: index - The index of the dentry we want to access in the boot block
 *         dentry - the dentry data structure to populate using dentry data
 * Return Value: If successful, return 0.
 *               If no dentry exists at the given index, return -1.
 * Side Effects: None
 */
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);

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
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

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
int32_t write_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

/*
 * get_file_size
 * Inputs: entry - the dentry for the file we want to get the size of
 * Return Value: returns length of file if valid dentry, else -1
 */
uint32_t get_file_size(dentry_t* entry);

/*
 * get_inode_by_idx
 * Inputs: idx - the index of the inode we want to retrieve
 * Return value - a pointer to the inode structure if it exists, else -1
 */
inode_t * get_inode_by_idx(uint32_t idx);

/*
 * claim_free_data_block
 * Return value - the index of the first free data block, or -1 if no blocks are free
 */
int32_t claim_free_data_block();

/*
 * claim_free_inode_block
 * Return value - the index of the first free inode block, or -1 if no blocks are free
 */
int32_t claim_free_inode_block();

/*
 * create_new_file
 * Creates a new directory entry and claims a free inode/data block if possible.
 * 
 * Return value - A pointer to our new dentry, or NULL if we couldn't create one.
 * Side effects: Will update the data block and inode bitmaps, as well as take up directory entry space.
 */
dentry_t * create_new_file(const uint8_t * fname);

#endif  /* _FS_H */
