bits 64

; void setIDT(IDTLocationDescriptor* descriptor);
global setIDT
setIDT:
	push rbp
	mov rbp, rsp

	; load IDT ; address given by 'descriptor' arg
	; mov eax, [ebp + 8]
	; lidt [eax]
	lidt [rdi]

	mov rsp, rbp
	pop rbp
	ret
; END setIDT
