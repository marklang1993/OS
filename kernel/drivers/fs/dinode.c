#include "dbg.h"
#include "drivers/fs/dinode.h"

/* Get a free dinode */
int32 get_dinode(const struct fs_partition_descriptor *ptr_descriptor)
{
    return -1;
}

/* Free a used dinode */
void put_dinode(
	uint32 index,
	const struct fs_partition_descriptor *ptr_descriptor
)
{
    ;
}

/* Read a dinode on the disk */
BOOL read_dinode(
	uint32 index,
	const struct fs_partition_descriptor *ptr_descriptor,
	struct dinode *out_dinode
)
{
    uint32 block_index = ptr_descriptor->sb.dinode_start + (index / DINODE_SIZE);
	uint32 offset = (index % DINODE_SIZE) * DINODE_SIZE;
	struct fs_data_block data_block;
	byte *ptr_inode_block = (byte *)(&data_block);
	BOOL ret;

	/* Read corresponding dinode block */
	ret = fslib_read_block(
		block_index,
		ptr_descriptor,
		&data_block
	);
	if (IS_TRUE(ret)) {
		/* copy the corresponding dinode */
		memcpy(out_dinode, offset + ptr_inode_block, DINODE_SIZE);
		return TRUE;
	}
	return FALSE;
}

/* Write a dinode to the disk */
BOOL write_dinode(
	uint32 index,
	const struct fs_partition_descriptor *ptr_descriptor,
	const struct dinode *in_dinode
)
{
    return FALSE;
}

/*
 # Read data block pointed by given dinode and given cursor
 * ptr_dinode    : given dinode
 * cur_data_block: data block cursor
 * ptr_descriptor: file system partition descriptor
 * ptr_data_block: data block pointer
 */
BOOL dinode_read_data_block(
	const struct dinode *ptr_dinode,
	uint32 cur_data_block,
	const struct fs_partition_descriptor *ptr_descriptor,
	struct fs_data_block *out_data_block
)
{
	const struct indir_block_ref *ptr_indir_ref;
	uint32 data_block_idx; /* Target data block index */
    BOOL ret;

    if (cur_data_block < DINODE_DIRECT_CNT) {
        /* Read the data block directly */
		data_block_idx = ptr_dinode->block_ref[cur_data_block];

    } else {
        
        /* Find the data block for indirect reference */
        ret = fslib_read_data_block(
            ptr_dinode->block_ref[cur_data_block],
            ptr_descriptor,
            out_data_block /* Borrow the temporary memory space */
        );
		/* Update */
		cur_data_block -= DINODE_DIRECT_CNT;
		ptr_indir_ref = (const struct indir_block_ref *)out_data_block;
        kassert(IS_TRUE(ret));
        
		while(cur_data_block >= DINODE_INREF_CNT) {
			/* 
			 * Currnet indirect reference block is not the target one.
			 * So, continue to find next one
			 */
			ret = fslib_read_data_block(
				ptr_indir_ref->next_ref,
				ptr_descriptor,
				out_data_block
			);
			/* Update */
			cur_data_block -= DINODE_INREF_CNT; /* Update cursor */
			ptr_indir_ref = (const struct indir_block_ref *)out_data_block;
        	kassert(IS_TRUE(ret));
		}

		/* Reach the target one */
		data_block_idx = ptr_indir_ref->block_ref[cur_data_block];
    }

	/* Read data block */
	return fslib_read_data_block(
		data_block_idx,
		ptr_descriptor,
		out_data_block
	);
}