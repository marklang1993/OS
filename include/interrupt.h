/* 
 * Interrupt related functions declaration
 * NOTE: Implemetation are in asm file
 */

#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_

#include "type.h"
#include "drivers/i8259a.h"

/*
 * Interrupts / Exceptions Vector Table
 * 0 ~ 19		CPU exceptions
 * 20 ~ 31		Reserved by Intel
 * 32 ~ 255		Maskable interrupts
 */
#define INTERRUPT_COUNT			256
#define INTERRUPT_8259A_OFFSET		32

/* Interrupt related struct */
struct int_plc_stack_frame
/* Interrupt (No Error Code, Priviledge Level Change) Stack Frame */
{
	uint32 eip;
	uint32 cs;
	uint32 eflags;
	uint32 esp;
	uint32 ss;
};

struct int_stack_frame
/* Interrupt (No Error Code, No Priviledge Level Change) Stack Frame */
{
	uint32 eip;
	uint32 cs;
	uint32 eflags;
};

/* Interrupt related funtions */
void sti(void);
void cli(void);

/* Interrupt handlers */
typedef ptr_void_function int_handler; /* Function pointer of interrupt handler*/

void int_entry_division_fault(void);
void int_entry_debug_exception(void);
void int_entry_non_maskable_interrupt(void);
void int_entry_break_point(void);
void int_entry_overflow(void);
void int_entry_out_bound(void);
void int_entry_undefined_opcode(void);
void int_entry_no_math_coprocessor(void);
void int_entry_double_fault(void);
void int_entry_reserved_coprocessor_seg_overbound(void);
void int_entry_invalid_tss(void);
void int_entry_segment_not_exist(void);
void int_entry_stack_segment_fault(void);
void int_entry_general_protect_fault(void);
void int_entry_page_fault(void);
void int_entry_reserved_intel(void);
void int_entry_x87fpu_fault(void);
void int_entry_alignment_check_fault(void);
void int_entry_machine_check_abort(void);
void int_entry_simd_float_fault(void);

void int_handler_default(void);
void int_handler_clock(void);
void int_handler_keyboard(void);
void int_handler_hdd(void);
void int_handler_syscall(void);

#endif
