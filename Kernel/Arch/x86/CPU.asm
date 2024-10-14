bits 32

section .text

; void x86_halt();
global x86_halt
x86_halt:
	.halt:
	cli
	hlt
	jmp .halt
; END x86_halt
