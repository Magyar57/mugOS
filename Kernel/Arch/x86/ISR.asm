bits 32

section .data

fmt_vector_message:
	db 'Interrupt handler: vector=%p err=%p', 0x0d, 0x0a, 0

section .text

; Tell the assembler about our C functions
extern printf
extern x86_ISR_C_Prehandler

; ISR Trap Handler (we have an error code)
%macro ISR_TRAP_HANDLER 1
global x86_ISR_%1:
x86_ISR_%1:
	; push err			; error code is pushed by the CPU
	push %1				; push interrupt number
	jmp x86_ISR_Asm_Prehandler	; jump to common handler code
%endmacro

; ISR Trap Handler (we don't have an error code)
%macro ISR_IRQ_OR_SYSCALL_HANDLER 1
global x86_ISR_%1:
x86_ISR_%1:
	push 0xffffffff				; push dummy/placeholder error code
	push %1						; push interrupt number
	jmp x86_ISR_Asm_Prehandler	; jump to common handler code
%endmacro

%include "ISR_defs.s"

x86_ISR_Asm_Prehandler:
	; CPU pushed 
	; caller pushed [dummy] error code
	; caller pushed interrupt number
	pushad			; push eax, ecx, edx, ebx, esp, ebp, esi, edi
	mov ebp, esp
	xor eax, eax	; push ds
	mov ax, ds		; push ds
	push eax		; push ds
	mov ax, 0x10	; assure that we're using kdata segment
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	; Call our global pre-handler in C
	push esp
	call x86_ISR_C_Prehandler
	add esp, 4

	; Call printf from assembly
	; mov eax, [ebp+36] ; err
	; push eax
	; mov eax, [ebp+32] ; vector
	; push eax
	; push fmt_vector_message
	; call printf
	; add esp, 12

	; Halt if vector=13 (memory segfault)
	; mov eax, [ebp+32]
	; cmp eax, 0x0d
	; jne .after 
	; hlt
	; .after:

	; restore old segment
	pop eax
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	popad ; restore registers
	add esp, 8 ; remove error code and IV
	iret
; END x86_ISR_Asm_Prehandler
