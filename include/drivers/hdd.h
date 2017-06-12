#ifndef _DRV_HDD_H_
#define _DRV_HDD_H_

#include "part_table.h"
#include "proc.h"

/*
 * HDD Minor Device Number:
 *
 * Current supported hdd devices:
 * hd0 : primary & master hdd
 * hd1 : primary & slave hdd
 *
 * Plan to support hdd devices:
 * hd2 : secondary & master hdd
 * hd3 : secondary & slave hdd
 */
#define HDD_DEV_PM		0
#define HDD_DEV_PS		1
#define HDD_DEV_SM		2
#define HDD_DEV_SS		3

/* Harddisk Driver Message Type */
#define HDD_MAGIC_NUM		'h'

#define HDD_MSG_OK		IPC_MSG_TYPE_GEN(HDD_MAGIC_NUM, DRV_PID_HDD, 0)
#define HDD_MSG_ERROR		IPC_MSG_TYPE_GEN(HDD_MAGIC_NUM, DRV_PID_HDD, -1)

#define HDD_MSG_OPEN		IPC_MSG_TYPE_GEN(HDD_MAGIC_NUM, DRV_PID_HDD, 10)
#define HDD_MSG_WRITE		IPC_MSG_TYPE_GEN(HDD_MAGIC_NUM, DRV_PID_HDD, 11)
#define HDD_MSG_READ		IPC_MSG_TYPE_GEN(HDD_MAGIC_NUM, DRV_PID_HDD, 12)
#define HDD_MSG_CLOSE		IPC_MSG_TYPE_GEN(HDD_MAGIC_NUM, DRV_PID_HDD, 13)
#define HDD_MSG_IOCTL		IPC_MSG_TYPE_GEN(HDD_MAGIC_NUM, DRV_PID_HDD, 14)

/* Harddisk Driver Ioctl Message Type */
#define HDD_IMSG_PRINT_ID	0x10

/* Harddisk Driver Message Payload */
struct ipc_msg_payload_hdd
{
	/* Minor device number */
	uint32 dev_num;
	/* Base address */
	uint32 base_low;
	uint32 base_high;
	/* Size in bytes */
	uint32 size;
	/* Ioctl message type */
	uint32 ioctl_msg;
	/* Memory address of buffer in other process */
	void *buf_address;
};

/* Common HDD macros */
#define HDD_MAX_DRIVES		2	/* Primary IDE: Master + Slave */
#define HDD_BYTES_PER_SECTOR	512
#define HDD_DRIVE_OFFSET	(PART_MAX_PART_MBR + 1)
#define HDD_LBA28_MAX		0xfffffff
#define HDD_LBA28_BYTE_MAX	0x1ffffffe00ull
#define HDD_DRV_MASTER		0
#define HDD_DRV_SLAVE		1


/* Harddisk Driver Functions */
void hdd_interrupt_handler(void);

void hdd_init(void);
void hdd_message_dispatcher(void);


#endif
