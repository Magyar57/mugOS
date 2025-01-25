bits 64

; void setGDT(GDT_LocationDescriptor_64* descriptor, uint16_t kcodeSegment, uint16_t kdataSegment);
; in System V ABI,   i386 (32 bits): descriptor in ebp+8 ; kcodeSegment in ebp+12 ; kdataSegment in ebp+16
; in System V ABI, x86_64 (64 bits): descriptor in rdi   ; kcodeSegment in rsi    ; kdataSegment in rdx
global setGDT
setGDT:
	push rbp
	mov rbp, rsp

	; load gdt ; address given by 'descriptor' arg
	; mov eax, [ebp+8]
	; lgdt [eax]
	lgdt [rdi]

	; setup segments registers
	; mov rax, [rbp+12]	; ax = kcodeSegment
	; push rax
	push rsi
	push .reload_cs	; cs cannot be mv-ed directly, we have to jump
	retf				; would be equivalent to jmp eax : .reload_cs
	.reload_cs:
	mov rax, rdx	; ax = kdataSegment
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	mov rsp, rbp
	pop rbp
	ret
; END setGDT
