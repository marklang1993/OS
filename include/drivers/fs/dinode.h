#ifndef _FS_DINODE_H_
#define _FS_DINODE_H_

#include "type.h"

#pragma pack(push, 1)

/* dinode common macros */
#define DINODE_DIRECT_CNT	11	/* Direct reference count */
#define DINODE_INDIRECT_CNT	1	/* Indirect reference count */
#define DINODE_REF_CNT		(DINODE_DIRECT_CNT + DINODE_INDIRECT_CNT)

/* dinode type */
#define DINODE_INVALID		0	/* It can also be EMPTY */
#define DINODE_FILE		1
#define DINODE_DIRECTORY	2
#define DINODE_DEVICE		3

/* Null reference */
#define DINODE_REF_NULL		0

/* inode struct stored on disk */
struct dinode {
	uint16 type;				/* Resource Type */
	uint16 link_cnt;			/* Count of Links to this dinode */
	/* Used by DINODE_DEVICE */
	uint32 major_dev;			/* Major Device Number */
	uint32 minor_dev;			/* Minor Device Number */
	/* Used by DINODE_FILE */
	uint32 size;				/* File Size in Bytes */
	uint32 block_ref[DINODE_REF_CNT];	/* Block References */
};
#define DINODE_SIZE		sizeof(struct dinode)

/* Indirect block reference - will use an entire data block */
struct indir_block_ref {
	uint32 block_ref[FS_BYTES_PER_BLOCK / sizeof(uint32) - 1];
	uint32 next_ref;	/* Next indir_block_ref */
};

#pragma pack(pop)

#endif
