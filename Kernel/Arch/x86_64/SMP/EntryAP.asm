section .note.GNU-stack noalloc noexec nowrite progbits

; EntryAP.asm: Entry point for the APs (Auxiliary Processors, aka non-BSP processors)

; The entry point for APs gets its own section, as it needs special alignment
; The 'exec' is a flag for the linker, to get proper debugging
section .ap_entry exec

; The CPUs start in 16 bits real mode
bits 16

global entryAP:function
global endEntryAP

entryAP:
	cli
	hlt
	jmp entryAP

	endEntryAP:
	; this label is used for computing the size of the function
;
