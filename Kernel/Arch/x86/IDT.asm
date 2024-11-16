bits 32

; void setIDT(IDT_LocationDescriptor_32* descriptor);
global setIDT
setIDT:
	push ebp
	mov ebp, esp

	; load IDT ; address given by 'descriptor' arg
	mov eax, [ebp + 8]
	lidt [eax]

	mov esp, ebp
	pop ebp
	ret
; END setIDT
