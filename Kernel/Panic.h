#ifndef __PANIC_H__
#define __PANIC_H__

/// @brief Trigger kernel panic. Will DEFINITELY halt the system
[[noreturn]]
void panic();

/// @brief Prints a message saying mugOS requires the feature 'feature', and panics
void panicForMissingFeature(const char* feature);

#endif
