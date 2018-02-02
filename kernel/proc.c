#include "dbg.h"
#include "interrupt.h"
#include "lib.h"
#include "proc.h"

/* Current Running User Process
 * NOTE :
 * 1. If scheduler is activated not under the 1st interrupt,
 * no user process will be scheduled.
 * 2. If scheduler is running, then system call cannot occur.
 * Since both scheduler and system call are executed in ring 0,
 * they are mutually exclusive.
 * 3. Even though clock interrupt barges before "INT INT_SYS_CALL",
 * "INT INT_SYS_CALL" continues to execute only if
 * "current_user_process" is the process that has been barged.
 * #. Thus, we can use "current_user_process" to get current pid
 * in system calls.
 */
struct process *current_user_process;

/* interrupt.c */
extern uint32 int_global_reenter;

/*
 # Check the stack boundary after context switching
 */
void stack_checker(void)
{
	/* Check the current esp with the esp boundary */
	uint32 current_esp = current_user_process->stack_frame.int_frame.esp;
	uint32 esp_boundary = (uint32)(current_user_process->stack);

	if (current_esp <= esp_boundary) {
		panic("STACK OVERFLOW AT PID: %d\n", current_user_process->pid);
	}
}

/*
 # Switch to next RUNNABLE process
 */
void schedule(void)
{
	uint32 base_pid, current_pid, next_pid;
	uint32 offset_idx;

	/* Get current pid as base_pid */
	base_pid = current_user_process->pid;

	/* Search for next RUNNABLE process */
	current_pid = base_pid;
	do {
		/* Go to next process */
		next_pid = (current_pid + 1) % USER_PROCESS_COUNT;

		/* Switch to next PCB */
		if(next_pid < current_pid) {
			/* wrap back */
			offset_idx = current_pid - next_pid;
			current_user_process -= offset_idx;

		} else {
			/* forward OR remained */
			offset_idx = next_pid - current_pid;
			current_user_process += offset_idx;
		}

		/* Check that "NO process can be run" */
		if (next_pid == base_pid &&
		    current_user_process->status != PROC_RUNNABLE) {
			panic("NO RUNNABLE PROCESS! MAY BE A DEADLOCK!\n");
		}

		/* Update current_pid */
		current_pid = next_pid;

	} while (current_user_process->status != PROC_RUNNABLE);

	/* Check the stack boundary for the current user process */
	stack_checker();

	/* Put the status of new process to RUNNING */
	current_user_process->status = PROC_RUNNING;
}


/*
 # Clock Interrupt Handler
 */
void process_scheduler(void)
{
	/* Check is able to switch process;
	 * If re-enter counter is 1, scheduler is the 1st interrupt.
	 */
	if (int_global_reenter != 1) {
		return;
	}

	/* Check remained cycles:
	 * (cycles == 0) OR (NOT RUNNING) => next process
	 */
	if (0 == current_user_process->cycles ||
	    current_user_process->status != PROC_RUNNING) {

		/* If current process's cycles is used up,
		 * refill the remained cycles of current process.
		 */
		if (0 == current_user_process->cycles) {
			current_user_process->cycles = USER_PROCESS_COUNT -
						current_user_process->priority;
		}
		/* If current process's status is RUNNING,
		 * switch to RUNNABLE.
		 */
		if (PROC_RUNNING == current_user_process->status) {
			current_user_process->status = PROC_RUNNABLE;
		}

		/* Switch to next RUNNABLE process */
		schedule();

	} else {
		/* Remained on current process */
		current_user_process->cycles -= 1;
	}
}

