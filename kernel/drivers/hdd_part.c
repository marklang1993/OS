#include "dbg.h"
#include "lib.h"
#include "drivers/hdd_part.h"

#define COPY_BUF(dst, src, size) \
        kassert(!ENABLE_SPLIT_KUSPACE); \
        memcpy((void *)(dst), (void *)(src), size)


/* Harddisk partition descriptor */
struct hdd_partition_descriptor
{
	BOOL is_valid; /* If not ture, others are invalid */
	BOOL is_open;
	BOOL is_bootable;
	uint32 type;
	uint32 base_sector;
	uint32 cnt_sectors;
};

/* Harddisk MBR partition descriptor */
struct hdd_mbr_partition_descriptor
{
	struct hdd_partition_descriptor main;
	struct hdd_partition_descriptor logicals[PART_MAX_L_PER_EX_PART];
};

/* Harddisk partition table */
static struct hdd_mbr_partition_descriptor hdd_part_table[HDDP_MAX_CNT];


/*
 # Init. hdd partition descriptor
 @ ptr_descriptor       : pointer to hdd_partition_descriptor initialized
 @ ptr_part_table_entry : raw partition table entry
 @ base_sector_offset   : base_sector_lba offset
 */
static void hddp_descriptor_init(
	struct hdd_partition_descriptor *ptr_descriptor,
	struct partition_table_entry const *ptr_part_table_entry,
	const uint32 base_sector_offset
)
{
	/* MUST be a valid partition table entry */
	kassert(ptr_part_table_entry->type != 0);

	/* Init. */
	ptr_descriptor->is_valid = TRUE;
	ptr_descriptor->is_open = FALSE;
	ptr_descriptor->is_bootable =
		(ptr_part_table_entry->is_bootable & PART_BOOTABLE) != 0 ?
		TRUE : FALSE;
	ptr_descriptor->type = ptr_part_table_entry->type;
	ptr_descriptor->base_sector = ptr_part_table_entry->base_sector_lba +
				base_sector_offset;
	ptr_descriptor->cnt_sectors = ptr_part_table_entry->cnt_sectors;

	printk("base: %d, cnt: %d\n", ptr_descriptor->base_sector,
		ptr_descriptor->cnt_sectors);
}


/*
 # HDD read bytes
 @ hdd_dev_num : hdd minor device number
 @ pos         : base address
 @ size        : size of bytes read
 @ buf         : buffer address
 */
static rtc hdd_read(
	uint32 hdd_dev_num,
	uint64 pos,
	uint32 size,
	void *buf
)
{
	struct proc_msg msg;
	struct ipc_msg_payload_hdd *ptr_payload_hdd; /* HDD Driver Payload */

	/* Send READ message to HDD driver */
	msg.type = HDD_MSG_READ;
	ptr_payload_hdd = (struct ipc_msg_payload_hdd *)msg.payload;
	ptr_payload_hdd->dev_num = hdd_dev_num;
	ptr_payload_hdd->base_low = (uint32)(pos & 0xffffffffull);
	ptr_payload_hdd->base_high = (uint32)((pos & 0xffffffff00000000ull) >> 32);
	ptr_payload_hdd->size = size;
	ptr_payload_hdd->buf_address = buf;

	comm_msg(DRV_PID_HDD, &msg);
	/* Check result */
	if (HDD_MSG_ERROR == msg.type)
		panic("HDDP: HDD READ ERROR!");

	return OK;
}


/*
 # HDDP_OPEN message handler
 @ param : hdd partition open parameters
 */
static void hddp_dev_open(struct ipc_msg_payload_hdd_part *param)
{
	struct proc_msg msg;
	struct ipc_msg_payload_hdd *ptr_payload_hdd; /* HDD Driver Payload */
	struct partition_table_entry main_part_table_buf[COUNT_M_PART_TABLE_ENTRY];
	struct partition_table_entry logic_part_table_buf[COUNT_L_PART_TABLE_ENTRY];
	struct hdd_partition_descriptor *ptr_last_part_descriptor;
	uint32 hdd_dev_num; /* hdd minor device number */
	uint32 hdd_part_table_base; /* hdd_part_table base index */
	uint32 extended_part_sector_offset; /* extended partition table offset in sectors */
	uint64 extended_part_offset; /* extended partition table offset in bytes */
	rtc ret;
	uint32 i, j;

	hdd_dev_num = HDDP_GET_MBR_NUM(param->dev_num) / HDDP_MBR_FACTOR;
	hdd_part_table_base = hdd_dev_num * HDDP_MBR_FACTOR;
	/* Send OPEN message to HDD driver */
	msg.type = HDD_MSG_OPEN;
	ptr_payload_hdd = (struct ipc_msg_payload_hdd *)msg.payload;
	ptr_payload_hdd->dev_num = hdd_dev_num;
	comm_msg(DRV_PID_HDD, &msg);

	/* Read MBR partition table */
	ret = hdd_read(hdd_dev_num,
			BASE_PARTITION_TABLE,
			PARTITION_TABLE_ENTRY_SIZE * COUNT_M_PART_TABLE_ENTRY,
			main_part_table_buf);

	/* Process MBR partition table */
	for (i = 0; i < COUNT_M_PART_TABLE_ENTRY; ++i) {
		/* Check current raw partition table entry is empty */
		if (0 == main_part_table_buf[i].type)
			continue;

		hddp_descriptor_init(
			&hdd_part_table[i + hdd_part_table_base].main,
			&main_part_table_buf[i],
			0
			);

		/* Check current MBR partition is EXTENDED partition */
		if (PART_TYPE_EXTENDED !=
			hdd_part_table[i + hdd_part_table_base].main.type)
			continue;

		/* Continue to process logical partitions */
		extended_part_sector_offset =
			hdd_part_table[i + hdd_part_table_base].main.base_sector;
		extended_part_offset = extended_part_sector_offset *
					HDD_BYTES_PER_SECTOR +
					BASE_PARTITION_TABLE;
		/* Read partition table entry of 1st logical partition */
		ret = hdd_read(hdd_dev_num,
				extended_part_offset,
				PARTITION_TABLE_ENTRY_SIZE * COUNT_L_PART_TABLE_ENTRY,
				logic_part_table_buf);

		/* Check current raw partition table entry is empty */
		if (0 == logic_part_table_buf[0].type)
			continue;

		/* Process this partition table entry */
		j = 0;
		hddp_descriptor_init(
			&hdd_part_table[i + hdd_part_table_base].logicals[j],
			&logic_part_table_buf[0],
			extended_part_sector_offset
			);

		/* Check next logical partition table */
		while (PART_TYPE_EXTENDED == logic_part_table_buf[1].type) {
			ptr_last_part_descriptor = &hdd_part_table[i + hdd_part_table_base].logicals[j];
			/* Read next logical partition table */
			ret = hdd_read(
				hdd_dev_num,
				extended_part_offset
				+ logic_part_table_buf[1].base_sector_lba
				* HDD_BYTES_PER_SECTOR,
				PARTITION_TABLE_ENTRY_SIZE * COUNT_L_PART_TABLE_ENTRY,
				logic_part_table_buf);

			/* Check current raw partition table entry is empty */
			if (0 == logic_part_table_buf[0].type)
				break;

			/* Process this partition table entry */
			++j;
			hddp_descriptor_init(
				&hdd_part_table[i + hdd_part_table_base].logicals[j],
				&logic_part_table_buf[0],
				ptr_last_part_descriptor->base_sector
				+ ptr_last_part_descriptor->cnt_sectors
			);
		}
	}
}


/*
 # Initialize Harddisk Partition Driver (Ring 0)
 */
void hddp_init(void)
{
	memset(hdd_part_table, 0x0, sizeof(hdd_part_table));
}


/*
 # Harddisk Partition Driver Message Dispatcher
 */
void hddp_message_dispatcher(void)
{
	rtc ret;
	struct proc_msg msg;
	uint32 src;

	while(1) {
		/* Receive message from other processes */
		recv_msg(IPC_PROC_ALL, &msg);
		src = msg.src;
		printk("HDDP MSG TYPE: 0x%x\n", msg.type);

		/* Check message type */
		switch(msg.type) {
		case HDDP_MSG_OPEN:
			hddp_dev_open((struct ipc_msg_payload_hdd_part *)msg.payload);
			break;

		case HDDP_MSG_WRITE:
			break;

		case HDDP_MSG_READ:
			break;

		case HDDP_MSG_CLOSE:
			break;

		default:
			panic("HDDP RECEIVED UNKNOWN MESSAGE!\n");
		}

		/* Response message */
		msg.type = HDDP_MSG_OK;
		send_msg(src, &msg);
	}
}
