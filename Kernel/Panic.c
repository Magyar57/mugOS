#include <stddef.h>
#include "Logging.h"
#include "CPU.h"

#include "Panic.h"

void panic(){
	log(PANIC, NULL, "halting");
	halt();
}
