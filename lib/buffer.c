#include "buffer.h"
#include "interrupt.h"
#include "kheap.h"

/*
 # Initialize a Circular Buffer
 @ ptr_cbuf	: pointer to a circular buffer
 @ capacity	: count of elements in buffer
 @ ptr_data	: pointer to data which are used to initialize buffer
 @ size		: size of initial data
 */
rtc cbuf_init(
	struct cbuf *ptr_cbuf,
	uint32 capacity,
	uint32 *ptr_data,
	uint32 size
)
{
	int i;

	/* Check pointer of cbuf & capacity */
	if (NULL == ptr_cbuf || 0 == capacity) {
		return EINVARG;
	}

	/* Initialze the parameters of the buffer */
	ptr_cbuf->capacity = capacity;
	ptr_cbuf->count = 0;
	ptr_cbuf->head = 0;
	ptr_cbuf->tail = 0;
	ptr_cbuf->data = (uint32 *)kmalloc(capacity * sizeof(uint32));	/* Convert to byte count */

	/* Check memory allocation result */
	if (NULL == ptr_cbuf->data) {
		return EOUTMEM;
	}

	/* Write initial data */
	if (ptr_data != NULL && size != 0) {

		/* Check : initial data size <= capacity of buffer */
		if (capacity < size) {
			/* If initial data is too long, then truncate the excess part */
			size = capacity;
		}

		for (i = 0; i < size; ++i) {
			ptr_cbuf->count += 1;
			ptr_cbuf->data[ptr_cbuf->head] = ptr_data[i];
			ptr_cbuf->head = (ptr_cbuf->head + 1) % ptr_cbuf->capacity;
		}
	}

	return OK;
}

/*
 # Read an Element from a Circular Buffer
 @ ptr_cbuf : Pointer to a circular buffer 
 @ ptr_val  : Pointer to an output value variable
 */
rtc cbuf_read(struct cbuf *ptr_cbuf, uint32 *ptr_val)
{
	// Check pointer of cbuf & val
	if(NULL == ptr_cbuf || NULL == ptr_val)
	{
		return EINVARG;
	}

	cli();

	if (0 == ptr_cbuf->count)
	{
		// No data in the buffer
		sti();
		return EBUFEMP;
	}

	ptr_cbuf->count -= 1;
	*ptr_val = ptr_cbuf->data[ptr_cbuf->tail];
	ptr_cbuf->tail = (ptr_cbuf->tail + 1) % ptr_cbuf->capacity;

	sti();

	return OK;
}

/*
 # Write an Element to a Circular Buffer
 @ ptr_cbuf : Pointer to a circular buffer 
 @ val      : Input value
 */
rtc cbuf_write(struct cbuf *ptr_cbuf, uint32 val)
{
	// Check pointer of cbuf
	if(NULL == ptr_cbuf)
	{
		return EINVARG;
	}

	cli();

	if (ptr_cbuf->count == ptr_cbuf->capacity)
	{
		// Buffer is full
		sti();
		return EBUFFUL;
	}

	ptr_cbuf->count += 1;
	ptr_cbuf->data[ptr_cbuf->head] = val;
	ptr_cbuf->head = (ptr_cbuf->head + 1) % ptr_cbuf->capacity;

	sti();

	return OK;
}


