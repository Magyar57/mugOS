#include "assert.h"
#include "Logging.h"
#include "HAL/IRQ/IrqFlags.h"
#include "CPU/CPU.h"
#include "Platform/ISR.h"
#include "Drivers/IrqChip/i8259.h"
#include "Drivers/IrqChip/APIC.h"
#include "Drivers/IrqChip/IOAPIC.h"

#include "HAL/Drivers/IrqChip/IrqChip.h"
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
		m_chip.enableSpecific = IOAPIC_enableSpecific;
		m_chip.disableSpecific = IOAPIC_disableSpecific;
		m_chip.enableAll = IOAPIC_enableAllIRQ;
		m_chip.disableAll = IOAPIC_disableAllIRQ;
	}
	else {
		log(INFO, MODULE, "APIC not found, using legacy 8259 PIC");
		m_chip.init = i8259_init;
		m_chip.sendEOI = i8259_sendEIO;
		m_chip.enableSpecific = i8259_enableSpecific;
		m_chip.disableSpecific = i8259_disableSpecific;
		m_chip.enableAll = i8259_enableAllIRQ;
		m_chip.disableAll = i8259_disableAllIRQ;
	}

	m_chip.installPrehandler = installPrehandler;
	return &m_chip;
}

int IRQChip_getIRQ(void* params){
	struct ISR_Params* isr_params = params;
	return isr_params->vector;
}
