bits 32

; void x86_disable_interrupts();
global x86_disable_interrupts
x86_disable_interrupts:
	cli
	ret
; END x86_disable_interrupts

; void x86_enable_interrupts();
global x86_enable_interrupts
x86_enable_interrupts:
	sti
	ret
; END x86_enable_interrupts
