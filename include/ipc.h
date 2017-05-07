#ifndef _IPC_H_
#define _IPC_H_

#include "errors.h"
#include "proc.h"

/* IPC pid */
#define IPC_PROC_ALL		USER_PROCESS_COUNT
#define IPC_PROC_NO		USER_PROCESS_COUNT + 1
#define IPC_PROC_INT		USER_PROCESS_COUNT + 2

/* IPC message type */
typedef uint32 IPC_MSG_TYPE;
/* IPC message type generator
 * format:       XX     | XXXX |  XX
 *         magic number | pid  | type
 */
#define IPC_MSG_TYPE_GEN(magic_num, pid, type) \
	(((magic_num & 0xff) << 24) | ((pid & 0xffff) << 8) | (type & 0xff))

/* IPC message */
#define IPC_MSG_CONTENTS_CNT	5

struct proc_msg
{
	uint32		msg_src;	/* Source process pid OR IPC pid */
	IPC_MSG_TYPE	msg_type;	/* Message Type */
	/*
	 * Message Payloads
	 * NOTE: use "uint32" for 4-bytes alignment
	 */
	uint32		payload[IPC_MSG_CONTENTS_CNT];
};


/* IPC - send_msg() / recv_msg() - In Ring 1/3
 * comm_msg(): send & receive message (wrapper)
 */
rtc send_msg(uint32 dst, struct proc_msg *ptr_msg);
rtc recv_msg(uint32 src, struct proc_msg *ptr_msg);
rtc comm_msg(uint32 dst, struct proc_msg *ptr_msg);

/* IPC - interrupt related message processing
 * wait_int()   : wait for interrupt
 * resume_int() : restart the process after interrupt
 */
void wait_int();		/* ring 1/3 */
void resume_int(uint32 pid);	/* ring 0 */

/* IPC - sys_call_send_msg() / sys_call_recv_msg() - In Ring 0 */
rtc sys_call_send_msg(void *base_arg);
rtc sys_call_recv_msg(void *base_arg);


#endif
