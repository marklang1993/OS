#include "dbg.h"
#include "drivers/fs/directory.h"

/*
 # Search dinode by directory name
 * ptr_dinode    : directory dinode that is being searched
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
    uint32 data_block_index;
    struct fs_data_block data_block;
    struct directory_block *ptr_dire_block;
    uint32 cnt_directory_entry; /* Total count of directory entries */
    uint32 cur_data_block = 0; /* data block cursor */

    int32 i, j;
    BOOL ret;

    /* Check type */
    if (ptr_dinode->type != DINODE_DIRECTORY) {
        panic("SEARCH: DINODE IS NOT DIRECTORY\n");
    }

    /* Calculate parameters */
    cnt_directory_entry = ptr_dinode->size / sizeof(struct directory_entry);

    /* printk("cnt_directory_entry = %d\n", cnt_directory_entry); */

    /* Search */
    while (i < cnt_directory_entry) {
        /* printk("DINODE READ DATA BLOCK AT %d\n", cur_data_block); */
        
        /* Read data block as directory block */
        ret = dinode_read_data_block(
            ptr_dinode,
            cur_data_block,
            ptr_descriptor,
            &data_block_index,
            &data_block
        );
        kassert(IS_TRUE(ret));
        ptr_dire_block = (struct directory_block *)&data_block;

        /* Search in this block */
        for (j = 0; j < DIR_ENTRY_CNT; ++j)
        {
            if (strcmp(ptr_dire_block->entry[j].name, name) == 0) {
                /* Found */
                return (int32)ptr_dire_block->entry[j].dinode_idx;
            }
            ++i; /* Update cnt_directory_entry */

            if (i >= cnt_directory_entry) break; /* if reach end, break */
        }
        ++cur_data_block; /* Update data block cursor */

    }

    /* Does not find out */
    return -1;
}

/*
 # Insert an directory entry to the given directory dinode
 * parent_dinode_index: index of parent directory dinode
 * parent_dinode      : parent directory dinode where the new directory entry is inserted
 * dir_name           : directory entry name (directory OR file) name
 * dir_dinode_index   : index of directory (directory OR file) dinode
 * ptr_descriptor     : file system partition descriptor
 */
BOOL insert_directory_entry(
    uint32 parent_dinode_index,
    struct dinode *parent_dinode, 
    const char* dir_name,
    uint32 dir_dinode_index,
    const struct fs_partition_descriptor *ptr_descriptor
)
{
    struct directory_entry new_dir_entry;
    struct directory_block *ptr_dir_block;
    uint32 data_block_pos;
    uint32 data_block_offset;
    /* New data block */
    struct fs_data_block new_data_block;
    int32 new_data_block_index;

    int32 i;
    BOOL ret;

    /* Check type */
    if (parent_dinode->type != DINODE_DIRECTORY) {
        panic("INSERT: DINODE IS NOT DIRECTORY\n");
    }

    /* Init. Directory Entry */
    new_dir_entry.dinode_idx = dir_dinode_index;
    if ((strlen(dir_name) + 1) > DIR_NAME_SIZE) {
        panic("FILE / DIRECTORY NAME IS TOO LONG - %d\n", strlen(dir_name));
    }
    strcpy(&new_dir_entry.name, dir_name);

    /* Do some calculations */
    data_block_pos = parent_dinode->size / FS_BYTES_PER_BLOCK;
    data_block_offset = parent_dinode->size % FS_BYTES_PER_BLOCK;


    printk("data_block_pos = %d; data_block_offset = %d\n", data_block_pos, data_block_offset);
    /* Determine is required to claim a new data block */
    if (data_block_offset == 0) {
        /* 
         * New data block is required, due to
         * 1. this dinode takes 0 data block
         * 2. current data block is full
         */
        new_data_block_index = fslib_get_data_block(ptr_descriptor);
        kassert(new_data_block_index >= 0);
        
        /* Init. & Copy the dir. entry to the new data block */
        memset(&new_data_block, 0, sizeof(struct fs_data_block));
        memcpy(&new_data_block, &new_dir_entry, sizeof(struct directory_entry));

        ret = dinode_insert_data_block(
            parent_dinode_index,
            parent_dinode,
            ptr_descriptor,
            new_data_block_index,
            &new_data_block
        );
        kassert(IS_TRUE(ret));

    } else {
        /* Can write to the current data block */
        data_block_offset = data_block_offset % sizeof(struct directory_entry);
        kassert(data_block_offset != 0);

        ret = dinode_read_data_block(
            parent_dinode,
            data_block_pos,
            ptr_descriptor,
            &new_data_block_index,
            &new_data_block
        );
        kassert(new_data_block_index >= 0);
        kassert(IS_TRUE(ret));

        ptr_dir_block = (struct directory_block *)&new_data_block;
        memcpy(&(ptr_dir_block[data_block_offset]), &new_dir_entry, sizeof(struct directory_entry));
        /* Update data block */
        ret = fslib_write_data_block(
            new_data_block_index,
            ptr_descriptor,
            &new_data_block
        );
        kassert(IS_TRUE(ret));
    }
    return TRUE;
}
