#ifndef __ASSERT_H__
#define __ASSERT_H__

#include "Logging.h"
#include "Panic.h"

// If NDEBUG is defined, do nothing
#ifdef NDEBUG

#define assert(expr) (__ASSERT_VOID_CAST (0))

#else // Not NDEBUG.

// ASSERT_VOID_CAST
#if defined __cplusplus
#define __ASSERT_VOID_CAST static_cast<void>
#else // not C++ (aka C)
#define __ASSERT_VOID_CAST (void)
#endif // C++

// assert

#define assert(expr) ((expr) ? __ASSERT_VOID_CAST(0) : __assert_fail(#expr, __FILE__, __LINE__, __FUNCTION__))

#define __assert_fail(expr_str, file, line, func) ({log(PANIC, __FILE__,"assertion '%s' failed at %s:%d in %s", expr_str, file, line, func); panic();})

#endif

// debug("print at %s:%d\n", __FILE__, __LINE__);
// debug("compile time is %s %s\n", __DATE__, __TIME__);

#endif
