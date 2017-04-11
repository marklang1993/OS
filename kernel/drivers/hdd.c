#include "dbg.h"
#include "io_port.h"
#include "drivers/i8259a.h"
#include "drivers/hdd.h"

/* ATA Channel */
#define IDE_CH_IS_PRIMARY		1
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
#define HDD_TIMEOUT			500  /* *10ms */


/* HDD internal global variables */
static uint8 hdd_status;

/* HDD internal struct/union */

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

/*
 # HDD wait for device ready
 */
static rtc hdd_wait_busy(void)
{
	union hdd_status_reg status;

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
	uint16 buf[256];	/* Store identify data */
	uint32 i;

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

	for(i = 0; i < 256; ++i)
	{
		printk("%x", buf[i]);
	}

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
