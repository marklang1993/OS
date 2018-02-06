#include "dbg.h"
#include "type.h"
#include "drivers/fs/directory.h"
#include "drivers/fs/fs.h"
#include "drivers/fs/fs_lib.h"



/*
 # Build superblock by given partition info.
 * ptr_descriptor: file system partition descriptor
 */
void fslib_build_superblock(struct fs_partition_descriptor *ptr_descriptor)
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

/*
 # Build 1st dinode on a full data block
 * buffer: a full data block with size of FS_BYTES_PER_BLOCK
 */
void fslib_build_1st_dinode(byte *ptr_buffer)
{
/*
	printf("size of buffer: %d\n", sizeof(ptr_buffer));
	printf("address of buffer: 0x%x\n", ptr_buffer);
*/
	struct dinode root;

	/* Clean buffer of data block & dinode */
	memset(ptr_buffer, 0, FS_BYTES_PER_BLOCK);

	/* Init. 1st dinode */
	root.type = DINODE_DIRECTORY;
	root.link_cnt = 1;
	root.major_dev = 0;
	root.minor_dev = 0;
	root.size = 0;

	/* Write 1st dinode to the buffer */
	/* printf("size of dinode is: %d\n", sizeof(struct dinode)); */
	memcpy(ptr_buffer, &root, sizeof(struct dinode));
}

/*
 # Open a file
 * path          : file path
 * mode          : open mode
 * src_pid       : caller's pid
 * ptr_descriptor: file system partition descriptor
 @ RETURN	     : file descriptor id in the caller's process; -1 when error
 */
int32 fslib_open_file(
	const char *path,
	FILELIB_OP_FLAG mode,
	uint32 src_pid,
	const struct fs_partition_descriptor *ptr_descriptor
)
{

}