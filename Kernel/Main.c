#include "stdlib.h"
#include "string.h"
#include "Logging.h"
#include "Boot/LimineRequests.h"
#include "Memory/MemoryMap.h"
#include "Memory/PMM.h"
#include "Memory/VMM.h"
#include "SMP/SMP.h"
#include "IRQ.h"
#include "HAL/HAL.h"
#include "HAL/Halt.h"
#include "Drivers/Graphics/Graphics.h"
#include "Drivers/ACPI/ACPI.h"
#include "Drivers/Serial.h"
#include "Drivers/PS2.h"
#include "Drivers/Keyboard.h"

extern uint8_t __bss_start;
extern uint8_t __end;

// Kernel entry point
void kmain(){
	// Clear uninitialized data
	memset(&__bss_start, 0, (&__end) - (&__bss_start));

	// Early graphics setup (for logging)
	Graphics_init(GRAPHICS_LIMINE_FRAMEBUFFER, (void *)g_framebufferReq.response->framebuffers[0]);
	Graphics_clearScreen();
	log(INFO, NULL, "Supposons que je sois dans votre kernel !");
	log(INFO, "mugOS", "Loaded by: %s v%s", g_infoReq.response->name, g_infoReq.response->version);

	// Hardware-specific initialization
	HAL_init();

	// Memory management initialization
	MMap_init(&g_memoryMap, g_memmapReq.response);
	VMM_setHHDM(g_hhdmReq.response->offset);
	PMM_init();
	VMM_init();
	SlabAllocator_init(); // Kernel heap (kmalloc, caches...)

	ACPI_init();

	// CPUs intializations
	SMP_init();
	IRQ_init();
	SMP_startCPUs();
	IRQ_enable();

	// Drivers initializations
	Serial_init();
	PS2_init();
	Keyboard_init();

	// Infinite loop: whenever an interrupts fire, handle it ; then stop again.
	while (true)
		halt();
}
