section .note.GNU-stack noalloc noexec nowrite progbits

section .text

global TSC_read

; uint64_t TSC_read();
TSC_read:
	rdtsc

	; rdtsc returns the TSC value in edx:eax
	; (and clears upper 32 bits of both registers)
	; We need to return the value in rax,
	; so we move edx to the upper 32 bits of rax
	shl rdx, 32
	or rax, rdx

	ret
;
