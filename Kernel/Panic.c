#include "stdio.h"
#include "Arch/CPU.h"

#include "Panic.h"

void PANIC(){
	puts("KERNEL PANIC!");
	halt();
}
