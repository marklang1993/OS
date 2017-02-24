#include "interrupt.h"
#include "io_port.h"
#include "lib.h"
#include "pm.h"
#include "type.h"

// GDT in kernel
// NOTE: Size of gdt must be same as / larger than the count of gdt in loader.asm
struct descriptor gdt[5];			// There are totally 5 global descriptors (VARIED)
struct descriptor_ptr gdt_ptr;

// IDT in kernel
struct gate_descriptor idt[INTERRUPT_COUNT];	// There are totally 256 exceptions/interrupts (FIXED)
struct descriptor_ptr idt_ptr;


/* 
 # Initialize GDT in kernel
 */
static void kernel_init_gdt(void)
{
	/* Procedure: 
	 * kernel.asm : sgdt[gdt_ptr]
	 * kernel_init : copy gdt_ptr to gdt_ptr_old
	 *               set gdt_ptr by using gdt_ptr_old
	 *               memcpy
	 * kernel.asm : ldgt[gdt_ptr]
	 */
	struct descriptor_ptr gdt_ptr_old;
	uint32 count, size;

	// Init. gdt pointer in loader
	gdt_ptr_old.ptr_base = gdt_ptr.ptr_base;	
	gdt_ptr_old.limit = gdt_ptr.limit;
	// Init. gdt pointer in kernel
	gdt_ptr.limit = gdt_ptr_old.limit;
	gdt_ptr.ptr_base = (void*)gdt;
	
	// Calculate count, size
	count = gdt_ptr_old.limit + 1;
	count = count >> 3;
	size = count * DESCRIPTOR_SIZE;

	memcpy(gdt_ptr_old.ptr_base, gdt, size);
}

/* 
 # Initialize IDT entry in kernel
 @ int_no    : Interrupt number
 @ handler   : Interrupt handler function
 @ privilege : Indicate which kind of segment can invoke this interrupt by DPL
 */
static void kernel_init_idt_entry(uint32 int_no, int_handler handler, uint16 privilege)
{
	uint32 int_handler_offset = (uint32)handler;	
	
	idt[int_no].offset_1 = (uint16)(int_handler_offset & 0xffff);
	idt[int_no].selector = KERNEL_GDT_FLAT_C_SELECTOR;
	idt[int_no].count_param = 0x0;
	idt[int_no].attribute = P_T + S_SG + TYPE_G_386INT + privilege;
	idt[int_no].offset_2 = (uint16)(int_handler_offset >> 16);
}

 
/* 
 # Initialize IDT in kernel
 */
static void kernel_init_idt(void)
{
	uint32 i;

	// # Program 8259A
	// ICW1
	io_port_out(PORT_8259A_MAIN_0, DATA_8259A_ICW1);
	io_port_out(PORT_8259A_SLAVE_0, DATA_8259A_ICW1);
	// ICW2
	io_port_out(PORT_8259A_MAIN_1, DATA_8259A_MAIN_ICW2);
	io_port_out(PORT_8259A_SLAVE_1, DATA_8259A_SLAVE_ICW2);
	// ICW3
	io_port_out(PORT_8259A_MAIN_1, DATA_8259A_MAIN_ICW3);
	io_port_out(PORT_8259A_SLAVE_1, DATA_8259A_SLAVE_ICW3);
	// ICW4
	io_port_out(PORT_8259A_MAIN_1, DATA_8259A_ICW4);
	io_port_out(PORT_8259A_SLAVE_1, DATA_8259A_ICW4);
	// OCW1
	io_port_out(PORT_8259A_MAIN_1, DATA_8259A_MAIN_OCW1);
	io_port_out(PORT_8259A_SLAVE_1, DATA_8259A_SLAVE_OCW1);

	// # Interrupt Descriptor Table
	// Intel defined interrupts 0 ~ 19
	kernel_init_idt_entry(0, &int_entry_division_fault, DPL_0);
	kernel_init_idt_entry(1, &int_entry_debug_exception, DPL_0);
	kernel_init_idt_entry(2, &int_entry_non_maskable_interrupt, DPL_0);
	kernel_init_idt_entry(3, &int_entry_break_point, DPL_0);
	kernel_init_idt_entry(4, &int_entry_overflow, DPL_0);
	kernel_init_idt_entry(5, &int_entry_out_bound, DPL_0);
	kernel_init_idt_entry(6, &int_entry_undefined_opcode, DPL_0);
	kernel_init_idt_entry(7, &int_entry_no_math_coprocessor, DPL_0);
	kernel_init_idt_entry(8, &int_entry_double_fault, DPL_0);
	kernel_init_idt_entry(9, &int_entry_reserved_coprocessor_seg_overbound, DPL_0);
	kernel_init_idt_entry(10, &int_entry_invalid_tss, DPL_0);
	kernel_init_idt_entry(11, &int_entry_segment_not_exist, DPL_0);
	kernel_init_idt_entry(12, &int_entry_stack_segment_fault, DPL_0);
	kernel_init_idt_entry(13, &int_entry_general_protect_fault, DPL_0);
	kernel_init_idt_entry(14, &int_entry_page_fault, DPL_0);
	kernel_init_idt_entry(15, &int_entry_reserved_intel, DPL_0);
	kernel_init_idt_entry(16, &int_entry_x87fpu_fault, DPL_0);
	kernel_init_idt_entry(17, &int_entry_alignment_check_fault, DPL_0);
	kernel_init_idt_entry(18, &int_entry_machine_check_abort, DPL_0);
	kernel_init_idt_entry(19, &int_entry_simd_float_fault, DPL_0);

	// Other interrupts 20 ~ 255
	for(i = 20; i < INTERRUPT_COUNT; ++i)
	{
		if (i == 32)
		{
			kernel_init_idt_entry(i, &int_handler_clock, DPL_0);
		}
		else
		{
			kernel_init_idt_entry(i, &int_handler_default, DPL_0);
		}
	}

	// # Pointer to Interrupt Descriptor Table
	idt_ptr.limit = INTERRUPT_COUNT * GATE_DESCRIPTOR_SIZE - 1;
	idt_ptr.ptr_base = (void*)idt;
}


/* 
 # Initialize Kernel
 */
void kernel_init(void)
{
	char kernel_running_str[] = "Kernel is Running!";

	kernel_init_gdt();
	kernel_init_idt();

	print_set_location(3, 0);
	print_cstring(kernel_running_str);

	print_set_location(4, 0);
	print_uint32(12345678);
}

/*
 # Kernel Main
 */
void kernel_main(void)
{
	int i = 1 / 0;

stop_here:
	;
	goto stop_here;	
}
