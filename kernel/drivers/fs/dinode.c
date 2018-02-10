#include "dbg.h"
#include "drivers/fs/dinode.h"

/* 
 # Initialize a dinode
 * ptr_dinode: pointer to a dinode struct
 */
void init_dinode(struct dinode *ptr_dinode)
{
	int i;

	ptr_dinode->type = DINODE_INVALID;
	ptr_dinode->link_cnt = 0;
	ptr_dinode->major_dev = 0;
	ptr_dinode->minor_dev = 0;
	ptr_dinode->size = 0;

	for(i = 0; i < DINODE_REF_CNT; ++i)
	{
		/* Set all block reference to REF_NULL */
		ptr_dinode->block_ref[i] = DINODE_REF_NULL;
	}
}

/*
 # Get a free dinode
 * ptr_descriptor: file system partition descriptor
 @ RETURN        : index of free dinode
 */
int32 get_dinode(const struct fs_partition_descriptor *ptr_descriptor)
{
    struct fs_data_block dinode_map_block;
	BOOL ret;
	uint32 i;
	int32 empty_block_index;

	/* Traverse all dinode map blocks */
	for (i = 0; i < ptr_descriptor->sb.dinode_map_cnt; ++i)
	{
		/* Read an inode map block */
		ret = fslib_read_block(
			ptr_descriptor->sb.dinode_map_start + i,
			ptr_descriptor,
			&dinode_map_block
		);
		kassert(IS_TRUE(ret));

		/* Find an empty dinode by using current dinode bitmap block */
		empty_block_index = fslib_find_empty_bitmap_block(&dinode_map_block);
		if (empty_block_index >= 0) {
			/* Find an empty block position */
			fslib_set_bitmap_block(
				empty_block_index,
				TRUE,
				&dinode_map_block
			); /* Mark that dinode in current dinode bitmap block */
			ret = fslib_write_block(
				ptr_descriptor->sb.dinode_map_start + i,
				ptr_descriptor,
				&dinode_map_block
			);
			kassert(IS_TRUE(ret));
			/* Calculate the absolute dinode index */
			empty_block_index += i * FS_CNT_BIT_PER_BLOCK;
			return empty_block_index;
		}
	}
	/* No free dinode is found */
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

/*
 # Read a dinode on the disk
 * index         : index of dinode, NOT the block index of dinode
 * ptr_descriptor: file system partition descriptor
 * out_dinode    : dinode read from disk
 */
BOOL read_dinode(
	uint32 index,
	const struct fs_partition_descriptor *ptr_descriptor,
	struct dinode *out_dinode
)
{
    uint32 block_index = ptr_descriptor->sb.dinode_start + (index / FS_CNT_DINODE_PER_BLOCK);
	uint32 offset = (index % FS_CNT_DINODE_PER_BLOCK) * DINODE_SIZE;
	struct fs_data_block data_block;
	byte *ptr_dinode_block = (byte *)(&data_block);
	BOOL ret;

	/* Read corresponding dinode block */
	ret = fslib_read_block(
		block_index,
		ptr_descriptor,
		&data_block
	);
	if (IS_TRUE(ret)) {
		/* Copy the corresponding dinode */
		memcpy(out_dinode, ptr_dinode_block + offset, DINODE_SIZE);
		return TRUE;
	}
	return FALSE;
}

/*
 # Write a dinode to the disk
 * index         : index of dinode, NOT the block index of dinode
 * ptr_descriptor: file system partition descriptor
 * in_dinode     : dinode written to disk
 */
BOOL write_dinode(
	uint32 index,
	const struct fs_partition_descriptor *ptr_descriptor,
	const struct dinode *in_dinode
)
{
	uint32 block_index = ptr_descriptor->sb.dinode_start + (index / FS_CNT_DINODE_PER_BLOCK);
	uint32 offset = (index % FS_CNT_DINODE_PER_BLOCK) * DINODE_SIZE;
	struct fs_data_block data_block;
	byte *ptr_dinode_block = (byte *)(&data_block);
	BOOL ret;

	/* Read corresponding dinode block */
	ret = fslib_read_block(
		block_index,
		ptr_descriptor,
		&data_block
	);
	if (IS_TRUE(ret)) {
		/* Copy the corresponding dinode */
		memcpy(ptr_dinode_block + offset, in_dinode, DINODE_SIZE);
		/* Write back to disk */
		ret = fslib_write_block(
			block_index,
			ptr_descriptor,
			&data_block
		);
		if (IS_TRUE(ret)) {
			return TRUE;
		}
	}
	return FALSE;
}

/*
 # Read data block pointed by given dinode and given cursor
 * ptr_dinode          : given dinode
 * cur_data_block      : data block cursor
 * ptr_descriptor      : file system partition descriptor
 * out_data_block_index: pointer of data block index
 * out_data_block      : data block pointer
 */
BOOL dinode_read_data_block(
	const struct dinode *ptr_dinode,
	uint32 cur_data_block,
	const struct fs_partition_descriptor *ptr_descriptor,
	uint32 *out_data_block_index,
	struct fs_data_block *out_data_block
)
{
	const struct indir_block_ref *ptr_indir_ref;
	uint32 data_block_idx; /* Target data block index */
    BOOL ret;

    if (cur_data_block < DINODE_DIRECT_CNT) {
        /* Read the data block directly */
		data_block_idx = ptr_dinode->block_ref[cur_data_block];

		// if (data_block_idx == DINODE_REF_NULL) {
		// 	return FALSE;
		// }
		kassert(data_block_idx != DINODE_REF_NULL);

    } else {
        
        /* Find the data block for indirect reference */
		kassert(ptr_dinode->block_ref[DINODE_INDIRECT_IDX] != DINODE_REF_NULL);
        ret = fslib_read_data_block(
            ptr_dinode->block_ref[DINODE_INDIRECT_IDX],
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
			kassert(ptr_indir_ref->next_ref != DINODE_REF_NULL);
			ret = fslib_read_data_block(
				ptr_indir_ref->next_ref,
				ptr_descriptor,
				out_data_block
			);
			/* Update */
			cur_data_block -= DINODE_INREF_CNT; /* Update cursor */
        	kassert(IS_TRUE(ret));
		}

		/* Reach the target one */
		data_block_idx = ptr_indir_ref->block_ref[cur_data_block];
		kassert(data_block_idx != DINODE_REF_NULL);
    }

	/* Read data block */
	return fslib_read_data_block(
		data_block_idx,
		ptr_descriptor,
		out_data_block
	);
}

/*
 # Insert data block pointed by given dinode and given cursor (recursively calling)
 * cur_reference_data_block_index: index of current reference data block
 * ptr_descriptor                : file system partition descriptor
 * data_block_index              : index of data block created
 * ptr_temp_data_block           : pointer to a temporary data block
 */
static BOOL dinode_insert_data_block_recursive(
	uint32 cur_reference_data_block_index,
	const struct fs_partition_descriptor *ptr_descriptor,
	uint32 data_block_index,
	struct fs_data_block *ptr_temp_data_block
)
{
	struct indir_block_ref *ptr_indir_ref = (struct indir_block_ref *)ptr_temp_data_block;
	int32 new_data_block_index;

	int32 i;
	BOOL ret;

	/* Read indirect reference data block */
	ret = fslib_read_data_block(
		cur_reference_data_block_index,
		ptr_descriptor,
		ptr_temp_data_block
	);
	if(NOT(IS_TRUE(ret))){
		panic("READING DINODE REFERENCE DATA BLOCK FAILED AT DATA BLOCK %d!\n",
			cur_reference_data_block_index);
	}
	/* Search an empty slot in that indirect reference data block */
	for (i = 0; i < DINODE_INREF_CNT; ++i)
	{
		if (ptr_indir_ref->block_ref[i] == DINODE_REF_NULL) {
			/* Record the data_block_index */
			ptr_indir_ref->block_ref[i] = data_block_index;
			/* Write the data block for indirect referece */
			ret = fslib_write_data_block(
				cur_reference_data_block_index,
				ptr_descriptor,
				ptr_temp_data_block
			);
			if(NOT(IS_TRUE(ret))){
				panic("UPDATING DINODE REFERENCE DATA BLOCK FAILED AT DATA BLOCK %d!\n",
					cur_reference_data_block_index);
			}
			break; /* assert: i < DINODE_INREF_CNT */
		}
	}
	if (i == DINODE_INREF_CNT) {
		/* Need to find a empty slot by next INDIRECT Reference */
		if (ptr_indir_ref->next_ref == DINODE_REF_NULL) {
			/* Create a new data block for indirect reference */
			new_data_block_index = fslib_get_data_block(ptr_descriptor); /* Get an empty data block location */
			kassert(new_data_block_index >= 0);
			/* Update the indirect reference index in current indirect reference block */
			ptr_indir_ref->next_ref = new_data_block_index;
			ret = fslib_write_data_block(
				cur_reference_data_block_index,
				ptr_descriptor,
				ptr_temp_data_block
			);
			if(NOT(IS_TRUE(ret))){
				panic("UPDATING INDIRECT REFERENCE BLOCK FAILED!\n");
			}
			/* Init. the data block for indirect referece */
			for (i = 0; i < DINODE_INREF_CNT; ++i) {
				ptr_indir_ref->block_ref[i] = DINODE_REF_NULL;
			}
			ptr_indir_ref->next_ref = DINODE_REF_NULL;
			/* Record the data_block_index into the 1st entry of the dinode */
			ptr_indir_ref->block_ref[0] = data_block_index;
			/* Write the data block for indirect referece */
			ret = fslib_write_data_block(
				new_data_block_index,
				ptr_descriptor,
				ptr_temp_data_block
			);
			if(NOT(IS_TRUE(ret))){
				panic("CREATING INDIRECT REFERENCE DATA BLOCK FAILED!\n");
			}
		} else {
			return dinode_insert_data_block_recursive(
				ptr_indir_ref->next_ref,
				ptr_descriptor,
				data_block_index,
				ptr_temp_data_block
			);
		}
	}
	return TRUE;
}


/*
 # Insert data block pointed by given dinode and given cursor
 * dinode_index    : index of given dinode
 * ptr_dinode      : given dinode
 * ptr_descriptor  : file system partition descriptor
 * data_block_index: data block index
 * in_data_block   : data block pointer
 */
 BOOL dinode_insert_data_block(
	uint32 dinode_index,
	struct dinode *ptr_dinode,
	const struct fs_partition_descriptor *ptr_descriptor,
	uint32 data_block_index,
	const struct fs_data_block *in_data_block
)
{
	/* For searching indirect reference data block */
	struct fs_data_block temp_data_block;
	struct indir_block_ref *ptr_indir_ref = (struct indir_block_ref *)&temp_data_block;
	/* For creating new data block */
	int32 new_data_block_index;

	int32 i;
    BOOL ret;

	kassert(data_block_index != DINODE_REF_NULL);
	/* Search an empty slot in DIRECT Reference */
	for (i = 0; i < DINODE_DIRECT_CNT; ++i)
	{
		if (ptr_dinode->block_ref[i] == DINODE_REF_NULL) {
			/* Update index recorded in dinode directly */
			ptr_dinode->block_ref[i] = data_block_index;
			ret = write_dinode(
				dinode_index,
				ptr_descriptor,
				ptr_dinode
			);
			if(NOT(IS_TRUE(ret))){
				panic("UPDATING DINODE FAILED!\n");
			}
			break; /* assert: i < DINODE_DIRECT_CNT */
		}
	}
	if (i == DINODE_DIRECT_CNT) {
		/* Need to find a empty slot by INDIRECT Reference */
		if (ptr_dinode->block_ref[DINODE_INDIRECT_IDX] == DINODE_REF_NULL) {
			/* 1. Indirect reference data block does not exist, create */
			new_data_block_index = fslib_get_data_block(ptr_descriptor); /* Get an empty data block location */
			kassert(new_data_block_index >= 0);
			/* Update the indirect reference index in current dinode */
			ptr_dinode->block_ref[DINODE_INDIRECT_IDX] = new_data_block_index;
			ret = write_dinode(
				dinode_index,
				ptr_descriptor,
				ptr_dinode
			);
			if(NOT(IS_TRUE(ret))){
				panic("UPDATING DINODE FAILED FOR INDIRECT REFERENCE!\n");
			}
			/* Init. the data block for indirect referece */
			for (i = 0; i < DINODE_INREF_CNT; ++i) {
				ptr_indir_ref->block_ref[i] = DINODE_REF_NULL;
			}
			ptr_indir_ref->next_ref = DINODE_REF_NULL;
			/* Record the data_block_index into the 1st entry of the dinode */
			ptr_indir_ref->block_ref[0] = data_block_index;
			/* Write the data block for indirect referece */
			ret = fslib_write_data_block(
				new_data_block_index,
				ptr_descriptor,
				&temp_data_block
			);
			if(NOT(IS_TRUE(ret))){
				panic("WRITING DINODE REFERENCE DATA BLOCK FAILED!\n");
			}
		} else {
			/* 2. Indirect reference block exists, read it and search for an empty slot */
			ret = dinode_insert_data_block_recursive(
					ptr_dinode->block_ref[DINODE_INDIRECT_IDX],
	 				ptr_descriptor,
					data_block_index,
					&temp_data_block
			);
			if (NOT(IS_TRUE(ret))) {
				panic("DINODE INSERT DATA BLOCK RECURSIVE FAILED!\n");
			}
		}
	}
	/* Update data to data block */
	return fslib_write_data_block(
		data_block_index,
		ptr_descriptor,
		in_data_block
	);
}