#include "interrupt.h"
#include "lib.h"
#include "type.h"


static const char *interrupt_msg[] = {
	"division_fault",
	"debug_exception",
	"non_maskable_interrupt",
	"break_point",
	"overflow",
	"out_bound",
	"undefined_opcode",
	"no_math_coprocessor",
	"double_fault",
	"reserved_coprocessor_seg_overbound",
	"invalid_tss",
	"segment_not_exist",
	"stack_segment_fault",
	"general_protect_fault",
	"page_fault",
	"reserved_intel",
	"x87fpu_fault",
	"alignment_check_fault",
	"machine_check_abort",
	"simd_float_fault"
}; 

void interrupt_handler(uint32 vector_no, uint32 error_code, uint32 eip, uint32 cs, uint32 eflags)
{
	// Output message
	print_set_location(0, 0);
	print_cstring("PANIC: #");
	print_uint32(vector_no);
	print_cstring(" ");
	print_cstring(interrupt_msg[vector_no]);

	print_set_location(1, 0);
	print_cstring("Error code: ");
	print_uint32(error_code);
	print_cstring("; EIP: ");
	print_uint32(eip);
	print_cstring("; CS: ");
	print_uint32(cs);
}

void interrupt_reenter_times_init(void)
{
	uint32 i;
	for (i = 0; i < INTERRUPT_COUNT; ++i)
	{
		int_reenter_times[i] = 1;
	}
}

