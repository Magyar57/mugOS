section .data

fmt_vector_message:
	db 'Interrupt handler: vector=%p err=%p', 0x0d, 0x0a, 0

section .text

; Tell the assembler about our C functions
extern printf
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
	; CPU pushed
	; caller pushed [dummy] error code
	; caller pushed interrupt number
	; pushad			; push eax, ecx, edx, ebx, esp, ebp, esi, edi
	push rax
	push rcx
	push rdx
	push rbx
	push rsp
	push rbp
	push rsi
	push rdi
	; push r8
	; push r9
	; push r10
	; push r11
	; push r12 ; TODO ???
	; push r13
	; push r14
	; push r15
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
	push rsp
	call ISR_C_prehandler
	add rsp, 8 ; or 8 in long mode ?

	; Call printf from assembly
	; mov eax, [ebp+36] ; err
	; push eax
	; mov eax, [ebp+32] ; vector
	; push eax
	; push fmt_vector_message
	; call printf
	; add esp, 12

	; restore old segment
	pop rax
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	; restore registers
	; push r15
	; push r14
	; push r13
	; push r12 ; TODO ???
	; push r11
	; push r10
	; push r9
	; push r8
	push rdi
	push rsi
	push rbp
	push rsp
	push rbx
	push rdx
	push rcx
	push rax
	add rsp, 16 ; remove error code and IV
	iret
; END ISR_asmPrehandler
