bits 32

; void x86_outb(uint16_t port, uint8_t value);
global outb
outb:
	mov dx, [esp + 4]
	mov al, [esp + 8]
	out dx, al
	ret
; END outb

; uint8_t inb(uint16_t port);
global inb
inb:
	mov dx, [esp + 4]
	xor eax, eax
	in al, dx
	ret
; END inb

; void io_wait();
global io_wait
io_wait:
	push 0
	push 0x80	; 0x80 is an unused port
	call outb
	add esp, 8
	ret
; END io_wait
