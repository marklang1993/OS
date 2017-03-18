#ifndef _ERRORS_H_
#define _ERRORS_H_

#include "type.h"

typedef		int32		rtc;

// Error Code Definitions
#define		OK		0
#define		EINVARG		-1		// Invalid arguments
#define		EBUFEMP		-2		// Buffer empty
#define		EBUFFUL		-3		// Buffer full
#define		EINVIDX		-4		// Invalid index
#define		ENOTINIT	-5		// NOT initialized
#define		EHEAPMEM	-6		// Heap memory error
#define		EINVPTR		-7		// Invalid pointer

#endif
