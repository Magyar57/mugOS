#include "stdio.h"
#include "CPU.h"

#include "Panic.h"

void PANIC(){
	puts("KERNEL PANIC!");
	halt();
}
