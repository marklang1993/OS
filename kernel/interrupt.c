#include "dbg.h"
#include "interrupt.h"
#include "lib.h"
#include "type.h"

/* Interrupt re-enter flag -- For all interrupt
 * 0: No interrupt occurs, need to switch stack once interrupter occurs
 * Others : Interrupt occurs, do not need to switch stack once interrupter occurs
 */
uint32 int_global_reenter = 0;

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
	panic("#%u %s\nError code: %d; EIP: 0x%x; CS: 0x%x; EFLAGS: 0x%x\n",
		vector_no, interrupt_msg[vector_no], error_code, eip, cs, eflags);
}


