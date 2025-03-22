#include "HAL/IRQChip.h"

#include "IRQ.h"

void IRQ_initialize(){
	IRQChip_initialize();
}

void IRQ_enableSpecific(int irq){
	IRQChip_enableSpecific(irq);
}

void IRQ_disableSpecific(int irq){
	IRQChip_disableSpecific(irq);
}

void IRQ_registerHandler(int irq, IRQHandler handler){
	IRQChip_registerHandler(irq, handler);
}

void IRQ_deregisterHandler(int irq){
	IRQChip_deregisterHandler(irq);
}
