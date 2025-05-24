#include "string.h"
#include "Logging.h"
#include "Boot/LimineRequests.h"
#include "Memory/MemoryMap.h"
#include "Memory/PMM.h"
#include "Memory/VMM.h"
#include "IRQ.h"
#include "HAL/HAL.h"
#include "HAL/CPU.h"
#include "Drivers/Graphics/Graphics.h"
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
	Graphics_initialize(GRAPHICS_LIMINE_FRAMEBUFFER, (void *)framebufferReq.response->framebuffers[0]);
	Graphics_clearScreen();
	log(INFO, NULL, "Supposons que je sois dans votre kernel !");
	log(INFO, "mugOS", "Loaded by: %s v%s", infoReq.response->name, infoReq.response->version);

	// Hardware-specific initialization
	HAL_initialize();

	// Memory management initialization
	MMap_initialize(&g_memoryMap, memmapReq.response);
	VMM_setHHDM(hhdmReq.response->offset);
	PMM_initialize();
	VMM_initialize();

	// Initialize IRQs
	IRQ_initialize();
	IRQ_enable();

	// Drivers initialization
	Serial_initialize();
	PS2_initialize();
	Keyboard_initialize();

	// Infinite loop: whenever an interrupts fire, handle it ; then stop again.
	while (true)
		halt();
}
