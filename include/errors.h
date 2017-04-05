#ifndef _ERRORS_H_
#define _ERRORS_H_

#include "type.h"

typedef		int32		rtc;

/* Information/Error Code Definitions */
#define		INOMSG		1		/* No message available */
#define		OK		0		/* OK */
#define		EINVARG		-1		/* Invalid arguments */
#define		EBUFEMP		-2		/* Buffer empty */
#define		EBUFFUL		-3		/* Buffer full */
#define		EINVIDX		-4		/* Invalid index */
#define		ENOTINIT	-5		/* NOT initialized */
#define		EHEAPMEM	-6		/* Heap memory error */
#define		EINVPTR		-7		/* Invalid pointer */
#define		EOUTMEM		-8		/* Out of memory */
#define		EDLOCK		-9		/* Deadlock */
#define		EINVPID		-10		/* Invalid PID */

#endif
