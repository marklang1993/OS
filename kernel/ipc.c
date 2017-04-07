#include "dbg.h"
#include "buffer.h"
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
 # IPC - wait for interrupt (Ring 1/3)
 */
rtc wait_int()
{
	return recv_msg(IPC_PROC_INT, NULL);
}

/*
 # IPC - restart after interrupt (Ring 0)
 @ pid : pid of the process which will be resumed
 */
void resume_int(uint32 pid)
{
	struct process *dst_proc;

	/* Check PID */
	if (pid < USER_PROCESS_COUNT) {
		panic("INVALID PID IN RESUME INTERRUPT!");
	}
	dst_proc = &(user_process[pid]);

	/* Check Process Status*/
	if (dst_proc->status != PROC_WAIT_INT) {
		panic("INVALID STATUS IN RESUME INTERRUPT!");
	}

	/* UNBLOCK dst. process */
	dst_proc->status = PROC_RUNNABLE;
	unblock(dst_proc);
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

	/* Get arguments */
	uint32 dst = *((uint32 *)base_arg);
	struct proc_msg *ptr_msg = *((struct proc_msg **)(((uint32 *)base_arg) + 1));
	COPY_ARG();

	/* Check PID & Process status & Self-sending */
	if (dst >= USER_PROCESS_COUNT) {
		panic("INVALID DST. PID IN IPC!");
		return EINVPID;
	}
	dst_proc = &user_process[dst];
	if (dst_proc->status == PROC_UNINIT ||
	    dst_proc->status == PROC_DEAD) {
		/* Dst. process status is incorrect */
		panic("INVALID DST. PID IN IPC!");
		return EINVPID;
	}
	if (dst_proc == current_user_process) {
		/* Self-sending */
		panic("SELF-SENDING IN IPC!");
		return EINVPID;
	}

	/* Check Deadlock */
	ret = is_deadlock(dst);
	if (ret != OK) {
		panic("DEADLOCK DETECTED IN IPC!\nSRC.: %u\nDST.: %u", current_user_process->pid, dst);
		return ret;
	}

	/* Record dst. process information */
	kassert(NULL == current_user_process->proc_sending_to);
	current_user_process->proc_sending_to = dst_proc;
	/* Copy the message to the src. process msg. buffer */
	COPY_MSG(&(current_user_process->msg_buf), ptr_msg);
	/* Set source PID */
	current_user_process->msg_buf.msg_src = current_user_process->pid;
	/* Append the dst. process address to the receive queue */
	ret = cbuf_write(&(dst_proc->recv_queue), ((uint32)current_user_process));
	kassert(OK == ret);

	/* Check dst. process is RECEIVING */
	if (dst_proc->status == PROC_RECEIVING) {
		/* UNBLOCK dst. process */
		dst_proc->status = PROC_RUNNABLE;
		unblock(dst_proc);
	}

	/* BLOCK current process status */
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
	struct process *src_proc;	/* Source process */
	struct process *ext_proc;	/* Extracted process */
	struct cbuf t_cbuf;		/* Temp cbuf, used to search for msg. with specified pid */

	/* Get arguments */
	uint32 src = *((uint32 *)base_arg);
	struct proc_msg *ptr_msg = *((struct proc_msg **)(((uint32 *)base_arg) + 1));
	COPY_ARG();

	/* Check: is source an interrupt */
	if (src == IPC_PROC_INT) {
		/* BLOCK current process */
		current_user_process->status = PROC_WAIT_INT;
		block(current_user_process);
		/* Force switch to next runnable process */
		schedule();

		/* Interrupt does not contain any message,
		 * and it is used to block, so return OK.
		 */
		return OK;
	}

	/* Check PID & Process status & Self-receiving & Get src_proc */
	if (src != IPC_PROC_ALL) {
		/* # Receive from a specified process */
		/* Invalid PID */
		if (src >= USER_PROCESS_COUNT) {
			panic("INVALID SRC. PID IN IPC!");
			return EINVPID;
		}
		/* Get src. process from src. PID */
		src_proc = &user_process[src];
		if (src_proc->status == PROC_UNINIT ||
		    src_proc->status == PROC_DEAD) {
			/* Src. process status is incorrect */
			panic("PROCESS STATUS IS INCORRECT IN IPC!");
			return EINVPID;
		}
		if (src_proc == current_user_process) {
			/* Self-receiving */
			panic("SELF-RECEIVING IN IPC!");
			return EINVPID;
		}
	}


	/* Check receiver's waiting list */
	if (IS_TRUE(cbuf_is_empty(&(current_user_process->recv_queue)))) {
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
			ret = cbuf_read(&(current_user_process->recv_queue), ((uint32 *)&src_proc));
			kassert(OK == ret);
			/* Copy message to user space */
			COPY_MSG(ptr_msg, &(src_proc->msg_buf));
			/* Set sending process status */
			kassert(src_proc->proc_sending_to != NULL);
			src_proc->proc_sending_to = NULL;
			kassert(PROC_SENDING == src_proc->status);
			src_proc->status = PROC_RUNNABLE;
			unblock(src_proc);

			return OK;

		} else {
			/* # Receive from a specified process */
			ret = cbuf_read(&(current_user_process->recv_queue), ((uint32 *)&src_proc));
			kassert(OK == ret);
			if (src_proc->pid == src) {
				/* The front one is what we are searching */

				/* Copy message to user space */
				COPY_MSG(ptr_msg, &(src_proc->msg_buf));
				/* Set sending process status */
				kassert(src_proc->proc_sending_to != NULL);
				src_proc->proc_sending_to = NULL;
				kassert(PROC_SENDING == src_proc->status);
				src_proc->status = PROC_RUNNABLE;
				unblock(src_proc);

				return OK;

			} else {
				/* Need to search from the 2nd one */

				/* Init. temp cbuf */
				ret = cbuf_init(&t_cbuf, USER_PROCESS_COUNT, NULL, 0);
				kassert(OK == ret);

				ext_proc = NULL;
				ret = cbuf_write(&t_cbuf, (uint32)src_proc);
				/* Search for the sending process with specified pid */
				while(NOT(IS_TRUE(cbuf_is_empty(&(current_user_process->recv_queue))))) {
					ret = cbuf_read(&(current_user_process->recv_queue), ((uint32 *)&src_proc));
					kassert(OK == ret);
					if (src_proc->pid == src) {
						/* PID matched */
						kassert(NULL == ext_proc); /* There should be no sending process with same pid */
						ext_proc = src_proc;
						continue;
					}
					ret = cbuf_write(&t_cbuf, (uint32)src_proc);
					kassert(OK == ret);
				}
				/* Store back all other processes */
				while(NOT(IS_TRUE(cbuf_is_empty(&t_cbuf)))) {
					ret = cbuf_read(&t_cbuf, ((uint32 *)&src_proc));
					kassert(OK == ret);
					ret = cbuf_write(&(current_user_process->recv_queue), (uint32 )src_proc);
					kassert(OK == ret);
				}
				/* Uninit. the temp cbuf */
				cbuf_uninit(&t_cbuf);

				/* Check "ext_proc" */
				if (ext_proc != NULL) {
					/* # Process found */

					/* Copy message to user space */
					COPY_MSG(ptr_msg, &(ext_proc->msg_buf));
					/* Set sending process status */
					kassert(src_proc->proc_sending_to != NULL);
					src_proc->proc_sending_to = NULL;
					kassert(PROC_SENDING == ext_proc->status);
					ext_proc->status = PROC_RUNNABLE;
					unblock(ext_proc);

					return OK;

				} else {
					/* # Process not found */

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
}
