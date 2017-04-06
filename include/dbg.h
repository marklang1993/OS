#ifndef _DEBUG_H_
#define _DEBUG_H_

#include "type.h"

/* System debug function utilities */

/* assert(exp) */
#ifdef _OS_DBG_
#define assert(exp) \
	if (!(exp))	\
		assert_fail(FALSE, #exp, __FILE__, __BASE_FILE__, __LINE__)

#define kassert(exp) \
	if (!(exp))	\
		assert_fail(TRUE, #exp, __FILE__, __BASE_FILE__, __LINE__)
#else
#define assert(exp)
#define kassert(exp)
#endif

void assert_fail(BOOL is_kernel, const char *exp, const char *file, const char *base_file, uint32 line);

/* panic() */
void panic(const char *format, ...);

#endif
