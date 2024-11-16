bits 32

section .text

; void halt();
global halt
halt:
	.halt:
	cli
	hlt
	jmp .halt
; END halt
