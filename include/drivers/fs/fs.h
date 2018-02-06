#ifndef _DRV_FS_H_
#define _DRV_FS_H_

#include "drivers/hdd_part.h"

/* File System Driver Message Type */
#define FS_MAGIC_NUM		'f'

#define FS_MSG_OK		IPC_MSG_TYPE_GEN(FS_MAGIC_NUM, DRV_PID_FS, 0)
#define FS_MSG_ERROR		IPC_MSG_TYPE_GEN(FS_MAGIC_NUM, DRV_PID_FS, -1)

#define FS_MSG_OPEN		IPC_MSG_TYPE_GEN(FS_MAGIC_NUM, DRV_PID_FS, 10)
#define FS_MSG_WRITE		IPC_MSG_TYPE_GEN(FS_MAGIC_NUM, DRV_PID_FS, 11)
#define FS_MSG_READ		IPC_MSG_TYPE_GEN(FS_MAGIC_NUM, DRV_PID_FS, 12)
#define FS_MSG_CLOSE		IPC_MSG_TYPE_GEN(FS_MAGIC_NUM, DRV_PID_FS, 13)
#define FS_MSG_IOCTL		IPC_MSG_TYPE_GEN(FS_MAGIC_NUM, DRV_PID_FS, 14)

/* File System Driver Ioctl Message Type */
#define FS_IMSG_MKFS		0x10	/* Make File System */
#define FS_IMSG_OPEN_FILE	0x11	/* Open file */
struct ipc_msg_payload_fs_open_file
{
	const char *path;
	uint32 mode;
};

/* Common FS macros */
#define FS_FACTOR_BS				1	/* Factor from Sector to Block */
#define FS_BYTES_PER_BLOCK			(HDD_BYTES_PER_SECTOR * FS_FACTOR_BS)
#define FS_MAX_MBR_P_CNT			HDDP_MAX_MBR_P_CNT
#define FS_MAX_PATH_LENGTH			256
#define FS_SYSROOT_MBR_IDX			1
#define FS_SYSROOT_LOGICAL_IDX		'a'

/*
 # File System Device Number Generator
 @ mbr     : MBR partition index (Range: 0~7)
 @ logical : Logical partition index (Range: a~p)
 */
#define FS_DEV_NUM_GEN(mbr, logical)	HDDP_DEV_NUM_GEN(mbr, logical)

/*
 # Convert FS Logical Partition Index to Logical Partition Index
 @ fs_logical : FS logical partition index
 */
#define LOGICAL_CONVERT(fs_logical) \
	(fs_logical == 0 ? 0 : fs_logical - 1 + 'a')

/*
 # Get MBR partition index from File System Device Number
 @ dev_number : File System Device Number
 */
#define FS_GET_MBR_NUM(dev_number)	HDDP_GET_MBR_NUM(dev_number)

/*
 # Get logical partition index from File System Device Number
 @ dev_number : File System Device Number
 */
#define FS_GET_LOGICAL_NUM(dev_number)	HDDP_GET_LOGICAL_NUM(dev_number)

/* Calculate byte offset w.r.t block index */
#define FS_BLOCK2BYTE(block_idx)	(((uint64)(block_idx)) * FS_BYTES_PER_BLOCK)

/* File System Driver Message Payload */
struct ipc_msg_payload_fs
{
	/* Minor device number */
	uint32 dev_num;
	/* Ioctl message type */
	uint32 ioctl_msg;
	/* Memory address of buffer in other process */
	void *buf_address;
};


/* File System Driver Functions */
void fs_init(void);
void fs_message_dispatcher(void);

#endif
