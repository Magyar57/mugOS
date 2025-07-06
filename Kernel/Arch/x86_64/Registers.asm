section .text

global Registers_readMSR
global Registers_writeMSR
global Registers_readCR0
global Registers_writeCR0
global Registers_readCR4
global Registers_writeCR4

; uint64_t Registers_readMSR(int msr);
; Reads and returns the value of the `msr` Model-Specific Register
; Warning: Does NOT check whether the MSR and the `rdmsr` instruction are supported
Registers_readMSR:
	; rdi = msr

	mov ecx, edi
	rdmsr

	; Return value of rdmsr is in edx:eax
	; High 32 bits of both registers are 0
	shl rdx, 32
	or rax, rdx

	ret
;

; void Registers_writeMSR(int msr, uint64_t value);
; Write the value `value` to the `msr` Model-Specific Register
; Warning: Does NOT check whether the MSR and the `wrmsr` instruction are supported
Registers_writeMSR:
	; rdi = msr
	; rsi = value

	; Set edx:eax to MSR value
	mov rdx, rsi
	shr rdx, 32
	mov eax, esi

	mov ecx, edi
	wrmsr

	ret
;

Registers_readCR0:
	mov rax, cr0
	ret
;

Registers_writeCR0:
	mov cr0, rdi
	ret
;

Registers_readCR4:
	mov rax, cr4
	ret
;

Registers_writeCR4:
	mov cr4, rdi
	ret
;
