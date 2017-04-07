#ifndef _BUFFER_H_
#define _BUFFER_H_

#include "errors.h"

/* Circular Buffer - Thread safe */
struct cbuf
{
	uint32 capacity;	/* Maximum count of elements in buffer */
        uint32 count;
        uint32 head;
        uint32 tail;
        uint32 *data;
};

rtc cbuf_init(struct cbuf *ptr_cbuf, uint32 capacity, const uint32 *ptr_data, uint32 size);
void cbuf_uninit(struct cbuf *ptr_cbuf);
rtc cbuf_read(struct cbuf *ptr_cbuf, uint32 *ptr_val);
rtc cbuf_write(struct cbuf *ptr_cbuf, uint32 val);
BOOL cbuf_is_empty(struct cbuf *ptr_cbuf);

/* String Buffer - Non-thread safe */
struct strbuf
{
	uint32 capacity;
	uint32 length;
	char *data;
};

rtc strbuf_init(struct strbuf *ptr_strbuf, uint32 capacity, const char *cstr);
void strbuf_uninit(struct strbuf *ptr_strbuf);
rtc strbuf_push(struct strbuf *ptr_strbuf, const char val);
rtc strbuf_pop(struct strbuf *ptr_strbuf, char *ptr_val);
rtc strbuf_str(struct strbuf *ptr_strbuf, char **ptr_str, uint32 *ptr_length);
rtc strbuf_empty(struct strbuf *ptr_strbuf);

#endif
