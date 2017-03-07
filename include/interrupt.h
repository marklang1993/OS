// Interrupt related functions declaration
// NOTE: Implemetation are in asm file

#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_

#include "type.h"

/* 
 * Interrupts / Exceptions Vector Table
 * 0 ~ 19		CPU exceptions
 * 20 ~ 31		Reserved by Intel
 * 32 ~ 255		Maskable interrupts
 */
#define INTERRUPT_COUNT			256

// 8259A Ports 
#define PORT_8259A_MAIN_0		0x20
#define PORT_8259A_MAIN_1		0x21
#define PORT_8259A_SLAVE_0		0xa0
#define PORT_8259A_SLAVE_1		0xa1


// ICW1 -> PORT0, ICW2~4 -> PORT1
#define DATA_8259A_ICW1			0x11
#define DATA_8259A_MAIN_ICW2		0x20	// IR0 <-> 32	
#define DATA_8259A_SLAVE_ICW2		0x28	// IR8 <-> 40
#define DATA_8259A_MAIN_ICW3		0x04	// Main IR2 (3rd bit) -> Slave 8259A
#define DATA_8259A_SLAVE_ICW3		0x02	// Slave 8259A -> Main IR2
#define DATA_8259A_ICW4			0x01	// x86 mode, auto EOI

// OCW1 -> PORT1, OCW2(EOI) -> PORT0
#define DATA_8259A_MAIN_OCW1		0xfe	// Enable only clock interrupt
#define DATA_8259A_SLAVE_OCW1 		0xff	// Disable all interrupts
#define DATA_8259A_OCW2			0x20	// Send EOI to notify 8259A interrupt processed

// Interrupt related struct
struct int_plc_stack_frame	// Interrupt (No Error Code, Priviledge Level Change) Stack Frame
{
	uint32 eip;
	uint32 cs;
	uint32 eflags;
	uint32 esp;
	uint32 ss;
};

struct int_stack_frame		// Interrupt (No Error Code, No Priviledge Level Change) Stack Frame
{
	uint32 eip;
	uint32 cs;
	uint32 eflags;
};

// Interrupt related funtions
void sti(void);
void cli(void);

// Interrupt handlers
typedef ptr_void_function int_handler;		// Function pointer of interrupt handler

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

#endif
