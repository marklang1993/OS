#include "dbg.h"
#include "lib.h"
#include "drivers/fs/fs.h"
#include "drivers/fs/inode.h"

/* Superblock */
#define SUPERBLOCK_MAGIC_NUM	0x50415241
#define SUPERBLOCK_IDX		0

/* File System Partition Status */
#define FS_INVALID		0	/* Did not open */
#define FS_UNRECOG		1	/* Open, but cannot r/w, limited ioctl */
#define FS_VALID		2	/* Open, all operations are available */

/* Calculate byte offset w.r.t block index */
#define BLOCK2BYTE(block_idx)	(((uint64)block_idx) * FS_BYTES_PER_BLOCK)

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
	uint32 block_cnt;	/* Especially, used by unrecognizable partition */
	struct superblock sb;	/* Superblock */
};

/* FS MBR partition descriptor */
struct fs_mbr_partition_descriptor
{
	struct fs_partition_descriptor main;
	struct fs_partition_descriptor logicals[PART_MAX_L_PER_EX_PART];
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

/* FS internal global variables */
static BOOL fs_is_init = FALSE;
/* FS partition table */
static struct fs_mbr_partition_descriptor fs_part_table[FS_MAX_MBR_P_CNT];


/*
 # HDD operation (r/w) in bytes
 @ param : operation parameters
 */
static void hdd_op(const struct fs_hdd_op *param)
{
	struct proc_msg msg;
	struct ipc_msg_payload_hddp *payload;
	struct ipc_msg_payload_hddp_get_info *ret_payload;

	payload = (struct ipc_msg_payload_hddp *)msg.payload;

	/* Prepare message */
	msg.type = IS_TRUE(param->is_read) ? HDDP_MSG_READ : HDDP_MSG_WRITE;
	payload->dev_num = HDDP_DEV_NUM_GEN(
			param->fs_mbr_index,
			LOGICAL_CONVERT(param->fs_logical_index)
			);
	payload->is_reserved = FALSE;
	payload->base_low = UINT64_LOW(param->base);
	payload->base_high = UINT64_HIGH(param->base);
	payload->size = param->size;
	payload->buf_address = param->buf_address;
	comm_msg(DRV_PID_HDDP, &msg);

	/* Check result */
	if (HDDP_MSG_OK != msg.type) {
		if (param->is_read) {
			panic("FS - HDDP READ ERROR!\n");
		} else {
			panic("FS - HDDP WRITE ERROR!\n");
		}
	}
}


/*
 # Get hdd partition information
 @ fs_mbr_index   : fs partition mbr index
 @ fs_logic_index : fs partition logic index
 @ ptr_info  : pointer to partition info. struct
 */
static void get_hddp_info(
	uint32 fs_mbr_index,
	uint32 fs_logical_index,
	struct ipc_msg_payload_hddp_get_info *ptr_info
)
{
	struct proc_msg msg;
	struct ipc_msg_payload_hddp *payload;
	struct ipc_msg_payload_hddp_get_info *ret_payload;

	payload = (struct ipc_msg_payload_hddp *)msg.payload;

	/* Send IOCTL message */
	msg.type = HDDP_MSG_IOCTL;
	payload->dev_num = HDDP_DEV_NUM_GEN(
			fs_mbr_index,
			LOGICAL_CONVERT(fs_logical_index)
			);
	payload->ioctl_msg = HDDP_IMSG_GET_INFO;
	comm_msg(DRV_PID_HDDP, &msg);

	/* Check result */
	if (HDDP_MSG_OK != msg.type)
		panic("FS - GET PARTITION INFO ERROR!\n");

	/* Copy return info. */
	memcpy(
		ptr_info,
		payload,
		sizeof(struct ipc_msg_payload_hddp_get_info)
		);
}


/*
 # FS_OPEN message handler
 @ param : fs partition open parameters
 */
static void fs_dev_open(struct ipc_msg_payload_fs *param)
{
	uint32 fs_mbr_index, fs_logical_index;
	struct fs_partition_descriptor *ptr_descriptor;
	struct ipc_msg_payload_hddp_get_info hddp_info;

	struct proc_msg msg;
	struct ipc_msg_payload_hddp *payload;
	struct fs_hdd_op hdd_op_param;

	/* Get partition table index */
	fs_mbr_index = FS_GET_MBR_NUM(param->dev_num);
	fs_logical_index = FS_GET_LOGICAL_NUM(param->dev_num);

	/* Get corresponding fs_partition_descriptor */
	if (0 == fs_logical_index) {
		/* Access to MBR partition */
		ptr_descriptor = &fs_part_table[fs_mbr_index].main;
	} else {
		/* Access to logical partition */
		ptr_descriptor = &(fs_part_table[fs_mbr_index].
			logicals[fs_logical_index - 1]);
	}

	/* Check the ref_cnt */
	if (0 == ptr_descriptor->ref_cnt) {
		/* Need to do some initializations */
		kassert(ptr_descriptor->status == FS_INVALID);
		/* Send HDDP_OPEN message */
		payload = (struct ipc_msg_payload_hddp *)msg.payload;
		msg.type = HDDP_MSG_OPEN;
		payload->dev_num = HDDP_DEV_NUM_GEN(
				fs_mbr_index,
				LOGICAL_CONVERT(fs_logical_index)
				);
		comm_msg(DRV_PID_HDDP, &msg);
		/* Check result */
		if (HDDP_MSG_OK != msg.type)
			panic("FS - OPEN PARTITION ERROR!\n");

		/* Read partition information */
		get_hddp_info(
			fs_mbr_index,
			fs_logical_index,
			&hddp_info
			);

		/* Read superblock */
		hdd_op_param.fs_mbr_index = fs_mbr_index;
		hdd_op_param.fs_logical_index = fs_logical_index;
		hdd_op_param.base = BLOCK2BYTE(SUPERBLOCK_IDX);
		hdd_op_param.size = sizeof(struct superblock);
		hdd_op_param.buf_address = &ptr_descriptor->sb;
		hdd_op_param.is_read = TRUE;
		hdd_op(&hdd_op_param);

		/* Check the magic number */
		if (SUPERBLOCK_MAGIC_NUM == ptr_descriptor->sb.magic_num) {
			/* Recognized file system */
			ptr_descriptor->status = FS_VALID;
			printk("FS VALID\n");
		} else {
			/* Unrecognizable file system */
			ptr_descriptor->status = FS_UNRECOG;
			printk("FS UNRECOG!\n");
		}
	} else {
		/* Already Init. */
		kassert(ptr_descriptor->status == FS_VALID);
		ptr_descriptor->ref_cnt += 1;
	}
}


/*
# FS_CLOSE message handler
@ param : fs partition close parameters
*/
static void fs_dev_close(struct ipc_msg_payload_fs *param)
{
}


/*
 # Initialize File System Driver (Ring 0)
 */
void fs_init(void)
{
	/* Init. fs partition table */
	memset(fs_part_table, 0x0, sizeof(fs_part_table));

	/* Set Init. flag */
	fs_is_init = TRUE;
}


/* File System Driver Message Dispatcher */
void fs_message_dispatcher(void)
{
/*
	struct proc_msg msg;
	struct ipc_msg_payload_hddp *payload;

	payload = (struct ipc_msg_payload_hddp *)msg.payload;

	msg.type = HDDP_MSG_OPEN;
	payload->dev_num = HDDP_DEV_NUM_GEN(0, 0);
	comm_msg(DRV_PID_HDDP, &msg);

	msg.type = HDDP_MSG_OPEN;
	payload->dev_num = HDDP_DEV_NUM_GEN(1, 'a');
	comm_msg(DRV_PID_HDDP, &msg);

	printk("\nOperation Finished!\n");
*/
	rtc ret;
	struct proc_msg msg;
	struct ipc_msg_payload_fs *ptr_payload;
	uint32 src;
	uint32 fs_mbr_index; /* fs partition mbr index */
	uint32 fs_logical_index; /* fs partition logical index */

	/* Check Init. flag */
	if (NOT(IS_TRUE(fs_is_init)))
		panic("FS DRIVER IS NOT INITIALIZED!\n");

	while(1) {
		/* Receive message from other processes */
		recv_msg(IPC_PROC_ALL, &msg);
		src = msg.src;
		ptr_payload = (struct ipc_msg_payload_fs *)msg.payload;
		printk("FS MSG TYPE: 0x%x\n", msg.type);

		/* Get partition table index */
		fs_mbr_index = FS_GET_MBR_NUM(ptr_payload->dev_num);
		fs_logical_index = FS_GET_LOGICAL_NUM(ptr_payload->dev_num);
		/* Validate */
		if (fs_mbr_index >= FS_MAX_MBR_P_CNT)
			panic("FS - MBR INDEX OUT OF BOUND!\n");
		/* NOTE: Valid range of logical index is 0~16 */
		if (fs_logical_index > PART_MAX_L_PER_EX_PART)
			panic("FS - LOGICAL INDEX OUT OF BOUND!\n");

		/* Check message type */
		switch(msg.type) {
		case FS_MSG_OPEN:
			fs_dev_open(ptr_payload);
			break;

		case FS_MSG_WRITE:
			break;

		case FS_MSG_READ:
			break;

		case FS_MSG_CLOSE:
			fs_dev_close(ptr_payload);
			break;

		case FS_MSG_IOCTL:
			/* fs_dev_ioctl(ptr_payload); */
			break;

		default:
			panic("FS RECEIVED UNKNOWN MESSAGE!\n");
		}

		/* Response message */
		msg.type = FS_MSG_OK;
		send_msg(src, &msg);
	}
}
