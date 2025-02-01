section .text

; void halt();
global halt
halt:
	hlt
	ret
; END halt

; void terminate();
global terminate
terminate:
	.terminate:
	cli
	hlt
	jmp .terminate
	ret
; END terminate

; void disableInterrupts();
global disableInterrupts
disableInterrupts:
	cli
	ret
; END disableInterrupts

; void enableInterrupts();
global enableInterrupts
enableInterrupts:
	sti
	ret
; END enableInterrupts

; bool CPU_supportsCpuid();
global CPU_supportsCpuid
CPU_supportsCpuid:
	push rbp
	mov rbp, rsp

	; move EFLAGS into eax
	pushfq
	pop rax

	; move (EFLAGS xor 'id flag') back to EFLAGS
	mov rcx, rax		; save eflags for later comparison
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

; void cpuidWrapper(int code, uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx);
; This function assumes that the cpuid instruction is supported. To test it, use CPUSupportsCpuid
global cpuidWrapper
cpuidWrapper:
	push rbp
	mov rbp, rsp

	mov rax, [rbp+8]
	cpuid

	mov rdi, [rbp+12] ; &eax
	mov [rdi], rax
	mov rdi, [rbp+16] ; &ebx
	mov [rdi], rbx
	mov rdi, [rbp+20] ; &ecx
	mov [rdi], rcx
	mov rdi, [rbp+24] ; &edx
	mov [rdi], rdx

	leave
	ret
; END cpuidWrapper
