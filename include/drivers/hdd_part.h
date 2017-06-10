#ifndef _DRV_HDD_PART_H_
#define _DRV_HDD_PART_H_

#include "drivers/hdd.h"

/* Harddisk Partition Driver Message Type */
#define HDDP_MAGIC_NUM		'p'

#define HDDP_MSG_OK              IPC_MSG_TYPE_GEN(HDDP_MAGIC_NUM, DRV_PID_HDDP, 0)
#define HDDP_MSG_ERROR           IPC_MSG_TYPE_GEN(HDDP_MAGIC_NUM, DRV_PID_HDDP, -1)

#define HDDP_MSG_OPEN            IPC_MSG_TYPE_GEN(HDDP_MAGIC_NUM, DRV_PID_HDDP, 10)
#define HDDP_MSG_WRITE           IPC_MSG_TYPE_GEN(HDDP_MAGIC_NUM, DRV_PID_HDDP, 11)
#define HDDP_MSG_READ            IPC_MSG_TYPE_GEN(HDDP_MAGIC_NUM, DRV_PID_HDDP, 12)
#define HDDP_MSG_CLOSE           IPC_MSG_TYPE_GEN(HDDP_MAGIC_NUM, DRV_PID_HDDP, 13)

/*
 * HDD Partition Device Number -- Minor Device Number
 *
 * Current supported hdd devices:
 *
 * HDD_DEV_PM
 * hdp0~3  : primary & master MBR partition 1~4
 * hdp0a~0p : primary & master logical partition 1~16 at MBR partition 1
 * hdp1a~1p : primary & master logical partition 1~16 at MBR partition 2
 * hdp2a~2p : primary & master logical partition 1~16 at MBR partition 3
 * hdp3a~3p : primary & master logical partition 1~16 at MBR partition 4
 *
 * HDD_DEV_PS
 * hdp4~7  : primary & slave MBR partition 1~4
 * hdp4a~4p : primary & slave logical partition 1~16 at MBR partition 1
 * hdp5a~5p : primary & slave logical partition 1~16 at MBR partition 2
 * hdp6a~6p : primary & slave logical partition 1~16 at MBR partition 3
 * hdp7a~7p : primary & slave logical partition 1~16 at MBR partition 4
 */
#define HDDP_MAX_MBR_P_CNT	(PART_MAX_PART_MBR * HDD_MAX_DRIVES)


/*
 # HDD Partition Device Number Generator
 @ mbr     : MBR partition index (Range: 0~7)
 @ logical : Logical partition index (Range: a~p)
 */
#define HDDP_DEV_NUM_GEN(mbr, logical) \
	((mbr << 8) | (logical == 0 ? 0 : (uint8)(logical - 'a' + 1)))
/*
 # Get MBR partition index from HDD Partition Device Number
 @ dev_number : HDD Partition Device Number
 */
#define HDDP_GET_MBR_NUM(dev_number) \
	((dev_number & 0xff00) >> 8)

/*
 # Get logical partition index from HDD Partition Device Number
 @ dev_number : HDD Partition Device Number
 */
#define HDDP_GET_LOGICAL_NUM(dev_number) \
	(dev_number & 0xff)


/* Harddisk Partition Driver Message Payload */
struct ipc_msg_payload_hdd_part
{
	/* Minor device number */
	uint32 dev_num;
	/* Is opeartion on reserved sectors */
	BOOL is_reserved;
	/* Base address w.r.t PARTITION base address */
	uint32 base_low;
	uint32 base_high;
	/* Size in bytes */
	uint32 size;
	/* Memory address of buffer in other process */
	void *buf_address;
};

/* Harddisk Partition Functions */
void hddp_init(void);
void hddp_message_dispatcher(void);

#endif
