#ifndef __PREPROCESSOR_H__
#define __PREPROCESSOR_H__

// Preprocessor.h:
// Maccros and checks ran at compile time

// Force the compiler to always inline the function
#define always_inline inline __attribute__((__always_inline__))

// Pack a structure (aka do not pad it with zeroes entries)
#define packed __attribute__((packed))

#define aligned(num) __attribute__((aligned(num)))

// Checks the type of the variable var at compile time
// Note: _Static_assert was deprecated in favour of static_assert in C23
#define typecheck(type, var) static_assert(_Generic((var), type: 1, default: 0), "Variable is not of type " # type)

#define compile_assert(condition) static_assert(condition, "Compile-time assertion failed: " # condition)

#endif
