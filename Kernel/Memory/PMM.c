#include <stdint.h>
#include <stddef.h>
#include "Logging.h"
#include "Boot/LimineRequests.h"

#include "PMM.h"

static const char* prefixes[] = {
	"B", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB",
	"ZiB" // not representible on uint64 (max is ~16 EiB)
};

static const char* memoryTypeNames[] = {
	"MEMORY_USABLE",
	"MEMORY_RESERVED",
	"MEMORY_ACPI_RECLAIMABLE",
	"MEMORY_ACPI_NVS",
	"MEMORY_BAD_MEMORY",
	"MEMORY_BOOTLOADER_RECLAIMABLE",
	"MEMORY_EXECUTABLE_AND_MODULES",
	"MEMORY_FRAMEBUFFER",
};

/// @returns n so that `1024^(n+1) >= byte_number > 1024^n`
/// @note Example: `512 B` => `0`, `1023 B` => `0`, `1024 B` => `1`, `1025 B` => `1`, `1048576` => `3`...
static int getMagnitude(uint64_t byte_number){
    int i = 0;
    while (byte_number >= 1024){
        byte_number /= 1024;
        i++;
    }
    return i;
}

void PMM_printMemoryMap(){
	size_t total_referenced_memory = 0;
	size_t total_usable_memory = 0;
	size_t totla_usable_reclaimable_memory = 0;

	for (uint64_t i=0 ; i<memmapReq.response->entry_count ; i++){
		struct limine_memmap_entry* cur = memmapReq.response->entries[i];

		int magnitude = getMagnitude(cur->length);
		magnitude = (magnitude > 6) ? 6 : magnitude; // clamp magnitude to the max prefix available on 64 bits
		uint64_t divisor = (1llu << (10*magnitude)); // 1024**magnitude = 2**(10*magnitude)

		log(INFO, "MEMMAP", "%#18.8llx %#18.8llx (%4d %s) - %lld %s ", cur->base, cur->length, cur->length/divisor, prefixes[magnitude], cur->type, memoryTypeNames[cur->type]);
		total_referenced_memory += cur->length;

		if (cur->type == LIMINE_MEMMAP_USABLE || cur->type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE || cur->type == LIMINE_MEMMAP_ACPI_RECLAIMABLE)
		totla_usable_reclaimable_memory += cur->length;
		if (cur->type == LIMINE_MEMMAP_USABLE)
			total_usable_memory += cur->length;
	}

	int magnitude = getMagnitude(total_referenced_memory);
	magnitude = (magnitude > 6) ? 6 : magnitude;
	uint64_t divisor = (1llu << (10*magnitude));
	log(INFO, "MEMMAP", "Total memory referenced by the map: %llu %s (%#llx)", total_referenced_memory/divisor, prefixes[magnitude], total_referenced_memory);

	magnitude = getMagnitude(total_usable_memory);
	magnitude = (magnitude > 6) ? 6 : magnitude;
	divisor = (1llu << (10*magnitude));
	log(INFO, "MEMMAP", "Total usable memory: %llu %s (%#llx)", total_usable_memory/divisor, prefixes[magnitude], total_usable_memory);

	magnitude = getMagnitude(totla_usable_reclaimable_memory);
	magnitude = (magnitude > 6) ? 6 : magnitude;
	divisor = (1llu << (10*magnitude));
	log(INFO, "MEMMAP", "Total usable+reclaimable memory: %llu %s (%#llx)", totla_usable_reclaimable_memory/divisor, prefixes[magnitude], totla_usable_reclaimable_memory);
}
