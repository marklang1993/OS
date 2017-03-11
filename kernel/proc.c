#include "lib.h"
#include "proc.h"

struct process *current_user_process;
static uint32 scheduler_count = 0;

static void scheduler_main()
{
	uint32 current_pid, next_pid;
	uint32 offset_idx;

	current_pid = current_user_process -> pid;
	next_pid = (current_pid + 1) % USER_PROCESS_COUNT;
	
	if(next_pid < current_pid)
	{
		// wrap back
		offset_idx = current_pid - next_pid;
		current_user_process -= offset_idx;
	}
	else
	{
		// forward
		offset_idx = next_pid - current_pid;
		current_user_process += offset_idx;
	}
}

void process_scheduler(void)
{
	char msg[] = "Scheduler is Running! Current PCB Address: ";
	char count_str[] = "0000000000";
	uint32 pos = 0;

	pos = strlen(msg);
	print_cstring_pos(msg, 16, 0);

//	itoa(scheduler_count, count_str);
	itoa((uint32)current_user_process, count_str);
	print_cstring_pos(count_str, 16, pos);

	++scheduler_count;

	scheduler_main();
}
