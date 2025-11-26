section .note.GNU-stack noalloc noexec nowrite progbits

section .text

global setIDT

; void setIDT(struct IDTLocationDescriptor* descriptor);
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
;
