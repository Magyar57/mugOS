#ifndef __PANIC_H__
#define __PANIC_H__

/// @brief Trigger kernel panic. Will DEFINITELY halt the system
/// @return Never returns
[[noreturn]]
void panic();

/// @brief Panic helper to handle an errno
/// @param errorStr Error string to print. Result of `strerror(errno)` will be appended at its end
/// @param errno The error number (0 for success)
/// @return Never returns if `errno != 0`
void panicOnError(const char* errorStr, int errno);

/// @brief Prints a message saying mugOS requires the feature 'feature', and panics
/// @return Never returns
void panicForMissingFeature(const char* feature);

#endif
