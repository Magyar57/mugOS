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

; void cpuidWrapper(int code, uint32_t* rax, uint32_t* rbx, uint32_t* rcx, uint32_t* rdx);
; This function assumes that the cpuid instruction is supported. To test it, use CPUSupportsCpuid
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

	mov rax, rdi
	cpuid

	; write results

	pop rdi			; rdi = &rdx
	mov [rdi], rdx
	pop rdi			; rdi = &rcx
	mov [rdi], rcx
	pop rdi			; rdi = &rbx
	mov [rdi], rbx
	pop rdi			; rdi = &rax
	mov [rdi], rax

	pop rbx
	leave
	ret
; END cpuidWrapper
