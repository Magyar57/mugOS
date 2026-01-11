#include "stdlib.h"
#include "string.h"
#include "Logging.h"
#include "Boot/LimineRequests.h"
#include "Memory/MemoryMap.h"
#include "Memory/PMM.h"
#include "Memory/VMM.h"
#include "IRQ/IRQ.h"
#include "Time/Time.h"
#include "SMP/SMP.h"
#include "Filesystem/VFS.h"
#include "Drivers/Graphics/Graphics.h"
#include "Drivers/ACPI/ACPI.h"
#include "Drivers/Output/Serial.h"
#include "Drivers/Input/PS2.h"
#include "Drivers/Input/Keyboard.h"
#include "HAL/HAL.h"
#include "HAL/Halt.h"

extern uint8_t __bss_start;
extern uint8_t __end;

// Kernel entry point
void kmain(){
	// Clear uninitialized data
	memset(&__bss_start, 0, (&__end) - (&__bss_start));

	// Early graphics setup (for logging)
	Graphics_init(GRAPHICS_LIMINE_FRAMEBUFFER, (void*)g_framebufferReq.response->framebuffers[0]);
	Graphics_clearScreen();
	log(INFO, NULL, "Supposons que je sois dans votre kernel !");
	log(INFO, NULL, "Welcome to mugOS v%d.%d !", MUGOS_MAJOR, MUGOS_MINOR);

	// Hardware-specific initialization
	HAL_init();

	// Memory management
	MMap_init(&g_memoryMap, g_memmapReq.response);
	VMM_setHHDM(g_hhdmReq.response->offset);
	PMM_init();
	VMM_init();
	SlabAllocator_init(); // Kernel heap (kmalloc, caches...)

	ACPI_init();

	// IRQ subsystem initialization, and enable IRQs for the boostrap CPU
	IRQ_init();
	IRQ_enable();

	// Time subsystem
	Time_init();

	// CPUs and multiprocessing
	SMP_init();
	SMP_startCPUs();

	// Virtual filesystem and filesystem drivers
	VFS_init();

	// Misc drivers
	Serial_init();
	PS2_init();
	Keyboard_init();

	// Infinite loop: whenever an interrupts fire, handle it ; then stop again.
	while (true)
		halt();
}
