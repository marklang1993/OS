#include "lib.h"
#include "part_table.h"
#include "drivers/fs.h"
#include "drivers/hdd_part.h"

/* File System Driver Message Dispatcher */
void fs_message_dispatcher(void)
{
	struct proc_msg msg;
	struct ipc_msg_payload_hddp *payload;
	struct ipc_msg_payload_hddp_get_info *ret_payload;

	payload = (struct ipc_msg_payload_hddp *)msg.payload;

	msg.type = HDDP_MSG_OPEN;
	payload->dev_num = HDDP_DEV_NUM_GEN(0, 0);
	comm_msg(DRV_PID_HDDP, &msg);

	msg.type = HDDP_MSG_OPEN;
	payload->dev_num = HDDP_DEV_NUM_GEN(1, 'a');
	comm_msg(DRV_PID_HDDP, &msg);

	msg.type = HDDP_MSG_IOCTL;
	payload->dev_num = HDDP_DEV_NUM_GEN(1, 'a');
	payload->ioctl_msg = HDDP_IMSG_GET_INFO;
	comm_msg(DRV_PID_HDDP, &msg);

	ret_payload = (struct ipc_msg_payload_hddp_get_info *)payload;
	printk("is_bootable: %d, type: %x, cnt: %d, rev: %d\n",
		ret_payload->is_bootable,
		ret_payload->type,
		ret_payload->cnt_sectors,
		ret_payload->rev_sectors
		);

	printk("\nOperation Finished!\n");

        while(1);
}
