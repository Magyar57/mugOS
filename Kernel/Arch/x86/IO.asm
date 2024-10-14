bits 32

; void x86_outb(uint16_t port, uint8_t value);
global x86_outb
x86_outb:
	mov dx, [esp + 4]
	mov al, [esp + 8]
	out dx, al
	ret
; END x86_outb

; uint8_t x86_inb(uint16_t port);
global x86_inb
x86_inb:
	mov dx, [esp + 4]
	xor eax, eax
	in al, dx
	ret
; END x86_inb

; void x86_io_wait();
global x86_io_wait
x86_io_wait:
	push 0
	push 0x80	; 0x80 is an unused port
	call x86_outb
	add esp, 8
	ret
