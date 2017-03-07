#include "lib.h"
#include "proc.h"

uint32 process_scheduler_running = 0;	// Flag for checking the scheduler is running or not
static uint32 scheduler_count = 0;

static void scheduler_main(struct process *user_process[])
{
/*
	uint32 i, j, k;
	uint32 m;

	for(i = 0; i < 10; ++i)
	{
		for(j = 0; j < 10; ++j)
		{
			for(k = 0; k < 1000; ++k)
			{
				++m;
			}
		}
	}
*/

	uint32 current_pid, next_pid;
	uint32 offset_idx;

	current_pid = (*user_process) -> pid;
	next_pid = (current_pid + 1) % USER_PROCESS_COUNT;
	
	if(next_pid < current_pid)
	{
		// wrap back
		offset_idx = current_pid - next_pid;
		*user_process -= offset_idx;
	}
	else
	{
		// forward
		offset_idx = next_pid - current_pid;
		*user_process += offset_idx;
	}
}

void process_scheduler(struct process user_process[])
{
	char msg[] = "Scheduler is Running! Current PCB Address: ";
	char count_str[] = "0000000000";
	uint32 pos = 0;

	pos = strlen(msg);
	print_cstring_pos(msg, 16, 0);

//	itoa(scheduler_count, count_str);
	itoa((uint32)user_process, count_str);
	print_cstring_pos(count_str, 16, pos);

	++scheduler_count;

	scheduler_main(&user_process);
}
