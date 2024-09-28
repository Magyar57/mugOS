bits 32

; void x86_setIDT(IDT_LocationDescriptor_32* descriptor);
global x86_setIDT
x86_setIDT:
	push ebp
	mov ebp, esp

	; load IDT ; address given by 'descriptor' arg
	mov eax, [ebp + 8]
	lidt [eax]

	mov esp, ebp
	pop ebp
	ret
; END x86_setIDT
