#ifndef _KHEAP_H_
#define _KHEAP_H_

#include "errors.h"
#include "type.h"

/* KHeap Memory Area */
#define ADDR_KHEAP_BASE			0x200000	/* KHeap start from 0x200000 */
#define ADDR_KHEAP_LIMIT		0x100000	/* KHeap size = 0x100000 (1MB) */

#define SIZE_KHEAP_BLOCK		0x4		/* Each block has 4-byte size */


/* This struct must be the integral multiples of SIZE_KHEAP_BLOCK */
struct kheap_node
{
	uint32 block_count;		/* count of blocks in this node */
	BOOL is_free;			/* is the memory represented by this node free to use */
	struct kheap_node *next;	/* pointer to the next node */
};
#define CNT_KHEAP_NODE_BYTE	sizeof(struct kheap_node)
#define CNT_KHEAP_NODE_BLOCK	(CNT_KHEAP_NODE_BYTE / SIZE_KHEAP_BLOCK)


/* KHeap functions */
void init_kheap(void);
void *kmalloc(uint32 count);
rtc kfree(void *kheap_mem);
void krealloc(void *kheap_mem, uint32 count);

void kchk_mem(void *kheap_mem, uint32 count);

#endif
