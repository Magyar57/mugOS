#include "HAL/IRQChip.h"

#include "IRQ.h"

void IRQ_initialize(){
	IRQChip_initialize();
}

void IRQ_registerHandler(uint8_t irq, IRQHandler handler){
	IRQChip_registerHandler(irq, handler);
}

void IRQ_deregisterHandler(uint8_t irq){
	IRQChip_deregisterHandler(irq);
}
