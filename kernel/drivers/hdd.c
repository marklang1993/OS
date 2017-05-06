#include "dbg.h"
#include "io_port.h"
#include "lib.h"
#include "drivers/i8259a.h"
#include "drivers/hdd.h"

/* ATA Channel */
#define IDE_CH_IS_PRIMARY		1	/* Only Primary IDE channel is supported */
#define IDE_CH_MASK			(IDE_CH_IS_PRIMARY << 7)

/* HDD Ports (WRITE_READ) */
#define PORT_HDD_DATA			(0x170 | IDE_CH_MASK)
#define PORT_HDD_FEATURE		(0x171 | IDE_CH_MASK)
#define PORT_HDD_ERROR			PORT_HDD_FEATURE
#define PORT_HDD_SECTOR_CNT		(0x172 | IDE_CH_MASK)
#define PORT_HDD_LBA_L			(0x173 | IDE_CH_MASK)
#define PORT_HDD_LBA_M			(0x174 | IDE_CH_MASK)
#define PORT_HDD_LBA_H			(0x175 | IDE_CH_MASK)
#define PORT_HDD_DEV			(0x176 | IDE_CH_MASK)
#define PORT_HDD_CMD			(0x177 | IDE_CH_MASK)
#define PORT_HDD_STATUS			PORT_HDD_CMD
#define PORT_HDD_DEV_CTRL		(0x376 | IDE_CH_MASK)
#define PORT_HDD_ALT_STATUS		PORT_HDD_DEV_CTRL

/* HDD BIOS Physical Address */
#define BIOS_HDD_CNT_DRV		0x475	/* Physical address for getting count of drivers */

/* HDD commands */
#define HDD_IDENTIFY			0xec

/* HDD Macros */
#define HDD_DRV_MASTER			0
#define HDD_DRV_SLAVE			1
#define HDD_TIMEOUT			500	/* *10ms */
#define HDD_MAX_DRIVES			2	/* Primary IDE: Master + Slave */

/* # HDD internal struct/union */

/* ATA IDENTIFY struct (Incomplete) */
#pragma pack(push, 1)
struct ata_identify
{
	uint16 skip0;
	uint16 cnt_cylinders;
	uint16 skip1;
	uint16 cnt_headers;
	uint16 skip2[2];
	uint16 cnt_sectors;
	uint16 vendor_id_1[3];
	char serial_number[20];
	uint16 skip3[3];
	char firmware_rev[8];
	char model_number[40];
	char skip4;
	char vendor_id_2;
	uint16 skip5;
	struct {
		char skip0;
		uint16 is_DMA_sup:1;
		uint16 is_LBA_sup:1;
		uint16 skip1:6;
		uint16 skip2;
	} capabilities;
	uint16 skip6[9];
	uint32 cnt_addressable_sectors;
};
#pragma pack(pop)

/* 1. HDD Device Register */
union hdd_dev_reg
{
	uint8 data;		/* # Register data */

	uint8 lba_haddr:4;	/* CHS: Header number 
				 * LBA: LBA28 high address: bit 24~27
				 */
	uint8 drv_sel:1;	/* DRV selector: master OR slave */
	uint8 fix_1:1;
	uint8 is_lba:1;		/* LBA enable flag */
	uint8 fix_2:1;
};
#define HDD_DEV_REG_GEN(is_lba, drv_sel, hs) \
	(0xa0 | ((is_lba & 0x1) << 6) | ((drv_sel & 0x1) << 4) | \
	((is_lba ? (hs >> 24) : hs) & 0xf))
/* NOTE: if LBA, "hs" is 28-bit LBA address! */

/* 2. HDD Status Register */
union hdd_status_reg
{
	uint8 data;		/* # Register data */

	uint8 error:1;		/* Error bit */
	uint8 obsolete:2;	/* Obsolete bits */
	uint8 data_req:1;	/* Data Request - ready to transfer data */
	uint8 param:1;		/* Command dependent - formerly DSC bit */
	uint8 df_se:1;		/* Device fault / Stream error */
	uint8 dev_rdy:1;	/* Device ready */
	uint8 busy:1;		/* Busy. other bits are invalid if set */
};

/* 3. HDD Device Control Register */
union hdd_dev_ctrl_reg
{
	uint8 data;		/* # Register data */

	uint8 zero:1;		/* Always 0 */
	uint8 int_en:1;		/* Interrupt enable */
	uint8 sw_rst:1;		/* Software reset */
	uint8 invalid:4;	/* Invalid bits */
	uint8 hob:1;		/* High order byte - defined by 48-bit address feature set */
};

/* #. HDD All Control Registers (WRITE) Struct
 * NOTE: this does not include HDD_DATA
 */
struct hdd_ctrl_regs
{
	union hdd_dev_ctrl_reg dev_ctrl;
	uint8 feature;
	uint8 sector_cnt;
	uint8 lba_l;
	uint8 lba_m;
	uint8 lba_h;
	union hdd_dev_reg dev;
	union hdd_status_reg cmd;
};


/* HDD internal global variables */
static uint8 hdd_status;
static struct ata_identify identify_info;


/*
 # Processing ata identity string
 @ str: raw string
 @ len: length of raw string
 */
static void ata_id_str_process(char *str, uint32 len)
{
	uint32 i, j;
	uint32 cnt_pairs = len / 2;
	char tmp;

	/* Swap high byte and low byte within a word */
	for (i = 0; i < cnt_pairs; ++i)
	{
		j = i * 2;
		tmp = str[j];
		str[j] = str[j + 1];
		str[j + 1] = tmp;
	}

	/* Add end mark of a c-style string */
	str[len - 1] = '\0';
}


/*
 # HDD wait for device ready
 */
static rtc hdd_wait_busy(void)
{
	union hdd_status_reg status;

	/* use volatile here? */
	while (1) {
		io_in_byte(PORT_HDD_STATUS, &status.data);
		printk("status: %x\n", status.data);
		if (!status.busy) {
			return OK;
		}
	}

	return ETIMEOUT;
}


/*
 # HDD send command
 @ ptr_hdd_ctrl: pointer to a hdd_ctrl_regs struct
 */
static void hdd_send_cmd(struct hdd_ctrl_regs *ptr_hdd_ctrl)
{
	if (hdd_wait_busy() != OK)
		panic("HDD TIME OUT!");

	printk("HDD SEND CMD\n");

	/* Write hdd device control register */
	io_out_byte(PORT_HDD_DEV_CTRL, ptr_hdd_ctrl->dev_ctrl.data);
	/* Write hdd parameters */	
	io_out_byte(PORT_HDD_FEATURE, ptr_hdd_ctrl->feature);
	io_out_byte(PORT_HDD_SECTOR_CNT, ptr_hdd_ctrl->sector_cnt);
	io_out_byte(PORT_HDD_LBA_L, ptr_hdd_ctrl->lba_l);
	io_out_byte(PORT_HDD_LBA_M, ptr_hdd_ctrl->lba_m);
	io_out_byte(PORT_HDD_LBA_H, ptr_hdd_ctrl->lba_h);
	io_out_byte(PORT_HDD_DEV, ptr_hdd_ctrl->dev.data);
	/* Write hdd command */	
	io_out_byte(PORT_HDD_CMD, ptr_hdd_ctrl->cmd.data);
}


/*
 # HDD_OPEN message handler
 */
static void hdd_dev_open(void)
{
	struct hdd_ctrl_regs ctrl_regs;
	uint16 buf[256];	/* Store raw identify data */

	uint32 capacities;
	char str_yes[] = "YES";
	char str_no[] = "NO";

	/* Prepare the command */
	memset(&ctrl_regs, 0, sizeof(struct hdd_ctrl_regs));
	ctrl_regs.dev_ctrl.data = 0; /* Activate */
	ctrl_regs.dev.data = HDD_DEV_REG_GEN(0, HDD_DRV_MASTER, 0);
	ctrl_regs.cmd.data = HDD_IDENTIFY;
	/* Send command & wait for hdd interrupt */
	hdd_send_cmd(&ctrl_regs);
	wait_int();

	/* After hdd interrupt, read data */
	io_bulk_in_word(PORT_HDD_DATA, buf, (sizeof(buf) / sizeof(uint16)));

	printk("HDD READ DATA FINISH!\n");

	/* Format data - discard redundant parts */
	memcpy(&identify_info, buf, sizeof(struct ata_identify));

	/* Print data */
	ata_id_str_process(
		identify_info.serial_number,
		sizeof(identify_info.serial_number));
	printk("HDD S/N: %s\n", identify_info.serial_number);
	ata_id_str_process(identify_info.model_number,
		sizeof(identify_info.model_number));
	printk("HDD MODEL: %s\n", identify_info.model_number);
	printk("C/H/S: %u/%u/%u\n",
		identify_info.cnt_cylinders,
		identify_info.cnt_headers,
		identify_info.cnt_sectors);
	capacities = identify_info.cnt_addressable_sectors * HDD_BYTES_PER_SECTOR / 1000000;
	printk("Capacities: %u MB\n", capacities);
	printk("DMA: %s\t",
		identify_info.capabilities.is_DMA_sup == 1 ?
		str_yes : str_no);
	printk("LBA: %s\n",
		identify_info.capabilities.is_LBA_sup == 1 ?
		str_yes : str_no);
}


/*
 # Initialize Harddisk Driver (Ring 0)
 */
void hdd_init(void)
{
	uint8 cnt_hdd;

	/* Get count of harddisk drive */
	cnt_hdd = *((uint8 *)BIOS_HDD_CNT_DRV);

	if (0 == cnt_hdd) {
		panic("NO HARDDISK DRIVE FOUND!");
	}

	/* Init. harddisk interrupt */
	i8259a_set_handler(INDEX_8259A_HDD, hdd_interrupt_handler);
	i8259a_int_enable(INDEX_8259A_HDD);
	i8259a_int_enable(INDEX_8259A_SLAVE);	/* IMPORTANT! */
}


/*
 # Harddisk Interrupt Handler (Ring 0)
 */
void hdd_interrupt_handler(void)
{
	/* Read HDD status */
	io_in_byte(PORT_HDD_STATUS, &hdd_status);

	printk("HDD INTERRUPT HANDLED!\n");

	/* Resume HDD driver function */
	resume_int(DRV_PID_HDD);
}


/*
 # Harddisk Driver Message Dispatcher
 */
void hdd_message_dispatcher(void)
{
	rtc ret;
	struct proc_msg msg;
	uint32 src;

	while(1) {
		/* Receive message from other processes */
		recv_msg(IPC_PROC_ALL, &msg);
		printk("HDD MSG TYPE: 0x%x\n", msg.msg_type);

		/* Check message type */
		switch(msg.msg_type) {
		case HDD_MSG_OPEN:
			hdd_dev_open();
			msg.msg_type = HDD_MSG_OK;
			break;

		case HDD_MSG_WRITE:
			break;
		case HDD_MSG_READ:
			break;
		case HDD_MSG_CLOSE:
			break;
		default:
			assert(0);
		}

		/* Response message */
		send_msg(src, &msg);
	}
}
