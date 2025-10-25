section .note.GNU-stack noalloc noexec nowrite progbits

section .text

global readTSC

; uint64_t readTSC();
readTSC:
	rdtsc

	; rdtsc returns the TSC value in edx:eax
	; (and clears upper 32 bits of both registers)
	; We need to return the value in rax,
	; so we move edx to the upper 32 bits of rax
	shl rdx, 32
	or rax, rdx

	ret
;
