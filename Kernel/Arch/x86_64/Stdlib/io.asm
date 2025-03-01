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

; void io_wait();
global io_wait
io_wait:
	mov rdi, 0x80	; 0x80 is an unused port
	mov rsi, 0
	call outb		; outb(0x80, 0)
	ret
; END io_wait
