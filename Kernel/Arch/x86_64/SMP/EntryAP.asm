; EntryAP.asm: Entry point for the APs (Auxiliary Processors, aka non-BSP processors)

; The CPUs start in 16 bits real mode
bits 16

; The entry point for APs gets its own section, as it needs special alignment
; The 'exec' is a flag for the linker, to get proper debugging
section .ap_entry exec

global entryAP
entryAP:
	cli
	hlt
	jmp entryAP
