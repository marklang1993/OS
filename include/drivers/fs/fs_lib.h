#ifndef _DRV_FS_LIB_H_
#define _DRV_FS_LIB_H_

#include "drivers/fs/fs.h"

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
	uint32 status;			/* File system partition status */
	uint32 ref_cnt;			/* Reference count */
	uint32 sector_cnt;		/* Especially, used by unrecognizable partition */
	/* used for inverse reference */
	uint32 mbr_index;		/* MBR index of this partition */
	uint32 logical_index;	/* Logical index of this partition */
	struct superblock sb;	/* Superblock */
};

/* FS MBR partition descriptor */
struct fs_mbr_partition_descriptor
{
	struct fs_partition_descriptor main;
	struct fs_partition_descriptor logicals[PART_MAX_L_PER_EX_PART];
};

/* FS data block */
struct fs_data_block
{
	byte data[FS_BYTES_PER_BLOCK];
};

/* HDD opeartion struct */
struct fs_hdd_op
{
	uint32 fs_mbr_index;	/* FS partition mbr index */
	uint32 fs_logical_index;/* FS partition logic index */
	uint64 base;		/* Base position of hdd operation */
	uint32 size;		/* Size of buffer */
	void *buf_address;	/* Buffer address */
	BOOL is_read;		/* Is read operation */
};

/* File system driver library functions */
void fslib_hdd_op(const struct fs_hdd_op *param); /* HDD operation */

/* Read bytes on the disk */
BOOL fslib_read_bytes(
	uint32 index,
	const struct fs_partition_descriptor *ptr_descriptor,
	void *buf,
	uint32 size
);
/* Write bytes to the disk */
BOOL fslib_write_bytes(
	uint32 index,
	const struct fs_partition_descriptor *ptr_descriptor,
	const void *buf,
	uint32 size
);
/* Read a block on the disk */
BOOL fslib_read_block(
	uint32 index,
	const struct fs_partition_descriptor *ptr_descriptor,
	struct fs_data_block *out_block
);
/* Write a block to the disk */
BOOL fslib_write_block(
	uint32 index,
	const struct fs_partition_descriptor *ptr_descriptor,
	const struct fs_data_block *in_block
);

/* Get a free datablock */
int32 fslib_get_data_block(const struct fs_partition_descriptor *ptr_descriptor);
/* Free a used datablock */
void fslib_put_data_block(
	uint32 index,
	const struct fs_partition_descriptor *ptr_descriptor
);
/* Read a datablock on the disk */
BOOL fslib_read_data_block(
	uint32 index,
	const struct fs_partition_descriptor *ptr_descriptor,
	struct fs_data_block *out_data_block
);
/* Write a datablock to the disk */
BOOL fslib_write_data_block(
	uint32 index,
	const struct fs_partition_descriptor *ptr_descriptor,
	const struct fs_data_block *in_data_block
);

/* Find an empty bit in a bitmap block */
int32 fslib_find_empty_bitmap_block(
	const struct fs_data_block *bitmap_block
);
/* Set a bit in a bitmap block */
void fslib_set_bitmap_block(
	uint32 index,
	BOOL val,
	struct fs_data_block *bitmap_block
);

/* library functions used in mkfs() */
void fslib_build_superblock(struct fs_partition_descriptor *ptr_descriptor);
void fslib_build_1st_dinode(byte *ptr_buffer);

typedef uint32 FILELIB_OP_FLAG; /* File Operation Flags */
#define FLIB_O_READ		0
#define FLIB_O_WRITE	1
#define FLIB_O_APPEND	2
int32 fslib_open_file(
	const char *path,
	FILELIB_OP_FLAG mode,
	uint32 src_pid,
	const struct fs_partition_descriptor *ptr_descriptor
);

#endif