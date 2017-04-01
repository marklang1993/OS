#include "lib.h"
#include "syscall.h"

/* SYS_CALL table defined here */
sys_call_fp sys_call_table[] = {
	sys_call_test
};

/*
 # Dispatch system call request
 @ base_arg : base address of all arguments
 */
void sys_call_dispatch(void *base_arg)
{
	uint32 function_num = *((uint32 *)base_arg);
	rtc *ptr_ret = *(rtc **)(((uint32 *)base_arg) + 1);

	/* Call corresponding system call */
	*ptr_ret = sys_call_table[function_num](
			(void *)(((uint32 *)base_arg) + 2)
			);
}


rtc sys_call_test(void *base_arg)
{
	uint32 *arg_ptr = base_arg;

	printk("test message: %u %u", *arg_ptr, *(arg_ptr + 1));

	return -10;
}
