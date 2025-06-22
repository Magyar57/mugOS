#ifndef __MEMORY_MAP_H__
#define __MEMORY_MAP_H__

#include <stddef.h>
#include "Memory/Memory.h"

enum MemoryType {
	MEMORY_USABLE,
	MEMORY_RESERVED,
	MEMORY_KERNEL, // ktext & kdata
	MEMORY_FRAMEBUFFER,
	MEMORY_ACPI_NVS,
	MEMORY_ACPI_RECLAIMABLE,
	MEMORY_BOOTLOADER_RECLAIMABLE,
};

#define MMAP_MAX_ENTRIES 128

struct MemoryMap {
	size_t totalUsableMemory; // in bytes
	physical_address_t firstUsablePage;
	physical_address_t lastUsablePage;
	physical_address_t kernelAddress;
	size_t kernelSize;

	int size; // number of MemoryMapEntry in entries
	struct MemoryMapEntry {
		physical_address_t address;
		uint64_t length;
		enum MemoryType type;
	} entries[MMAP_MAX_ENTRIES];
};

extern struct MemoryMap g_memoryMap;

/// @brief Initialize the Memory Map.
/// @param memmap The memory map to initialize. Should be `&g_memoryMap`
/// @param firmware_mmap The bootloader or firmware memory map.
/// Only Limine's `struct limine_memmap_response*` supported
void MMap_init(struct MemoryMap* memmap, void* firmware_mmap);

// ================ Display number in powers of two ================

static const char* SIZE_UNITS[] = {
	"B", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB",
	"ZiB" // not representible on uint64 (max is ~16 EiB)
};

/// @brief Returns a magnitude in order to print sizes
/// @returns n so that `1024^(n+1) >= byte_number > 1024^n`.
/// Example: `512 B` => `0`, `1023 B` => `0`, `1024 B` => `1`, `1025 B` => `1`, `1048576` => `3`...
/// @note Use with SIZE_UNITS to print sizes:
/// ```
/// uint64_t size = 57*1024*1024;
/// int magnitude = getMagnitude(size);
/// magnitude = (magnitude > 6) ? 6 : magnitude;
/// int divisor = (1llu << (10*magnitude));
/// uint64_t size_atMagnitude = (divisor == 0) ? size : size/divisor;
/// printf("Some object's size: %llu %s", size_atMagnitude, SIZE_UNITS[magnitude_usedMem]);
/// ```
int getMagnitude(uint64_t byte_number);

#endif
