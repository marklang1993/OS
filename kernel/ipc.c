#include "dbg.h"
#include "lib.h"
#include "proc.h"
#include "syscall.h"

#define COPY_MSG(dst, src) \
	kassert(!ENABLE_SPLIT_KUSPACE);	\
	memcpy((void *)(dst), (void *)(src), sizeof(struct proc_msg))

#define COPY_ARG()	kassert(!ENABLE_SPLIT_KUSPACE)


/* kernel.c */
extern struct process user_process[];
/* proc.c */
extern struct process *current_user_process;


/*
 # Block a process
 @ ptr_proc : operated process
 */
static void block(struct process *ptr_proc)
{
	;
}

/*
 # Unblock a process
 @ ptr_proc : operated process
 */
static void unblock(struct process *ptr_proc)
{
	;
}

/*
 # Check deadlock: is there a cycle in the linked list
   of sending message processes
 @ dst : destination process id
 */
static rtc is_deadlock(uint32 dst)
{
	struct process *fast, *slow;

	/* Init. "fast" and "slow" pointer & current_user_process */
	fast = current_user_process;
	slow = current_user_process;
	current_user_process->proc_sending_to = &user_process[dst];

	while (fast != NULL && fast->proc_sending_to != NULL) {
		slow = slow->proc_sending_to;
		fast = fast->proc_sending_to->proc_sending_to;
		if (slow == fast) {
			/* "fast" catches up "slow" - cycle detected */
			current_user_process->proc_sending_to = NULL;
			return EDLOCK;
		}
	}

	current_user_process->proc_sending_to = NULL;
	return OK;
}

/*
 # IPC - send message (Ring 1/3)
 @ dst     : destination pid (NOT SUPPORT BROADCAST - IPC_PROC_ALL)
 @ ptr_msg : pointer process message
 */
rtc send_msg(uint32 dst, struct proc_msg* ptr_msg)
{
	rtc ret;
	sys_call(SYS_CALL_SEND_MSG, &ret, dst, ptr_msg);

	return ret;
}

/*
 # IPC - receive message (Ring 1/3)
 @ src     : source pid (SUPPORT BROADCAST - IPC_PROC_ALL)
 @ ptr_msg : pointer to process message
 */
rtc recv_msg(uint32 src, struct proc_msg* ptr_msg)
{
	rtc ret;
	sys_call(SYS_CALL_RECV_MSG, &ret, src, ptr_msg);

	return ret;
}

/*
 # IPC - send & receive message (Ring 1/3)
 @ target  : target pid
 @ ptr_msg : pointer to process message
 */
rtc comm_msg(uint32 target, struct proc_msg* ptr_msg)
{
	rtc ret;
	ret = send_msg(target, ptr_msg);
	if (ret != OK) {
		return ret;
	}

	ret = recv_msg(target, ptr_msg);
	return ret;
}

/*
 # IPC - send message (Ring 0)
 # (uint32 dst, struct proc_msg* ptr_msg)
 @ dst     : destination pid (NOT SUPPORT BROADCAST - IPC_PROC_ALL)
 @ ptr_msg : pointer process message
 */
rtc sys_call_send_msg(void *base_arg)
{
	rtc ret;
	struct process *dst_proc;	/* Dest. process */

	/* Get arguments */
	uint32 dst = *((uint32 *)base_arg);
	struct proc_msg *ptr_msg = *((struct proc_msg **)(((uint32 *)base_arg) + 1));

	COPY_ARG();
	printk("dst pid: %u, ptr_msg: 0x%x\n", dst, (uint32)ptr_msg);
	printk("msg_src: %u, msg_type: %u\n", ptr_msg->msg_src, ptr_msg->msg_type);

	/* Check PID & Process status  & Self-sending */
	if (dst >= USER_PROCESS_COUNT) {
		return EINVPID;
	}
	dst_proc = &user_process[dst];
	if (dst_proc->status == PROC_UNINIT ||
	    dst_proc->status == PROC_DEAD) {
		/* Dest. process status is incorrect */
		return EINVPID;
	}
	if (dst_proc == current_user_process) {
		return EINVPID;
	}

	/* ASSERT current_user_process status */
	kassert(IPC_PROC_NO == current_user_process->receiver);
	kassert(NULL == ((uint32)current_user_process->proc_sending_to));

	/* Check Deadlock */
	ret = is_deadlock(dst);
	if (ret != OK) {
		return ret;
	}

	/* Check acceptable receiver */
	if (dst_proc->apt_sender != IPC_PROC_ALL) {
		if (dst_proc->apt_sender != dst) {
			return EINVRECV;
		}
	}

	/* Check dest. process is RECEVING */
	if (dst_proc->status == PROC_RECEVING) {
		COPY_MSG(&(dst_proc->msg_buf), ptr_msg);

		kassert(NULL == ((uint32)dst_proc->proc_recving_from));
		dst_proc->proc_recving_from = current_user_process;
		dst_proc->status = PROC_RUNNABLE;

	} else {
		/* Record dest. process information */
		current_user_process->receiver = dst;
		current_user_process->proc_sending_to = dst_proc;
		/* BLOCK current user process */
		current_user_process->status = PROC_SENDING;
		/* Force switch to next runnable process */
		schedule();
	}

	return OK;
}

/*
 # IPC - receive message (Ring 0)
 # (uint32 src, struct proc_msg* ptr_msg)
 @ src     : source pid (SUPPORT BROADCAST - IPC_PROC_ALL)
 @ ptr_msg : pointer to process message
 */
rtc sys_call_recv_msg(void *base_arg)
{
	rtc ret;

	/* Get arguments */
	uint32 src = *((uint32 *)base_arg);
	struct proc_msg *ptr_msg = *((struct proc_msg **)(((uint32 *)base_arg) + 1));
	COPY_ARG();


}
