section .text

; bool CPU_supportsCpuid();
global CPU_supportsCpuid
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
; END CPU_supportsCpuid

; void cpuidWrapper(int leaf, uint32_t* eaxOut, uint32_t* ebxOut, uint32_t* ecxOut, uint32_t* edxOut);
; This function assumes that the cpuid instruction is supported. To test it, use CPU_supportsCpuid
global cpuidWrapper
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
; END cpuidWrapper

; void cpuidWrapperWithSubleaf(int leaf, int subleaf, uint32_t* eaxOut, uint32_t* ebxOut, uint32_t* ecxOut, uint32_t* edxOut);
; This function assumes that the cpuid instruction is supported. To test it, use CPU_supportsCpuid
global cpuidWrapperWithSubleaf
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
; END cpuidWrapper

; uint64_t CPU_readMSR(int msr);
; Reads and returns the value of the `msr` Model-Specific Register
; Warning: Does NOT check whether the MSR and the `rdmsr` instruction are supported
global CPU_readMSR
CPU_readMSR:
	; rdi = msr

	mov ecx, edi
	rdmsr

	; Return value of rdmsr is in edx:eax
	; High 32 bits of both registers are 0
	shl rdx, 32
	or rax, rdx

	ret
; END CPU_readMSR
