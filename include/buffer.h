#ifndef _BUFFER_H_
#define _BUFFER_H_

#include "errors.h"
#include "lib.h"

// Circular Buffer

struct cbuf
{
	uint32 capacity;	/* Maximum count of elements in buffer */
        uint32 count;
        uint32 head;
        uint32 tail;
        uint32 *data;
};

rtc cbuf_init(struct cbuf *ptr_cbuf, uint32 count);
rtc cbuf_uninit(struct cbuf *ptr_cbuf);
rtc cbuf_read(struct cbuf *ptr_cbuf, uint32 *ptr_val);
rtc cbuf_write(struct cbuf *ptr_cbuf, uint32 val);

#endif
