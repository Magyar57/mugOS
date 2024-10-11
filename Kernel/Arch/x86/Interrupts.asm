bits 32

; void x86_DisableInterrupts();
global x86_DisableInterrupts
x86_DisableInterrupts:
	cli
	ret
; END x86_DisableInterrupts

; void x86_EnableInterrupts();
global x86_EnableInterrupts
x86_EnableInterrupts:
	sti
	ret
; END x86_EnableInterrupts
