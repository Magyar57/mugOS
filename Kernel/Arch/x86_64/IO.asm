section .text

global outb
global inb
global io_wait

; void x86_outb(uint16_t port, uint8_t value);
outb:
	mov rdx, rdi ; dx = port
	mov rax, rsi ; al = value

	out dx, al
	ret
;

; uint8_t inb(uint16_t port);
inb:
	mov rdx, rdi ; dx = port

	xor rax, rax
	in al, dx
	ret
;

; void io_wait(); (unused function)
io_wait:
	xor rax, rax
	out 0x80, al
	ret
;
