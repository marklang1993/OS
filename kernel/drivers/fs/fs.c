#include "dbg.h"
#include "lib.h"
#include "drivers/fs/fs.h"
#include "drivers/fs/fs_lib.h"

/* File System Partition Status */
#define FS_INVALID		0	/* Did not open */
#define FS_UNRECOG		1	/* Open, but cannot r/w, limited ioctl */
#define FS_VALID		2	/* Open, all operations are available */

/* Calculate byte offset w.r.t block index */
#define BLOCK2BYTE(block_idx)	(((uint64)(block_idx)) * FS_BYTES_PER_BLOCK)

#define COPY_BUF(dst, src, size) \
	kassert(!ENABLE_SPLIT_KUSPACE); \
	memcpy((void *)(dst), (void *)(src), size)

/* Get fs partition descriptor & Check open status */
#define PRE_DEV_USE \
	get_descriptor_ptr(param, &ptr_descriptor); \
	if (0 == ptr_descriptor->ref_cnt) \
		panic("FS - PARTITION %x NOT OPEN ERROR!\n", \
			param->dev_num)

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
static BOOL fs_is_init = FALSE;	/* Flag: is file system driver initialized */
static struct fs_mbr_partition_descriptor fs_part_table[FS_MAX_MBR_P_CNT];	/* FS partition table */

/* FS global variables */
struct file_table_entry file_table[FILE_TABLE_SIZE];	/* File Table */


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
 # Get pointer of fs partition descriptor
 @ param         : fs partition parameters
 @ pp_descriptor : pointer to pointer of fs partition descriptor
 */
static void get_descriptor_ptr(
	const struct ipc_msg_payload_fs *param,
	struct fs_partition_descriptor **const pp_descriptor
)
{
	uint32 fs_mbr_index, fs_logical_index;
	struct fs_partition_descriptor *ptr_descriptor;

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

	/* Return descriptor pointer */
	*pp_descriptor = ptr_descriptor;
}


/*
 # FS_OPEN message handler
 @ param : fs partition open parameters
 */
static void fs_dev_open(struct ipc_msg_payload_fs *param)
{
	struct fs_partition_descriptor *ptr_descriptor;
	uint32 fs_mbr_index, fs_logical_index;
	struct ipc_msg_payload_hddp_get_info hddp_info;

	struct proc_msg msg;
	struct ipc_msg_payload_hddp *payload;
	struct fs_hdd_op hdd_op_param;

	/* Get fs partition descriptor */
	get_descriptor_ptr(param, &ptr_descriptor);
	/* Get fs partition table index */
	fs_mbr_index = FS_GET_MBR_NUM(param->dev_num);
	fs_logical_index = FS_GET_LOGICAL_NUM(param->dev_num);

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
		ptr_descriptor->sector_cnt = hddp_info.cnt_sectors;

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

			/*
			printk("Superblock Contents:\n");
			printk("Magic Number: 0x%x\tSize: %d\n",
				ptr_descriptor->sb.magic_num,
				ptr_descriptor->sb.size);
			printk(" - Log: %d, %d\n",
				ptr_descriptor->sb.log_start,
				ptr_descriptor->sb.log_cnt);
			printk(" - dinode Map: %d, %d\n",
				ptr_descriptor->sb.dinode_map_start,
				ptr_descriptor->sb.dinode_map_cnt);
			printk(" - dinode: %d, %d\n",
				ptr_descriptor->sb.dinode_start,
				ptr_descriptor->sb.dinode_cnt);
			printk(" - Data Map: %d, %d\n",
				ptr_descriptor->sb.data_map_start,
				ptr_descriptor->sb.data_map_cnt);
			printk(" - Data: %d, %d\n",
				ptr_descriptor->sb.data_start,
				ptr_descriptor->sb.data_cnt);
			printk(" - Total dinode Count: %d\n",
				ptr_descriptor->sb.dinode_tot_cnt);
			*/
		} else {
			/* Unrecognizable file system */
			ptr_descriptor->status = FS_UNRECOG;
			printk("FS UNRECOG!\n");
		}

	} else {
		/* Already Init. */
		kassert(ptr_descriptor->status == FS_VALID);
	}

	/* Increase ref_cnt */
	ptr_descriptor->ref_cnt += 1;
}


/*
# FS_CLOSE message handler
@ param : fs partition close parameters
*/
static void fs_dev_close(struct ipc_msg_payload_fs *param)
{
	struct fs_partition_descriptor *ptr_descriptor;

	struct proc_msg msg;
	struct ipc_msg_payload_hddp *payload;

	/* Get fs partition descriptor and check */
	PRE_DEV_USE;

	/* Decrease the reference count */
	ptr_descriptor->ref_cnt -= 1;

	/* Do some clean if this fs descriptor is no longer used */
	if (0 == ptr_descriptor->ref_cnt) {
		/* Clear the fs descriptor */
		memset(ptr_descriptor, 0x0, sizeof(struct fs_partition_descriptor));
		/* Close the corresponding hdd partition */
		payload = (struct ipc_msg_payload_hddp *)msg.payload;
		msg.type = HDDP_MSG_CLOSE;
		payload->dev_num = HDDP_DEV_NUM_GEN(
				FS_GET_MBR_NUM(param->dev_num),
				LOGICAL_CONVERT(FS_GET_LOGICAL_NUM(param->dev_num))
				);
		comm_msg(DRV_PID_HDDP, &msg);
		/* Check result */
		if (HDDP_MSG_OK != msg.type)
			panic("FS CLOSE DEV - CLOSE PARTITION ERROR!\n");
	}
}


/*
 # Initialize File System Driver (Ring 0)
 */
void fs_init(void)
{
	/* Init. inode */
	inode_init();

	/* Init. fs partition table */
	memset(fs_part_table, 0x0, sizeof(fs_part_table));
	
	/* Init. file table */
	memset(file_table, 0x0, sizeof(file_table));

	/* Set Init. flag */
	fs_is_init = TRUE;
}


/*
 # Make file system
 @ param          : pointer to fs message payload
 @ ptr_descriptor : pointer to fs partition descriptor
 */
static void fs_ioctl_mkfs(
	struct ipc_msg_payload_fs *const param,
	struct fs_partition_descriptor *ptr_descriptor
)
{
	uint32 fs_mbr_index, fs_logical_index;
	struct fs_hdd_op hdd_op_param;
	struct superblock *sb_ptr = &ptr_descriptor->sb;
	byte dummy_data[FS_BYTES_PER_BLOCK];
	void *dummy_block_ptr = &dummy_data[0];
	byte *tmp_block_ptr = &dummy_data[0];

	int i;
/*
	printk("dummy_data SIZE: %d\n", sizeof(dummy_data));
	printk("address of dummy_data start: 0x%x\n", (uint32)(&dummy_data[0]));
	printk("address of dummy_data end: 0x%x\n", (uint32)(&dummy_data[FS_BYTES_PER_BLOCK - 1]));
*/

	/* Init. */
	memset(dummy_block_ptr, 0x0, sizeof(dummy_data));

	/* 1. Build superblock */
	printf("FS: Building superblock.");
	build_superblock(ptr_descriptor);

	/* Get partition table index */
	fs_mbr_index = FS_GET_MBR_NUM(param->dev_num);
	fs_logical_index = FS_GET_LOGICAL_NUM(param->dev_num);

	/* Write Superblock to the corresponding partition */
	hdd_op_param.fs_mbr_index = fs_mbr_index;
	hdd_op_param.fs_logical_index = fs_logical_index;
	hdd_op_param.base = BLOCK2BYTE(SUPERBLOCK_IDX);
	hdd_op_param.size = sizeof(struct superblock);
	hdd_op_param.buf_address = sb_ptr;
	hdd_op_param.is_read = FALSE;
	hdd_op(&hdd_op_param);
	printf("Done\n");

	/* 2. TODO: Clean log blocks */

	/* 3. Clean dinode bitmap blocks */
	printf("FS: Clean dinode bitmap blocks.");
	hdd_op_param.fs_mbr_index = fs_mbr_index;
	hdd_op_param.fs_logical_index = fs_logical_index;
	hdd_op_param.size = sizeof(dummy_data);
	hdd_op_param.buf_address = dummy_block_ptr;
	hdd_op_param.is_read = FALSE;
	for(i = 0; i < sb_ptr->dinode_map_cnt; ++i)
	{
		hdd_op_param.base = BLOCK2BYTE(i + sb_ptr->dinode_map_start);
		hdd_op(&hdd_op_param);
	}
	printf("Done\n");
	
	/* 4. Clean data bitmap blocks */
	printf("FS: Clean data bitmap blocks.");
	for(i = 0; i < sb_ptr->data_map_cnt; ++i)
	{
		hdd_op_param.base = BLOCK2BYTE(i + sb_ptr->data_map_start);
		hdd_op(&hdd_op_param);
	}
	printf("Done\n");

	/* 5. Init. 1st dinode with root directory data block */
	printf("FS: Init. 1st dinode.");
	build_1st_dinode(tmp_block_ptr);
	/* Write to harddisk */
	hdd_op_param.buf_address = tmp_block_ptr;
	hdd_op_param.base = BLOCK2BYTE(sb_ptr->dinode_start);
	hdd_op(&hdd_op_param);
	printf("Done\n");

	/* 6. Update dinode bitmap */
	printf("FS: Update dinode bitmap.");
	memset(tmp_block_ptr, 0, sizeof(dummy_data));
	dummy_data[0] = 1;
	/* Write to harddisk */
	hdd_op_param.buf_address = tmp_block_ptr;
	hdd_op_param.base = BLOCK2BYTE(sb_ptr->dinode_map_start);
	hdd_op(&hdd_op_param);
	printf("Done\n");
}


/*
 # FS_IOCTL message handler
 @ param : File system partition ioctl parameters
 */
static void fs_dev_ioctl(struct ipc_msg_payload_fs *const param)
{
	struct fs_partition_descriptor *ptr_descriptor;

	/* Get fs partition descriptor and check */
	PRE_DEV_USE;

	/* Determine ioctl message type */
	switch (param->ioctl_msg) {

	case FS_IMSG_MKFS:
		/* Make file system */
		fs_ioctl_mkfs(param, ptr_descriptor);
		break;

	default:
		panic("FS - IOCTL RECEIVED UNKNOWN MESSAGE!\n");
		break;
	}
}


/*
 # File System Driver Message Dispatcher
 */
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
		/* printk("FS MSG TYPE: 0x%x\n", msg.type); */

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
			fs_dev_ioctl(ptr_payload);
			break;

		default:
			panic("FS RECEIVED UNKNOWN MESSAGE!\n");
		}

		/* Response message */
		msg.type = FS_MSG_OK;
		send_msg(src, &msg);
	}
}
