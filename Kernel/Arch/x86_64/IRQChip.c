#include "assert.h"
#include "Logging.h"
#include "CPU.h"
#include "Drivers/i8259.h"
#include "Drivers/APIC.h"
#include "HAL/IRQFlags.h"
#include "ISR.h"

#include "HAL/Drivers/IRQChip.h"
#define MODULE "IRQ Chip"

static struct IRQChip m_chip;

static void installPrehandler(irqhandler_t prehandler){
	for (int irq=32 ; irq<256 ; irq++){
		if (!ISR_isHandlerPresent(irq)){
			ISR_installHandler(irq, (isr_t) prehandler);
		}
	}
}

struct IRQChip* IRQChip_get(){
	// Try to use the APIC if available
	if (g_CPU.features.bits.APIC){
		log(INFO, MODULE, "APIC found");
		m_chip.init = APIC_init;
		m_chip.sendEOI = APIC_sendEIO;
	}
	else {
		log(INFO, MODULE, "APIC not found, using legacy 8259 PIC");
		m_chip.init = i8259_init;
		m_chip.sendEOI = i8259_sendEIO;
	}

	m_chip.installPrehandler = installPrehandler;
	return &m_chip;
}

int IRQChip_getIRQ(void* params){
	struct ISR_Params* isr_params = params;
	return isr_params->vector;
}
