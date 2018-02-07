#ifndef _FS_DIRECTORY_H_
#define _FS_DIRECTORY_H_

#include "type.h"
#include "drivers/fs/dinode.h"
#include "drivers/fs/fs_lib.h"

#define DIRE_FILENAME_SIZE  60

/* Directory entry */
struct directory_entry {
    uint32 dinode_idx; /* index of dinode it points to */
    char *name[DIRE_FILENAME_SIZE];
};
#define DIRE_ENTRY_CNT (FS_BYTES_PER_BLOCK / sizeof(struct directory_entry))

/* Directory block */
struct directory_block {
    struct directory_entry entry[DIRE_ENTRY_CNT];
};

/* Directory related functions */
int32 search_dinode_by_directory(
    const struct dinode *ptr_dinode, 
    const char* name,
    const struct fs_partition_descriptor *ptr_descriptor
);
BOOL insert_directory_entry(
    const struct dinode *ptr_dinode, 
    const char* name,
    int32 dinode_index,
    const struct fs_partition_descriptor *ptr_descriptor
);

#endif