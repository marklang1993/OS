#ifndef _FS_DINODE_H_
#define _FS_DINODE_H_

#include "drivers/fs/fs_lib.h"

#pragma pack(push, 1)

/* dinode common macros */
#define DINODE_DIRECT_CNT	11	/* Direct reference count */
#define DINODE_INDIRECT_IDX DINODE_DIRECT_CNT /* Index of indirect reference block */
#define DINODE_INDIRECT_CNT	1	/* Indirect reference count */
#define DINODE_REF_CNT		(DINODE_DIRECT_CNT + DINODE_INDIRECT_CNT)

/* dinode type */
#define DINODE_INVALID		0	/* It can also be EMPTY */
#define DINODE_FILE			1
#define DINODE_DIRECTORY	2
#define DINODE_DEVICE		3

/* Null reference */
#define DINODE_REF_NULL		0xffffffff

/* Root directory dinode */
#define DINODE_ROOT_DIRE_IDX	0

/* inode struct stored on disk */
struct dinode {
	uint16 type;				/* Resource Type */
	/*
	 * Count of Links to this dinode
	 * Rules:
	 * 1. Any valid dinode must have at least 1;
	 * 2. Root dinode that is not referred by any dinode has link_cnt = 1
	 */
	uint16 link_cnt;
	/* Used by DINODE_DEVICE */
	uint32 major_dev;			/* Major Device Number */
	uint32 minor_dev;			/* Minor Device Number */
	/* Used by DINODE_FILE */
	uint32 size;				/* File Size in Bytes */
	uint32 block_ref[DINODE_REF_CNT];	/* Block References */
};
#define DINODE_SIZE			sizeof(struct dinode)

/* Indirect block reference - will use an entire data block */
#define DINODE_INREF_CNT	(FS_BYTES_PER_BLOCK / sizeof(uint32) - 1)
struct indir_block_ref {
	uint32 block_ref[DINODE_INREF_CNT];
	uint32 next_ref;	/* Next indir_block_ref */
};

/* Disk Inode Related Functions */
/* Get a free dinode */
int32 get_dinode(const struct fs_partition_descriptor *ptr_descriptor);
/* Free a used dinode */
void put_dinode(
	uint32 index,
	const struct fs_partition_descriptor *ptr_descriptor
);
/* Read a dinode on the disk */
BOOL read_dinode(
	uint32 index,
	const struct fs_partition_descriptor *ptr_descriptor,
	struct dinode *out_dinode
);
/* Write a dinode to the disk */
BOOL write_dinode(
	uint32 index,
	const struct fs_partition_descriptor *ptr_descriptor,
	const struct dinode *in_dinode
);
/* Read data block pointed by given dinode and given cursor */
BOOL dinode_read_data_block(
	const struct dinode *ptr_dinode,
	uint32 cur_data_block,
	const struct fs_partition_descriptor *ptr_descriptor,
	uint32 *out_data_block_index,
	struct fs_data_block *out_data_block
);
/* Insert data block pointed by given dinode and given cursor & Update the dinode */
BOOL dinode_insert_data_block(
	uint32 dinode_index,
	struct dinode *ptr_dinode,
	const struct fs_partition_descriptor *ptr_descriptor,
	uint32 data_block_index,
	const struct fs_data_block *in_data_block
);

#pragma pack(pop)

#endif
