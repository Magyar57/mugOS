bits 32

; uint32_t x86_get_ip();
global x86_get_ip
x86_get_ip:
	; Since we cannot access the eip register,
	; we return the return address (stored on top of the stack)
	mov eax, [esp]
	ret
; END x86_get_ip
