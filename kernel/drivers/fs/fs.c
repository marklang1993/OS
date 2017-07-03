#include "lib.h"
#include "drivers/fs/fs.h"
#include "drivers/fs/inode.h"

/* File System Driver Message Dispatcher */
void fs_message_dispatcher(void)
{
	struct indir_block_ref ref;
/*
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
*/
	printk("DINODE SIZE: %d\n", DINODE_SIZE);
	printk("INDIR_REF SIZE: %d\n", sizeof(ref));
        while(1);
}
