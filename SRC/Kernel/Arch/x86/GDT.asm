bits 32

; void x86_setGDT(GDT_LocationDescriptor_32* descriptor, uint16_t kcodeSegment, uint16_t kdataSegment);
global x86_setGDT
x86_setGDT:
	push ebp
	mov ebp, esp

	; load gdt ; address given by 'descriptor' arg
	mov eax, [ebp + 8]
	lgdt [eax]

	; setup segments registers
	mov eax, [ebp+12]	; ax = kcodeSegment
	push eax
	push .reload_cs	; cs cannot be mv-ed directly, we have to jump
	retf				; would be equivalent to jmp eax : .reload_cs
	.reload_cs:
	mov ax, [ebp+16]	; ax = kdataSegment
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	mov esp, ebp
	pop ebp
	ret
; END x86_setGDT
