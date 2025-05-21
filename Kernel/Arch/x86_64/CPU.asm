section .text

global CPU_supportsCpuid
global cpuidWrapper
global cpuidWrapperWithSubleaf
global Registers_readMSR
global Registers_writeMSR
global Registers_readCR0
global Registers_writeCR0
global Registers_readCR4
global Registers_writeCR4

; bool CPU_supportsCpuid();
CPU_supportsCpuid:
	push rbp
	mov rbp, rsp

	; move EFLAGS into eax
	pushfq
	pop rax

	; move (EFLAGS xor 'id flag') back to EFLAGS
	mov rcx, rax		; save EFLAGS for later comparison
	xor rax, 1<<21		; toggle bit 21 (ID flag)
	push rax
	popfq				; eflags = eax

	; re-move EFLAGS into eax, and compare it with the previous value
	pushfq
	pop rax
	xor rax, rcx
	and rax, 1<<21		; if bit clear, both EFLAGS had the same id flag value (cpuid unsupported)
	shr rax, 21			; return value is on 8 bits

	leave
	ret
;

; void cpuidWrapper(int leaf, uint32_t* eaxOut, uint32_t* ebxOut, uint32_t* ecxOut, uint32_t* edxOut);
; This function assumes that the cpuid instruction is supported. To test it, use CPU_supportsCpuid
cpuidWrapper:
	push rbp
	mov rbp, rsp
	push rbx

	; preserve args for later
	push rsi
	push rdx
	push rcx
	push r8

	xor ecx, ecx ; sub-leaf = 0
	mov eax, edi
	cpuid

	; write results
	pop rdi			; rdi = edxOut
	mov [rdi], edx
	pop rdi			; rdi = ecxOut
	mov [rdi], ecx
	pop rdi			; rdi = ebxOut
	mov [rdi], ebx
	pop rdi			; rdi = eaxOut
	mov [rdi], eax

	pop rbx
	leave
	ret
;

; void cpuidWrapperWithSubleaf(int leaf, int subleaf, uint32_t* eaxOut, uint32_t* ebxOut, uint32_t* ecxOut, uint32_t* edxOut);
; This function assumes that the cpuid instruction is supported. To test it, use CPU_supportsCpuid
cpuidWrapperWithSubleaf:
	push rbp
	mov rbp, rsp
	push rbx

	; preserve args for later
	push rdx
	push rcx
	push r8
	push r9

	mov ecx, esi ; ecx = sub-leaf (which is in esi)
	mov eax, edi
	cpuid

	; write results
	pop rdi			; rdi = edxOut
	mov [rdi], edx
	pop rdi			; rdi = ecxOut
	mov [rdi], ecx
	pop rdi			; rdi = ebxOut
	mov [rdi], ebx
	pop rdi			; rdi = eaxOut
	mov [rdi], eax

	pop rbx
	leave
	ret
;

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
