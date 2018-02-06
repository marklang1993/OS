#include "dbg.h"
#include "drivers/fs/directory.h"

/*
 # Search dinode by directory name
 * ptr_dinode    : dinode that is being searched
 * name          : directory name
 * ptr_descriptor: file system partition descriptor
 @ RETURN        : next dinode index, -1 when error
 */
int32 search_dinode_by_directory(
    const struct dinode *ptr_dinode, 
    const char *name,
    const struct fs_partition_descriptor *ptr_descriptor
)
{
    struct fs_data_block data_block;
    struct directory_block *ptr_dire_block;
    uint32 cnt_directory_entry; /* Total count of directory entries */
    uint32 cur_data_block = 0; /* data block cursor */

    int32 i, j;
    BOOL ret;

    /* Check type */
    if (ptr_dinode->type != DINODE_DIRECTORY) {
        panic("DINODE IS NOT DIRECTORY\n");
    }

    /* Calculate parameters */
    cnt_directory_entry = ptr_dinode->size / sizeof(struct directory_entry);

    /* Search */
    do {
        /* Read data block as directory block */
        ret = dinode_read_data_block(
            ptr_dinode,
            cur_data_block,
            ptr_descriptor,
            &data_block
        );
        kassert(IS_TRUE(ret));
        ptr_dire_block = (struct directory_block *)&data_block;

        /* Search in this block */
        for (j = 0; j < DIRE_ENTRY_CNT; ++j)
        {
            if (strcmp(ptr_dire_block->entry[j].name, name) == 0) {
                /* Found */
                return (int32)ptr_dire_block->entry[j].dinode_idx;
            }
            ++i; /* Update cnt_directory_entry */

            if (i >= cnt_directory_entry) break; /* if reach end, break */
        }
        ++cur_data_block; /* Update data block cursor */

    } while (i < cnt_directory_entry);

    /* Does not find out */
    return -1;
}
