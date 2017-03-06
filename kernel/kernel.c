#include "io_port.h"
#include "lib.h"
#include "proc.h"

// GDT in kernel
// NOTE: Size of gdt must be same as / larger than the count of gdt in loader.asm
struct descriptor gdt[GDT_COUNT];		// Global descriptors
struct descriptor_ptr gdt_ptr;

// IDT in kernel
struct gate_descriptor idt[INTERRUPT_COUNT];	// There are totally 256 exceptions/interrupts (FIXED)
struct descriptor_ptr idt_ptr;

// TSS in kernel
struct task_state_segment tss;

// Process in kernel
struct process user_process[USER_PROCESS_COUNT];


/* 
 # Initialize GDT / LDT entry in kernel
 @ dt         : descriptor table
 @ dte_no     : index of descriptor table entry
 @ base       : segment base address
 @ limit      : segment limit (20 bits)
 @ attributes : segment attribute: P, DPL, S, Type, G, D/B, AVL
 */
static void kernel_init_dt_entry
(
	struct descriptor *dt,
	uint32 dte_no,
	uint32 base,
	uint32 limit,
	uint32 attributes
)
{
	dt[dte_no].segment_limit_1 = (uint16)(limit & 0xffff);
	dt[dte_no].base_address_1 = (uint16)(base & 0xffff);
	dt[dte_no].base_address_2 = (uint8)((base >> 16) & 0xff);
	dt[dte_no].attribute_1 = (uint8)(attributes & 0xff);
	dt[dte_no].attribute_2 = (uint8)(((limit >> 16) & 0xf) | ((attributes >> 4) & 0xf0));
	dt[dte_no].base_address_3 = (uint8)((base >> 24) & 0xff);
}


/* 
 # Initialize GDT in kernel
 */
static void kernel_init_gdt(void)
{
	/* Procedure: 
	 * kernel.asm : sgdt[gdt_ptr]
	 * kernel_init : copy gdt_ptr to gdt_ptr_old
	 * 		 init. new gdt entries in kernel (tss, ldt)
	 *               set gdt_ptr by using gdt_ptr_old
	 *               memcpy
	 * kernel.asm : ldgt[gdt_ptr]
	 */
	struct descriptor_ptr gdt_ptr_old;
	uint32 size;
	uint32 i;

	// Init. gdt pointer in loader (old)
	gdt_ptr_old.ptr_base = gdt_ptr.ptr_base;	
	gdt_ptr_old.limit = gdt_ptr.limit;
	// Calculate gdt count, size (old)	
	size = gdt_ptr_old.limit + 1;

	// Copy old gdt to new gdt
	memcpy(gdt, gdt_ptr_old.ptr_base, size);

	// Init. gdt entry of tss in kernel
	kernel_init_dt_entry(
			gdt,
			SEL_TO_IDX(KERNEL_GDT_FLAT_TSS_SELECTOR), 
			(uint32)&tss, 
			TASK_STATE_SEGMENT_SIZE,
			G_BYTE | P_T | S_SG | TYPE_S_A386TSS
			);

	// Init. gdt entries of ldt in each user process
	for(i = 0; i < USER_PROCESS_COUNT; ++i)
	{
		kernel_init_dt_entry(
			gdt,
			SEL_TO_IDX(KERNEL_GDT_FLAT_LDT_0_SELECTOR) + i,
			(uint32)&(user_process[i].ldt),
			DESCRIPTOR_SIZE * LDT_COUNT,
			G_BYTE | P_T | S_SG | TYPE_S_LDT
			);
	}

	// Calculate gdt size (new)
	size = GDT_COUNT * DESCRIPTOR_SIZE;
	// Init. gdt pointer in kernel (new)
	gdt_ptr.limit = size - 1;
	gdt_ptr.ptr_base = (void*)gdt;
}


/* 
 # Initialize TSS in kernel
 */
static void kernel_init_tss(void)
{
	memset(&tss, 0, TASK_STATE_SEGMENT_SIZE);
	tss.ss_0 = KERNEL_GDT_FLAT_STACK_SELECTOR;
}


/* 
 # Initialize IDT entry in kernel
 @ int_no    : Interrupt number
 @ handler   : Interrupt handler function
 @ privilege : Indicate which kind of segment can invoke this interrupt by DPL
 */
static void kernel_init_idt_entry
(
	uint32 int_no, 
	int_handler handler,
	uint16 privilege
)
{
	uint32 int_handler_offset = (uint32)handler;	
	
	idt[int_no].offset_1 = (uint16)(int_handler_offset & 0xffff);
	idt[int_no].selector = KERNEL_GDT_FLAT_CODE_SELECTOR;
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
	// Init. all kernel data structures
	kernel_init_gdt();
	kernel_init_tss();
	kernel_init_idt();

	// Print msg.
	print_set_location(3, 0);
	print_cstring("Kernel is Running!");

/*
	print_set_location(4, 0);
	memset(kernel_running_str, '0', 18);
	print_cstring(kernel_running_str);

	print_set_location(4, 0);
	print_uint32((uint32)&gdt);
	print_set_location(4, 9);
	print_uint32((uint32)&idt);
*/

}

void user_main(void);

/*
 # Kernel Main
 */
void kernel_main(void)
{
	// Init. ldt in user process -- 0
	memset(user_process, 0, sizeof(struct process));
	kernel_init_dt_entry(
			user_process[0].ldt,
                        SEL_TO_IDX(KERNEL_LDT_CODE_SELECTOR),
                        0,
                        0xfffff,
                        TYPE_C_E + TYPE_C_R + S_DC + P_T + D_EC_32 + G_4KB + DPL_3
                        );
	kernel_init_dt_entry(
			user_process[0].ldt,
                        SEL_TO_IDX(KERNEL_LDT_DATA_SELECTOR),
                        0,
                        0xfffff,
                        TYPE_D_W + TYPE_D_R + S_DC + P_T + D_EC_32 + G_4KB + DPL_3
                        );

	// Init. stack frame
	user_process[0].stack_frame.gs = KERNEL_GDT_VIDEO_SELECTOR;
	user_process[0].stack_frame.fs = KERNEL_LDT_DATA_SELECTOR;
	user_process[0].stack_frame.es = KERNEL_LDT_DATA_SELECTOR;
	user_process[0].stack_frame.ds = KERNEL_LDT_DATA_SELECTOR;
	user_process[0].stack_frame.int_frame.eip = (uint32)user_main;
	user_process[0].stack_frame.int_frame.cs = KERNEL_LDT_CODE_SELECTOR;	
	user_process[0].stack_frame.int_frame.eflags = 0x3002;	// IOPL = 3	
	user_process[0].stack_frame.int_frame.esp = (uint32)(user_process[0].stack + USER_PROCESS_COUNT);	
	user_process[0].stack_frame.int_frame.ss = KERNEL_LDT_DATA_SELECTOR;
	
	// Print msg.
	print_set_location(4, 0);
	print_cstring("Start User Process!");
}


/*
 # Test User
 */
void user_main(void)
{
	uint32 i = 0;

	while(1)
	{
		print_set_location(17, 0);
		print_cstring("User Process is Running: ");
		print_set_location(17, 26);
		print_uint32(i);
		++i;
	}
}
