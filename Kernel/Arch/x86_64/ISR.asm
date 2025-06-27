section .text

; C handler functions, that we call from the interrupt entry points below
extern ISR_exceptionPrehandler
extern IRQ_prehandler
extern ISR_syscallPrehandler

; ISR trap handler
%macro ISR_TRAP_HANDLER 1
global ISR_%1:
ISR_%1:
	; push err							; error code is pushed by the CPU
	push %1								; push interrupt number
	jmp ISR_asmExceptionPrehandler		; jump to common handler code
%endmacro

; ISR exception handler
%macro ISR_EXCEPTION_HANDLER 1
global ISR_%1:
ISR_%1:
	push 0xffffffffffffffff
	push %1
	jmp ISR_asmExceptionPrehandler
%endmacro

; ISR IRQ handler
%macro ISR_IRQ_HANDLER 1
global ISR_%1:
ISR_%1:
	push 0xffffffffffffffff
	push %1
	jmp ISR_asmIRQPrehandler
%endmacro

; ISR syscall handler
%macro ISR_SYSCALL_HANDLER 1
global ISR_%1:
ISR_%1:
	push 0xffffffffffffffff
	push %1
	jmp ISR_asmSyscallPrehandler
%endmacro

; Contains the actual macro calls, that implements all 256 ISRs
%include "ISR_defs.s"

ISR_asmExceptionPrehandler:
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
	mov ax, 0x10	; ensure that we're using kdata segment
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	; Call our global prehandler in C
	mov rdi, rsp					; 1st arg: ISR_Params* params
	call ISR_exceptionPrehandler

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
	pop rbx
	pop rdx
	pop rcx
	pop rax
	add rsp, 16 ; remove error code and IV
	iretq
; END ISR_asmExceptionPrehandler

ISR_asmIRQPrehandler:
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
	mov ax, 0x10	; ensure that we're using kdata segment
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	; Call the global, architecture-agnostic, C IRQ prehandler
	mov rdi, [rbp+0x78]		; 1st arg: int irq
	mov rsi, rsp			; 2nd arg: ISR_Params* params
	call IRQ_prehandler

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
	pop rbx
	pop rdx
	pop rcx
	pop rax
	add rsp, 16 ; remove error code and IV
	iretq
; END ISR_irqPrehandler

ISR_asmSyscallPrehandler:
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
	mov ax, 0x10	; ensure that we're using kdata segment
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	; Call the (temporary) C syscall handler
	mov rdi, rsp			; 1st arg: ISR_Params* params
	call ISR_syscallPrehandler

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
	pop rbx
	pop rdx
	pop rcx
	pop rax
	add rsp, 16 ; remove error code and IV
	iretq
; END ISR_asmSyscallPrehandler
