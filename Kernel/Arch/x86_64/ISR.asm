section .text

; Array of C handlers, managed in ISR.c
extern m_handlers
extern ISR_noHandler

; ISR trap handler
%macro ISR_TRAP_HANDLER 1
global ISR_%1:
ISR_%1:
	; push err					; error code is pushed by the CPU
	push %1						; push interrupt number
	jmp ISR_asmPrehandler		; jump to common handler code
%endmacro

; ISR exception handler
%macro ISR_EXCEPTION_HANDLER 1
global ISR_%1:
ISR_%1:
	push 0xffffffffffffffff		; dummy error code
	push %1
	jmp ISR_asmPrehandler
%endmacro

; ISR IRQ handler
%macro ISR_IRQ_HANDLER 1
global ISR_%1:
ISR_%1:
	push 0xffffffffffffffff
	push %1
	jmp ISR_asmPrehandler
%endmacro

; ISR syscall handler
%macro ISR_SYSCALL_HANDLER 1
global ISR_%1:
ISR_%1:
	push 0xffffffffffffffff
	push %1
	jmp ISR_asmPrehandler
%endmacro

; Contains the actual macro calls, that implements all 256 ISRs
%include "ISR_defs.s"

ISR_asmPrehandler:
	; CPU pushed SS:RSP, RFLAGS, cs, rip
	; caller pushed [dummy] error code
	; caller pushed interrupt vector
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

	; Switch gs_base with kernel_gs_base if we came from userspace
	mov rax, [rbp+0x90]		; rax = cs
	test al, 3				; (al & 0b00000011) == 0 ?
	jz .from_kernel
	.from_userspace:
	swapgs
	; Note: if in the future we want to use fs, we must save it here
	.from_kernel:

	; rax = m_handlers[irq]
	mov rax, [rbp+0x78]				; rax = irq
	mov rax, [m_handlers + 8*rax]	; 8*irq cause m_handlers elements are 8 bytes

	; if handler is NULL, call ISR_noHandler instead
	cmp rax, 0
	je .no_handler

	; call the registered handler
	mov rdi, rsp		; 1st arg: ISR_Params* params
	call rax
	jmp .exit

	.no_handler:
	mov rdi, rsp
	call ISR_noHandler

	.exit:

	; Restore user's gs if we came from userspace
	mov rax, [rbp+0x90]		; rax = cs
	test al, 3				; (al & 0b00000011) == 0 ?
	jz .from_kernel_bis
	.from_userspace_bis:
	swapgs
	.from_kernel_bis:

	; restore old segments
	pop rax
	mov ds, ax
	mov es, ax

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
	add rsp, 16 ; remove error code and vector
	iretq
;
