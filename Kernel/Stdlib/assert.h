#ifndef __ASSERT_H__
#define __ASSERT_H__

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

void __assert_fail(const char* assertion, const char* file, unsigned int line, const char* func);

#define assert(expr) \
	((expr) ? __ASSERT_VOID_CAST(0) : __assert_fail(#expr, __FILE__, __LINE__, __FUNCTION__))

#endif

#endif
