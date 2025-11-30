#ifndef __ERRNO_H__
#define __ERRNO_H__

// errno is not used in the kernel
#ifndef KERNEL
extern int errno;
#endif

// ================ Error numbers (kernel) ================

// Error code for success
#define E_SUCCESS		0

// Out of memory error
#define E_NOMEM			-1
// Invalid input value error
#define E_INVAL			-2

// ================ Error numbers (userspace, POSIX) ================

#ifndef KERNEL

#define ENOMEM			E_NOMEM
#define EINVAL			E_INVAL

#endif // ndef KERNEL

#endif
