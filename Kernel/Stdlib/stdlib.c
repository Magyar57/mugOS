#ifdef KERNEL
#include "Panic.h"
#else
#error "Stdlib unsupported in userspace yet"
#endif

#include "stdlib.h"

void abort(){
	#ifdef KERNEL
		panic();
	#else
		// TODO we need an kill or exit syscall
	#endif
}
