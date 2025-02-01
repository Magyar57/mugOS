section .text

; void x86_outb(uint16_t port, uint8_t value);
global outb
outb:
	mov dx, [rsp + 4]
	mov al, [rsp + 8]
	out dx, al
	ret
; END outb

; uint8_t inb(uint16_t port);
global inb
inb:
	mov dx, [esp + 4]
	xor rax, rax
	in al, dx
	ret
; END inb

; void io_wait();
global io_wait
io_wait:
	push 0
	push 0x80	; 0x80 is an unused port
	call outb
	add rsp, 8
	ret
; END io_wait
