#ifndef __ERRNO_H__
#define __ERRNO_H__

// errno.h: the 'errno' variable & error numbers definitions (as macros)
// We define the POSIX erros, as well as the kernel errors.
// Note that the kernel uses special names, and makes kernel_error = -POSIX_error

// errno is not used in the kernel
#ifndef KERNEL
extern int errno;
#endif

// ================ POSIX/userspace error numbers ================

/// @brief Error: No such file or directory
#define ENOENT			2
/// @brief Error: Out of memory error
#define ENOMEM			12
/// @brief Error: Invalid argument
#define EINVAL			22

// ================ Kernel error numbers ================

#ifdef KERNEL

/// @brief Code for success
#define E_SUCCESS		0

#define E_NOENT			-ENOENT
#define E_NOMEM			-ENOMEM
#define E_INVAL			-EINVAL

#endif // def KERNEL

#endif
