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
	do {
		/* If no message is available but unblocked, retry */
		sys_call(SYS_CALL_RECV_MSG, &ret, src, ptr_msg);
	} while (ret == INOMSG);

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
	struct process *dst_proc;	/* Dst. process */
	struct process *next_recv_proc;	/* Next process which receives from */

	/* Get arguments */
	uint32 dst = *((uint32 *)base_arg);
	struct proc_msg *ptr_msg = *((struct proc_msg **)(((uint32 *)base_arg) + 1));
	COPY_ARG();

	/* Check PID & Process status & Self-sending */
	if (dst >= USER_PROCESS_COUNT) {
		return EINVPID;
	}
	dst_proc = &user_process[dst];
	if (dst_proc->status == PROC_UNINIT ||
	    dst_proc->status == PROC_DEAD) {
		/* Dst. process status is incorrect */
		return EINVPID;
	}
	if (dst_proc == current_user_process) {
		/* Self-sending */
		return EINVPID;
	}

	/* Check Deadlock */
	ret = is_deadlock(dst);
	if (ret != OK) {
		return ret;
	}

	/* Record dst. process information */
	kassert(NULL == current_user_process->proc_sending_to);
	current_user_process->proc_sending_to = dst_proc;
	/* Copy the message to the src. process msg. buffer */
	COPY_MSG(&(current_user_process->msg_buf), ptr_msg);
	/* Check dst. process waiting list status */
	if (NULL == dst_proc->proc_next_receive) {
		/* # The dst. process msg. buffer is empty */

		/* Set the next process which needs to receive from */
		dst_proc->proc_next_receive = current_user_process;

	} else {
		/* # The dst. process msg. buffer is full */

		/* Find the end of the waiting list */
		next_recv_proc = dst_proc->proc_next_receive;
		while (NULL != next_recv_proc->proc_next_receive) {
			next_recv_proc = next_recv_proc->proc_next_receive;
		}
		/* Append the current sending process to the end of the waiting list */
		kassert(NULL == current_user_process->proc_next_receive);
		next_recv_proc->proc_next_receive = current_user_process;
	}

	/* Check dst. process is RECEIVING (BLOCKED) */
	if (dst_proc->status == PROC_RECEIVING) {
		/* UNBLOCK dst. process */
		dst_proc->status = PROC_RUNNABLE;
		unblock(dst_proc);
	}

/*
	// Dump Linked-List
	printk("pid: %u; address: 0x%x\n", current_user_process->pid, (uint32)dst_proc);
	next_recv_proc = dst_proc->proc_next_receive;
	while (NULL != next_recv_proc->proc_next_receive) {
		printk("pid: %u; address: 0x%x\n", current_user_process->pid, (uint32)next_recv_proc);
		next_recv_proc = next_recv_proc->proc_next_receive;
	}
	printk("pid: %u; address: 0x%x\n", current_user_process->pid, (uint32)next_recv_proc);
*/

	/* BLOCK current process */
	current_user_process->status = PROC_SENDING;
	block(current_user_process);
	/* Force switch to next runnable process */
	schedule();

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
	struct process *src_proc;	/* Src. process */
	struct process *cur_recv_proc;	/* Current process which receives from */
	struct process *last_recv_proc;	/* Last process which receives from */

	/* Get arguments */
	uint32 src = *((uint32 *)base_arg);
	struct proc_msg *ptr_msg = *((struct proc_msg **)(((uint32 *)base_arg) + 1));
	COPY_ARG();

	/* Check PID & Process status & Self-receiving & Get src_proc */
	if (src != IPC_PROC_ALL) {
		/* # Receive from a specified process */
		/* Invalid PID */
		if (src >= USER_PROCESS_COUNT) {
			return EINVPID;
		}
		/* Get src. process from src. PID */
		src_proc = &user_process[src];
		if (src_proc->status == PROC_UNINIT ||
		    src_proc->status == PROC_DEAD) {
			/* Src. process status is incorrect */
			return EINVPID;
		}
		if (src_proc == current_user_process) {
			/* Self-receiving */
			return EINVPID;
		}
	}


	/* Check receiver's waiting list */
	if (NULL == current_user_process->proc_next_receive) {
		/* BLOCK current process */
		current_user_process->status = PROC_RECEIVING;
		block(current_user_process);
		/* Force switch to next runnable process */
		schedule();

		return INOMSG;

	} else {
		/* Get next sender */
		if (src == IPC_PROC_ALL) {
			/* # Receive from all processes */

			/* Extract the process from the front of the waiting list */
			src_proc = current_user_process->proc_next_receive;
			current_user_process->proc_next_receive = src_proc->proc_next_receive;
			src_proc->proc_next_receive = NULL;
			kassert(PROC_SENDING == src_proc->status);

			/* Copy message to user space */
			COPY_MSG(ptr_msg, &(src_proc->msg_buf));
			/* Set sending process status */
			src_proc->proc_sending_to = NULL;
			src_proc->status = PROC_RUNNABLE;
			unblock(src_proc);

			return OK;

		} else {
			/* # Receive from a specified process */
			src_proc = current_user_process->proc_next_receive;
			if (src_proc->pid == src) {
				/* The front one is what we are searching */

				/* Extract the front process in the waiting list */
				current_user_process->proc_next_receive = src_proc->proc_next_receive;
				src_proc->proc_next_receive = NULL;
				kassert(PROC_SENDING == src_proc->status);

				/* Copy message to user space */
				COPY_MSG(ptr_msg, &(src_proc->msg_buf));
				/* Set sending process status */
				src_proc->proc_sending_to = NULL;
				src_proc->status = PROC_RUNNABLE;
				unblock(src_proc);

				return OK;

			} else {
				/* Need to search from the 2nd one */
				last_recv_proc = src_proc;
				cur_recv_proc = last_recv_proc->proc_next_receive;
				while (NULL != cur_recv_proc) {

					if (cur_recv_proc->pid == src) {
						/* # Node found */

						/* Extract the front process in the waiting list */
						last_recv_proc->proc_next_receive = cur_recv_proc->proc_next_receive;
						cur_recv_proc->proc_next_receive = NULL;
						kassert(PROC_SENDING == cur_recv_proc->status);

						/* Copy message to user space */
						COPY_MSG(ptr_msg, &(cur_recv_proc->msg_buf));
						/* Set sending process status */
						cur_recv_proc->proc_sending_to = NULL;
						cur_recv_proc->status = PROC_RUNNABLE;
						unblock(cur_recv_proc);

						return OK;
					}

					/* # Continue */
					kassert (cur_recv_proc != cur_recv_proc->proc_next_receive);
					last_recv_proc = cur_recv_proc;
					cur_recv_proc = cur_recv_proc->proc_next_receive;
				}
				/* # Node not found */

				/* BLOCK current process */
				current_user_process->status = PROC_RECEIVING;
				block(current_user_process);

				/* Force switch to next runnable process */
				schedule();
				return INOMSG;
			}
		}
	}
}
