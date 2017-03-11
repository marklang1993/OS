#include "buffer.h"
#include "interrupt.h"

/*
 # Initialize a Circular Buffer
 @ ptr_cbuf : Pointer to a circular buffer 
 */
rtc cbuf_init(struct cbuf *ptr_cbuf)
{
	// Check pointer of cbuf
	if(NULL == ptr_cbuf)
	{
		return EINVARG;
	}

	cli();

	ptr_cbuf->count = 0;
	ptr_cbuf->head = 0;
	ptr_cbuf->tail = 0;
	memset(ptr_cbuf, 0, SIZE_CIRCULAR_BUFFER);		

	sti();

	return OK;
}

/*
 # Read an Element from a Circular Buffer
 @ ptr_cbuf : Pointer to a circular buffer 
 @ ptr_val  : Pointer to an output value variable
 */
rtc cbuf_read(struct cbuf *ptr_cbuf, uint8 *ptr_val)
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
	ptr_cbuf->tail = (ptr_cbuf->tail + 1) % SIZE_CIRCULAR_BUFFER;

	sti();

	return OK;
}

/*
 # Write an Element to a Circular Buffer
 @ ptr_cbuf : Pointer to a circular buffer 
 @ val      : Input value
 */
rtc cbuf_write(struct cbuf *ptr_cbuf, uint8 val)
{
	// Check pointer of cbuf
	if(NULL == ptr_cbuf)
	{
		return EINVARG;
	}

	cli();

	if (ptr_cbuf->count == SIZE_CIRCULAR_BUFFER)
	{
		// Buffer is full
		sti();
		return EBUFFUL;
	}

	ptr_cbuf->count += 1;
	ptr_cbuf->data[ptr_cbuf->head] = val;
	ptr_cbuf->head = (ptr_cbuf->head + 1) % SIZE_CIRCULAR_BUFFER;

	sti();

	return OK;
}


