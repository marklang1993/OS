#ifndef _DRV_FS_LIB_H_
#define _DRV_FS_LIB_H_

#include "drivers/fs/fs.h"
#include "drivers/fs/file_desc.h"

/* File System Common macros */
#define FS_MINIMUM_SECTORS	20	/* Minimum requirement of sectors */
#define FS_CNT_DINODE_PER_BLOCK	(FS_BYTES_PER_BLOCK / DINODE_SIZE)
#define FS_CNT_BIT_PER_BLOCK	(FS_BYTES_PER_BLOCK * 8)
#define FS_DINODE_FB_RATIO	4	/* Ratio of dinode count and free blocks */

/* Superblock */
#define SUPERBLOCK_MAGIC_NUM	0x50415241
#define SUPERBLOCK_IDX		0
#define SUPERBLOCK_CNT		1	/* ! HARD CODE SIZE */

/* Log block */
#define LOGBLOCK_IDX		(SUPERBLOCK_IDX + SUPERBLOCK_CNT)
#define LOGBLOCK_CNT		10

/* Superblock struct */
struct superblock {
	/* Meta Data */
	uint32 magic_num;	/* Magic number */
	uint32 size;		/* Size of this partition in blocks */
	uint32 log_start;	/* Log block start position */
	uint32 log_cnt;		/* Log block count */
	uint32 dinode_map_start;/* dinode map block start position */
	uint32 dinode_map_cnt;	/* dinode map block count */
	uint32 dinode_start;	/* dinode block start position */
	uint32 dinode_cnt;	/* dinode block count */
	uint32 data_map_start;	/* Data blocks map block start position */
	uint32 data_map_cnt;	/* Data blocks map block count */
	uint32 data_start;	/* Data blocks block start position */
	uint32 data_cnt;	/* Data blocks block count */
	/* Other Data */
	uint32 dinode_tot_cnt;	/* Total count of dinode */
};

/* FS partition descriptor */
struct fs_partition_descriptor
{
	uint32 status;		/* File system partition status */
	uint32 ref_cnt;		/* Reference count */
	uint32 sector_cnt;	/* Especially, used by unrecognizable partition */
	struct superblock sb;	/* Superblock */
};

/* FS MBR partition descriptor */
struct fs_mbr_partition_descriptor
{
	struct fs_partition_descriptor main;
	struct fs_partition_descriptor logicals[PART_MAX_L_PER_EX_PART];
};


/* File system driver library functions */
void build_superblock(struct fs_partition_descriptor *ptr_descriptor);

#endif