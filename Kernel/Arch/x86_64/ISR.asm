section .data

isr_message:
	db 'ISR occurred', 0

section .text

; Tell the assembler about our C functions
extern puts
extern ISR_C_prehandler

; ISR Trap Handler (we have an error code)
%macro ISR_TRAP_HANDLER 1
global ISR_%1:
ISR_%1:
	; push err			; error code is pushed by the CPU
	push %1				; push interrupt number
	jmp ISR_asmPrehandler	; jump to common handler code
%endmacro

; ISR Trap Handler (we don't have an error code)
%macro ISR_IRQ_OR_SYSCALL_HANDLER 1
global ISR_%1:
ISR_%1:
	push 0xffffffffffffffff				; push dummy/placeholder error code
	push %1						; push interrupt number
	jmp ISR_asmPrehandler		; jump to common handler code
%endmacro

%include "ISR_defs.s"

ISR_asmPrehandler:
	; CPU pushed SS:RSP, RFLAGS, cs, rip
	; caller pushed [dummy] error code
	; caller pushed interrupt number (vector)
	push rax
	push rcx
	push rdx
	push rbx
	push rbp
	push rsi
	push rdi
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
	mov rbp, rsp

	xor rax, rax	; push ds
	mov ax, ds		; push ds
	push rax		; push ds
	mov ax, 0x10	; assure that we're using kdata segment
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	; Call our global pre-handler in C
	mov rdi, rsp ; first (only) argument: ISR_Params* params
	call ISR_C_prehandler

	; Call puts
	; lea rdi, [rel isr_message]
	; call puts

	; restore old segment
	pop rax
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	; restore registers
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rdi
	pop rsi
	pop rbp
	pop rsp
	pop rbx
	pop rdx
	pop rcx
	pop rax
	add rsp, 16 ; remove error code and IV
	iretq
; END ISR_asmPrehandler
