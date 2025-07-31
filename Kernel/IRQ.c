#include <stddef.h>
#include "assert.h"
#include "Logging.h"
#include "HAL/Drivers/IRQChip.h"

#include "IRQ.h"
#define MODULE "IRQ"

struct IRQInfo {
	bool enabled;			// Note: `true` doesn't mean that handler is not NULL !!
	IRQHandler handler;		// Installed IRQ handler (nullable)
};

#define N_IRQ 256
#define isValidIRQ(irq) (irq >= 32 && irq < N_IRQ)

static struct IRQChip* m_chip;
static struct IRQInfo m_irqInfos[N_IRQ]; // note: the first 32 are reserved

// TODO write a PIT driver instead
// Simple blinking timer on the bottom right of the screen
#include "Drivers/Graphics/Framebuffer.h"
void timer(void*){
	extern Framebuffer m_framebuffer;
	static bool clock = false;
	const int rect_size = 4;
	color_t color = (clock) ? COLOR_GREEN : COLOR_32BPP(31,31,31);
	Framebuffer_fillRectangle(&m_framebuffer, m_framebuffer.width-rect_size-1, m_framebuffer.height-rect_size-1, rect_size, rect_size, color);
	clock = !clock;
}

void IRQ_init(){
	for (int i=0 ; i<N_IRQ ; i++){
		m_irqInfos[i].enabled = true; // we want 'unhandled IRQ' messages by default
		m_irqInfos[i].handler = NULL;
	}

	m_chip = IRQChip_get();
	m_chip->init();
	m_chip->installPrehandler(IRQ_prehandler);

	// Temporary, until we have a Timer subsystem or whatever
	IRQ_installHandler(IRQ_PIT, timer);
}

void IRQ_enableSpecific(int irq){
	assert(isValidIRQ(irq));
	m_irqInfos[irq].enabled = true;
}

void IRQ_disableSpecific(int irq){
	assert(isValidIRQ(irq));
	m_irqInfos[irq].enabled = false;
}

void IRQ_installHandler(int irq, IRQHandler handler){
	assert(isValidIRQ(irq));
	m_irqInfos[irq].handler = handler;
	m_irqInfos[irq].enabled = true;
}

void IRQ_removeHandler(int irq){
	assert(isValidIRQ(irq));
	m_irqInfos[irq].handler = NULL;
	m_irqInfos[irq].enabled = false;
}

void IRQ_prehandler(void* params){
	int irq = IRQChip_getIRQ(params);

	if (m_irqInfos[irq].enabled){
		if (m_irqInfos[irq].handler != NULL){
			m_irqInfos[irq].handler(params);
		}
		else {
			log(WARNING, MODULE, "Unhandled IRQ %d", irq);
		}
	}

	// Finally, signal the chip that we handled the interrupt
	m_chip->sendEOI(irq);
}
