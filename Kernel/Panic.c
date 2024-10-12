#include "stdio.h"
#include "Arch/Arch.h"

#include "Panic.h"

void PANIC(){
	puts("KERNEL PANIC!");
	halt();
}
