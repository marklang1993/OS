#ifndef _KHEAP_H_
#define _KHEAP_H_

#include "errors.h"
#include "type.h"

/* Size of each block in kernel heap */
#define SIZE_KHEAP_BLOCK		0x4


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
void *krealloc(void *kheap_mem, uint32 count);

void kchk_mem(void *kheap_mem, uint32 count);

#endif
