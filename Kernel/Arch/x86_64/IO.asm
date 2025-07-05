section .text

; #include "io.h"

; void x86_outb(uint16_t port, uint8_t value);
global outb
outb:
	mov rdx, rdi ; dx = port
	mov rax, rsi ; al = value

	out dx, al
	ret
; END outb

; uint8_t inb(uint16_t port);
global inb
inb:
	mov rdx, rdi ; dx = port

	xor rax, rax
	in al, dx
	ret
; END inb

; void io_wait(); (unused function)
global io_wait
io_wait:
	xor rax, rax
	out 0x80, al
	ret
; END io_wait
