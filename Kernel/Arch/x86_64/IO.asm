section .note.GNU-stack noalloc noexec nowrite progbits

section .text

global outb
global outw
global outl
global inb
global inw
global inl
global io_wait

; void outb(uint16_t port, uint8_t value);
outb:
	mov rdx, rdi ; dx = port
	mov rax, rsi ; al = value

	out dx, al
	ret
;

; void outw(uint16_t port, uint16_t value);
outw:
	mov rdx, rdi
	mov rax, rsi

	out dx, ax
	ret

; void outl(uint16_t port, uint32_t value);
outl:
	mov rdx, rdi
	mov rax, rsi

	out dx, eax
	ret

; uint8_t inb(uint16_t port);
inb:
	mov rdx, rdi ; dx = port

	xor rax, rax
	in al, dx
	ret
;

; uint16_t inw(uint16_t port);
inw:
	mov rdx, rdi

	xor rax, rax
	in ax, dx
	ret

; uint32_t inl(uint16_t port);
inl:
	mov rdx, rdi

	xor rax, rax
	in eax, dx
	ret

; void io_wait();
io_wait:
	xor rax, rax
	out 0x80, al
	ret
;
