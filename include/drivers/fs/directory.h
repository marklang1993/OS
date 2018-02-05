#ifndef _FS_DIRECTORY_H_
#define _FS_DIRECTORY_H_

#include "type.h"

#define DIRE_FILENAME_SIZE  60

/* Directory entry */
struct directory_entry {
    uint32 dinode_idx; /* index of dinode it points to */
    char *name[DIRE_FILENAME_SIZE];
};

#endif