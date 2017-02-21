#include "pm.h"
#include "lib.h"
#include "type.h"

// GDT in kernel
// NOTE: Size of gdt must be same as / larger than the count of gdt in loader.asm
struct descriptor gdt[5];
struct descriptor_ptr gdt_ptr;

/* 
 # kernel_init:
 */
void kernel_init(void)
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
	gdt_ptr.ptr_base = gdt;
	
	// Calculate count, size
	count = gdt_ptr_old.limit + 1;
	count = count >> 3;
	size = count * DESCRIPTOR_SIZE;

	memcpy((void*)gdt_ptr_old.ptr_base, (void*)gdt, size);
}
