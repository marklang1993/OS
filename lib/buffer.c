#include "buffer.h"
#include "interrupt.h"
#include "kheap.h"
#include "lib.h"

/* Circular Buffer - Thread safe */

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
	const uint32 *ptr_data,
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
 # Uninitialize a Circular Buffer
 @ ptr_cbuf	: pointer to a circular buffer
 */
void cbuf_uninit(struct cbuf *ptr_cbuf)
{
	/* Check pointer of cbuf */
	if (NULL == ptr_cbuf) {
		return;
	}

	kfree((void *)ptr_cbuf->data);
	ptr_cbuf->data = NULL;
}


/*
 # Read an Element from a Circular Buffer
 @ ptr_cbuf : Pointer to a circular buffer 
 @ ptr_val  : Pointer to an output value variable
 */
rtc cbuf_read(struct cbuf *ptr_cbuf, uint32 *ptr_val)
{
	/* Check pointer of cbuf & val */
	if(NULL == ptr_cbuf || NULL == ptr_val) {
		return EINVARG;
	}

	cli();

	if (0 == ptr_cbuf->count) {
		/* No data in the buffer */
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
	/* Check pointer of cbuf */
	if (NULL == ptr_cbuf) {
		return EINVARG;
	}

	cli();

	if (ptr_cbuf->count == ptr_cbuf->capacity) {
		/* Buffer is full */
		sti();
		return EBUFFUL;
	}

	ptr_cbuf->count += 1;
	ptr_cbuf->data[ptr_cbuf->head] = val;
	ptr_cbuf->head = (ptr_cbuf->head + 1) % ptr_cbuf->capacity;

	sti();

	return OK;
}

/*
 # Check Is the Circular Buffer Empty
 @ ptr_cbuf : Pointer to a circular buffer
 */
BOOL cbuf_is_empty(struct cbuf *ptr_cbuf)
{
	/* Check pointer of cbuf */
        if (NULL == ptr_cbuf) {
                return EINVARG;
        }

	return (0 == ptr_cbuf->count) ? TRUE : FALSE;
}


/* String Buffer - Non-thread safe */

/*
 # Initialize a String Buffer
 @ ptr_strbuf	: pointer to a string buffer
 @ capacity	: count of elements in buffer
 @ cstr		: pointer to c-style string which are used to initialize buffer
 */
rtc strbuf_init(struct strbuf *ptr_strbuf, uint32 capacity, const char *cstr)
{
	uint32 len_cstr = 0;

	/* Check pointer of strbuf */
	if (NULL == ptr_strbuf) {
		return EINVARG;
	}

	/* Get length of initial string */
	if (cstr != NULL) {
		len_cstr = strlen(cstr);
	}

	/* Calculate the initial capacity of strbuf */
	capacity = MAX(capacity, len_cstr);
	if (0 == capacity) {
		return EINVARG;
	}

	/* Initialize */
	ptr_strbuf->capacity = capacity;
	ptr_strbuf->length = len_cstr;
	ptr_strbuf->data = (char *)kmalloc(capacity * sizeof(char));

	/* Check memory allocation result */
	if (NULL == ptr_strbuf->data) {
		return EOUTMEM;
	}

	/* Copy initial string */
	if (cstr != NULL && len_cstr != 0) {
		memcpy((void *)ptr_strbuf->data, (const void *)cstr, len_cstr);
	}

	return OK;
}

/*
 # Uninitialize a String Buffer
 @ ptr_strbuf	: pointer to a string buffer
 */
void strbuf_uninit(struct strbuf *ptr_strbuf)
{
	/* Check pointer of strbuf */
	if (NULL == ptr_strbuf) {
		return;
	}

	kfree((void *)ptr_strbuf->data);
	ptr_strbuf->data = NULL;
}

/*
 # Push a Char to a String Buffer, and Automatically
   Increase Buffer Capacity once Buffer is Full
 @ ptr_strbuf	: pointer to a string buffer
 @ val		: input char
 */
rtc strbuf_push(struct strbuf *ptr_strbuf, const char val)
{
	char *new_data;
	uint32 new_capacity;

	/* Check pointer of strbuf */
	if (NULL == ptr_strbuf) {
		return EINVARG;
	}

	/* Check is re-allocation needed */
	if (ptr_strbuf->capacity == ptr_strbuf->length) {
		new_capacity = ptr_strbuf->capacity * 2;
		new_data = krealloc(
				(void *)ptr_strbuf->data,
				new_capacity * sizeof(char)
				);
		/* Re-allocation failed */
		if (NULL == new_data) {
			return EOUTMEM;
		}

		ptr_strbuf->data = new_data;
		ptr_strbuf->capacity = new_capacity;
	}

	/* Push */
	ptr_strbuf->data[ptr_strbuf->length] = val;
	ptr_strbuf->length += 1;

	return OK;
}

/*
 # Pop a Char from a String Buffer
 @ ptr_strbuf	: pointer to a string buffer
 @ ptr_val	: pointer to output char
 */
rtc strbuf_pop(struct strbuf *ptr_strbuf, char *ptr_val)
{
	/* Check pointer of strbuf */
	if (NULL == ptr_strbuf) {
		return EINVARG;
	}

	/* Check buffer empty */
	if (0 == ptr_strbuf->length) {
		return EBUFEMP;
	}

	/* Pop */
	*ptr_val = ptr_strbuf->data[ptr_strbuf->length];
	ptr_strbuf->length -= 1;

	return OK;
}

/*
 # Return an Array of Chars from a String Buffer
 @ ptr_strbuf	: pointer to a string buffer
 @ ptr_str	: pointer to output array of chars
 @ ptr_length	: pointer to lengh of the output array of chars
 */
rtc strbuf_str(struct strbuf *ptr_strbuf, char **ptr_str, uint32 *ptr_length)
{
	/* Check pointer of strbuf */
	if (NULL == ptr_strbuf) {
		return EINVARG;
	}

	/* Return string and length */
	*ptr_str = ptr_strbuf->data;
	*ptr_length = ptr_strbuf->length;

	return OK;
}


/*
 # Empty a String Buffer
 @ ptr_strbuf	: pointer to a string buffer
 */
rtc strbuf_empty(struct strbuf *ptr_strbuf)
{
	/* Check pointer of strbuf */
	if (NULL == ptr_strbuf) {
		return EINVARG;
	}

	ptr_strbuf->length = 0;

	return OK;	
}


