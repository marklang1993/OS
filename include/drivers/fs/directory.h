#ifndef _FS_DIRECTORY_H_
#define _FS_DIRECTORY_H_

#include "type.h"
#include "drivers/fs/dinode.h"
#include "drivers/fs/fs_lib.h"

#define DIR_NAME_SIZE  60

/* Directory entry */
struct directory_entry {
    uint32 dinode_idx; /* index of dinode it points to */
    char name[DIR_NAME_SIZE];
};
#define DIR_ENTRY_CNT (FS_BYTES_PER_BLOCK / sizeof(struct directory_entry))

/* Directory block */
struct directory_block {
    struct directory_entry entry[DIR_ENTRY_CNT];
};

/* Directory related functions */
int32 search_dinode_by_directory(
    const struct dinode *ptr_dinode, 
    const char* name,
    const struct fs_partition_descriptor *ptr_descriptor
);

/* Insert directory entry */
BOOL insert_directory_entry(
    uint32 parent_dinode_index,
    struct dinode *parent_dinode, 
    const char* dir_name,
    uint32 dir_dinode_index,
    const struct fs_partition_descriptor *ptr_descriptor
);

#endif