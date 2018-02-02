#include "drivers/fs/fs.h"
#include "lib.h"

/* 
 # Open a file
 @ filename : full file name and path
 @ flag     : operation flags
 @ RETURN   : -1 means failed, other number is the file descriptor index
 */
FILE open(const char *filename, FILE_OP_FLAG flag) {
	return -1;
}

/*
 # Write bytes to a file
 @ fd   : file descriptor index
 @ buf  : source buffer
 @ size : count of bytes
 @ RETURN : actual write bytes
 */
uint32 write(FILE fd, const void *buf, uint32 size) {
	return 0;
}

/*
 # Read bytes from a file
 @ fd   : file descriptor index
 @ buf  : destination buffer
 @ size : count of bytes
 @ RETURN : actual read bytes
 */
uint32 read(FILE fd, void *buf, uint32 size) {
	return 0;
}

/*
 # Close a file
 @ fd     : file descriptor index
 */
rtc close(FILE fd) {
	return OK;
}

/*
 # Make file system
 @ mbr_index     : MBR partition index
 @ logical_index : Logical partition index
 */
rtc mkfs(uint8 mbr_index, uint8 logical_index) {
    struct proc_msg msg;
	struct ipc_msg_payload_fs *ptr_payload;
	uint32 dev_num;

	/* Initialize */
	ptr_payload = (struct ipc_msg_payload_fs *)&msg.payload;
	dev_num = FS_DEV_NUM_GEN(((uint32)mbr_index), ((uint32)logical_index));

	/* Open FS device */
	msg.type = FS_MSG_OPEN;
	ptr_payload->dev_num = dev_num;
	comm_msg(DRV_PID_FS, &msg);

	/* Send MKFS message to FS driver */
    msg.type = FS_MSG_IOCTL;
	ptr_payload->dev_num = dev_num;
	ptr_payload->ioctl_msg = FS_IMSG_MKFS;
	comm_msg(DRV_PID_FS, &msg);

	/* Close FS device */
	msg.type = FS_MSG_CLOSE;
	ptr_payload->dev_num = dev_num;
	comm_msg(DRV_PID_FS, &msg);

    return OK;
}