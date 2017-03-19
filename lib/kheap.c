#include "kheap.h"
#include "memory.h"

/* Kernel Heap Memory Area */
#define ADDR_KHEAP_BASE			0x200000	/* KHeap start from 0x200000 */
#define ADDR_KHEAP_LIMIT		0x100000	/* KHeap size = 0x100000 (1MB) */

/* Kernel Heap Root Node */
static struct kheap_node *kheap_base = NULL;


/*
 # Init. Kheap by creating the 1st kheap node
 */
void init_kheap(void)
{
	kheap_base = (struct kheap_node *)ADDR_KHEAP_BASE;
	/* Init. parameters */
	kheap_base->block_count = ADDR_KHEAP_LIMIT / SIZE_KHEAP_BLOCK - CNT_KHEAP_NODE_BLOCK;
	kheap_base->is_free = TRUE;
	kheap_base->next = NULL;
}


/*
 # Allocate kernel heap memory
 @ count  : size of heap memory required in bytes
 * RETURN : pointer to the memory
 */
void *kmalloc(uint32 count)
{
	struct kheap_node *current;
	struct kheap_node *next;
	struct kheap_node *new_next;
	void *mem = NULL;

	uint32 block_count;		// required blocks
	uint32 insufficient_bytes;	// used to determine whether 1 more block is needed
	uint32 available_block_count;	// count of blocks can get after splitting

	/* Check base node */
	if (NULL == kheap_base) {
		return mem;
	}

	/* Check count */
	if (0 == count) {
		return EINVARG;
	}

	/* Search */
	current = kheap_base;
	while(NULL != current) {
		next = current->next;

		if (IS_TRUE(current->is_free)) {
			/* current block is free */
			/* calculate required blocks */
			block_count = count / SIZE_KHEAP_BLOCK;
			insufficient_bytes = count % SIZE_KHEAP_BLOCK;
			if (0 != insufficient_bytes) {
				/* Need 1 more block */
				block_count += 1;
			}
			/* check do we need to split the current block */
			if (block_count == current->block_count) {
				/* Best fit */
				current->is_free = FALSE;
				mem = (void *)(((uint32)current) + CNT_KHEAP_NODE_BYTE);
				return mem;

			} else if (block_count > current->block_count) {
				/* This node has insufficient blocks */
				current = next;
				continue;

			} else {
				/* Fit, but need to split to save memory */
				available_block_count = current->block_count - block_count;
				/* Check is there enough memory to put 1 kheap_node and 1 block */
				if (available_block_count > CNT_KHEAP_NODE_BLOCK) {
					/* Can be splitted */
					mem = (void *)(((uint32)current) + CNT_KHEAP_NODE_BYTE);
					current->block_count = block_count;
					current->is_free = FALSE;
					
					/* Create splitted node */
					new_next = (struct kheap_node *)(((uint32)mem) + block_count * SIZE_KHEAP_BLOCK);
					new_next->block_count = available_block_count - CNT_KHEAP_NODE_BLOCK;
					new_next->is_free = TRUE;
					new_next->next = NULL;
					current->next = new_next;

					return mem;
				} else {
					/* Cannot be splitted -> return the current block*/
					current->is_free = FALSE;
					mem = (void *)(((uint32)current) + CNT_KHEAP_NODE_BYTE);
					return mem;
				}
			}
	
		} else {
			/* current block is not free */
			current = next;
		}
	}

	/* No enough memory in kernel heap*/
	return mem;
}


/*
 # Deallocated kernel heap memory
 @ kheap_mem : allocated memory pointer
 * RETURN    : for debug purpose, 0 on success
 */
rtc kfree(void *kheap_mem)
{
	struct kheap_node *current;
	struct kheap_node *last, *next;
	void *mem;

	/* Check base node */
	if (NULL == kheap_base) {
		return ENOTINIT;
	}

	/* Check kheap_mem */
	if (NULL == kheap_mem) {
		return EINVARG;
	}

	/* Search */
	current = kheap_base;
	last = NULL;
	while (current != NULL) {
		next = current->next;

		/* Find the corresponding kheap node */
		mem = (void *)(((uint32)current) + CNT_KHEAP_NODE_BYTE);
		if (mem == kheap_mem) {
			/* Node found*/
			if (IS_TRUE(current->is_free)) {
				/* That node is free alreadly - Do nothing */
				return EHEAPMEM;

			} else {
				/* Free that node */
				current->is_free = TRUE;
				/* Check merge */
				if (NULL != next) {
					/* next node is presented */
					if (IS_TRUE(next->is_free)) {
						/* Merge with next node */
						current->next = next->next;
						current->block_count += next->block_count + CNT_KHEAP_NODE_BLOCK;
					}
				}

				if (NULL != last) {
					/* last node is presented */
					if (IS_TRUE(last->is_free)) {
						/* Merge with last node */
						last->next = current->next;
						last->block_count += current->block_count + CNT_KHEAP_NODE_BLOCK;
					}
				}

				/* Otherwise, cannot merge */
				return OK;
			}

		} else {
			/* Node not found */
			last = current;
			current = next;
			continue;
		}
	}

	return EINVPTR;
}


void krealloc(void *kheap_mem, uint32 count)
{
}


void kchk_mem(void *kheap_mem, uint32 count)
{
}

