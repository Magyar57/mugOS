#include <stddef.h>
#include "stdlib.h"

#ifdef KERNEL
#include "Logging.h"
#else // userspace library
#include "stdio.h"
#endif

#include "assert.h"

void __assert_fail(const char* assertion, const char* file, unsigned int line, const char* func){
	#ifdef KERNEL
		log(PANIC, NULL, "Assertion '%s' failed at %s:%d in %s", assertion, file, line, func);
	#else
		fprintf(stderr, "Assertion '%s' failed at %s:%d in %s\n", assertion, file, line, func);
	#endif

	abort();
}
