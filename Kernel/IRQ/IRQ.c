#include <stddef.h>
#include "assert.h"
#include "Logging.h"
#include "HAL/Drivers/IrqChip/IrqChip.h"

#include "IRQ/IRQ.h"
#define MODULE "IRQ"

#define N_IRQ 256
#define isValidIRQ(irq) (irq >= 32 && irq < N_IRQ)

static struct IRQChip* m_chip;
static irqhandler_t m_handlers[N_IRQ]; // note: the first 32 are reserved

void IRQ_init(){
	for (int i=0 ; i<N_IRQ ; i++)
		m_handlers[i] = NULL;

	m_chip = IRQChip_get();
	m_chip->init();
	m_chip->installPrehandler(IRQ_prehandler);

	// Disable all IRQs ; each driver will enable the IRQ it needs
	m_chip->disableAll();
}

void IRQ_enableSpecific(int irq){
	assert(isValidIRQ(irq));
	m_chip->enableSpecific(irq);
}

void IRQ_disableSpecific(int irq){
	assert(isValidIRQ(irq));
	m_chip->disableSpecific(irq);
}

void IRQ_installHandler(int irq, irqhandler_t handler){
	assert(isValidIRQ(irq));
	m_handlers[irq] = handler;
}

void IRQ_removeHandler(int irq){
	assert(isValidIRQ(irq));
	m_handlers[irq] = NULL;
}

void IRQ_prehandler(void* params){
	int irq = IRQChip_getIRQ(params);

	if (m_handlers[irq] != NULL)
		m_handlers[irq](params);
	else
		log(WARNING, MODULE, "Unhandled IRQ %d", irq);

	// Finally, signal the chip that we handled the interrupt
	m_chip->sendEOI(irq);
}
