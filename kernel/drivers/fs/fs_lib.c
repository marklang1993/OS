#include "dbg.h"
#include "type.h"
#include "drivers/fs/fs_lib.h"

/*
 # Build superblock by given partition info.
 * ptr_descriptor: file system partition descriptor
 */
void build_superblock(struct fs_partition_descriptor *ptr_descriptor)
{
    uint32 remained_block_cnt; /* Count of current remained free blocks */
	uint32 dinode_tot_cnt; /* Total count of all dinode */
	uint32 data_map_tot_cnt; /* Total count of data map blocks */

    /* Check minimum requirement of sectors */
	if (ptr_descriptor->sector_cnt < FS_MINIMUM_SECTORS)
		panic("FS - NO ENOUGH SPACE FOR MKFS!\n");
	/* Build Superblock - Basic */
	ptr_descriptor->sb.magic_num = SUPERBLOCK_MAGIC_NUM;
	ptr_descriptor->sb.size = ptr_descriptor->sector_cnt / FS_FACTOR_BS;
	ptr_descriptor->sb.log_start = LOGBLOCK_IDX;
	ptr_descriptor->sb.log_cnt = LOGBLOCK_CNT;

	remained_block_cnt = ptr_descriptor->sb.size;
	/* Build Superblock - DINODE Map */
	remained_block_cnt -= SUPERBLOCK_CNT + LOGBLOCK_CNT;
	dinode_tot_cnt = ((remained_block_cnt / FS_DINODE_FB_RATIO) +
			(FS_CNT_DINODE_PER_BLOCK - 1)) / FS_CNT_DINODE_PER_BLOCK *
			FS_CNT_DINODE_PER_BLOCK; /* Round up */
	ptr_descriptor->sb.dinode_tot_cnt = dinode_tot_cnt;
	ptr_descriptor->sb.dinode_map_start = LOGBLOCK_IDX + LOGBLOCK_CNT;
	ptr_descriptor->sb.dinode_map_cnt = (dinode_tot_cnt +
				(FS_CNT_BIT_PER_BLOCK - 1)) /
				FS_CNT_BIT_PER_BLOCK; /* Round up */

	/* Build Superblock - DINODE */
	remained_block_cnt -= ptr_descriptor->sb.dinode_map_cnt;
	ptr_descriptor->sb.dinode_start = ptr_descriptor->sb.dinode_map_start +
			ptr_descriptor->sb.dinode_map_cnt;
	ptr_descriptor->sb.dinode_cnt = dinode_tot_cnt / FS_CNT_DINODE_PER_BLOCK;

	/* Build Superblock - Data Map */
	remained_block_cnt -= ptr_descriptor->sb.dinode_cnt;
	kassert(remained_block_cnt <= 0xffffffff - FS_CNT_BIT_PER_BLOCK);
	data_map_tot_cnt = remained_block_cnt + FS_CNT_BIT_PER_BLOCK; /* Round Up */
	data_map_tot_cnt /= FS_CNT_BIT_PER_BLOCK + 1;
	ptr_descriptor->sb.data_map_start = ptr_descriptor->sb.dinode_start +
			ptr_descriptor->sb.dinode_cnt;
	ptr_descriptor->sb.data_map_cnt = data_map_tot_cnt;

	/* Build Superblock - Data */
	remained_block_cnt -= ptr_descriptor->sb.data_map_cnt;
	ptr_descriptor->sb.data_start = ptr_descriptor->sb.data_map_start +
			ptr_descriptor->sb.data_map_cnt;
	ptr_descriptor->sb.data_cnt = remained_block_cnt;
}