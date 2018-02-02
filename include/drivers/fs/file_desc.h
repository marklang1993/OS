#ifndef _FS_FILE_DESC_H_
#define _FS_FILE_DESC_H_

#include "drivers/fs/inode.h"

#define FILE_TABLE_SIZE     64

/* Preserved file table entry */
#define FILE_STDIN          0
#define FILE_STDOUT         1
#define FILE_STDERR         2
#define FILE_OTHER_BASE     3   /* other file operatiions start from 3 */

/* File Status */
typedef uint32 FILE_STATUS;
#define FILE_STATUS_NULL    0 /* Empty OR Invalid entry */
#define FILE_STATUS_READ    1
#define FILE_STATUS_WRITE   2

/* File Table Entry */ 
struct file_table_entry {
    FILE_STATUS status;         /* File status */
    uint32 offset;              /* Current offset of operation */
    struct inode *inode_ptr;    /* Pointer to inode */
};

#endif