#include "lib.h"
#include "drivers/fs.h"
#include "drivers/hdd.h"

/* File System Driver Message Dispatcher */
void fs_message_dispatcher(void)
{
	struct proc_msg msg;
	struct ipc_msg_payload_hdd *payload;
	uint8 data[0xa00];
	uint32 i;

        msg.msg_type = HDD_MSG_OPEN;
        comm_msg(DRV_PID_HDD, &msg);

	msg.msg_type = HDD_MSG_WRITE;
	memset(data, 0x11, 0xa00);
	payload = (struct ipc_msg_payload_hdd *)msg.payload;
	payload->is_master = FALSE;
	payload->pos = 1;
	payload->count = 5;
	payload->buf_address = data;
	comm_msg(DRV_PID_HDD, &msg);

	msg.msg_type = HDD_MSG_READ;
	payload = (struct ipc_msg_payload_hdd *)msg.payload;
	payload->is_master = TRUE;
	payload->pos = 0;
	payload->count = 1;
	payload->buf_address = data;
	comm_msg(DRV_PID_HDD, &msg);

	printk("Data:");
	for (i = 0; i < 512; ++i) {
		if (i % 16 == 0) {
			printk("\nROW:%x  ", i/16);
		}
		printk("%x ", data[i]);
	}

	printk("\nContinue to run fs.c\n");
        while(1);
}
