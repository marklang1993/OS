#ifndef _BUFFER_H_
#define _BUFFER_H_

#include "errors.h"
#include "lib.h"

// Circular Buffer
#define SIZE_CIRCULAR_BUFFER		16

struct cbuf
{
        uint32 count;
        uint32 head;
        uint32 tail;
        uint8 data[SIZE_CIRCULAR_BUFFER];
};

rtc cbuf_init(struct cbuf *ptr_cbuf);
rtc cbuf_read(struct cbuf *ptr_cbuf, uint8 *ptr_val);
rtc cbuf_write(struct cbuf *ptr_cbuf, uint8 val);

#endif
