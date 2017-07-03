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

/* Common FS macros */
#define FS_BYTES_PER_BLOCK	HDD_BYTES_PER_SECTOR

/* File System Driver Functions */
void fs_message_dispatcher(void);

#endif
