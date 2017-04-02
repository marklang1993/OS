#include "dbg.h"
#include "interrupt.h"
#include "lib.h"
#include "proc.h"

struct process *current_user_process;

/* interrupt.c */
extern uint32 int_global_reenter;

/*
 # Switch to next process
 */
static void switch_process(void)
{
	uint32 base_pid, current_pid, next_pid;
	uint32 offset_idx;

	/* Get current pid as base_pid */
	base_pid = current_user_process->pid;

	/* If current process's cycles is used up,
	 * refill the remained cycles of current process.
	 */
	if (0 == current_user_process->cycles) {
		current_user_process->cycles = USER_PROCESS_COUNT - current_user_process->priority;
	}

	/* If current process's status is RUNNING,
	 * switch to RUNNABLE.
	 */
	if (PROC_RUNNING == current_user_process->status) {
		current_user_process->status = PROC_RUNNABLE;
	}

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
		switch_process();

	} else {
		/* Remained on current process */
		current_user_process->cycles -= 1;
	}
}

