bits 16

; x86_EnterProtectedMode
%macro x86_EnterProtectedMode 0
    [bits 16]
	cli

	; Set the protected mode bit in cr0
	mov eax, cr0
	or al, 0b0001
	mov cr0, eax

	; Perform far jump to selector 08h (offset into GDT, pointing at a 32bit PM code segment descriptor) 
	; to load CS with proper PM32 descriptor)
	jmp dword 0x08:.pmode
	.pmode:
	; Here we are in protected mode ! :)
	[bits 32]

	; Setup segment registers: DS, ES, FS, GS, SS, ESP
	mov ax, 0x10 ; Offset (in the GDT). We're using the 32-bit pmode data segment (3rd entry), its offset is 0x10 = 16
	mov ds, ax
	; mov ds, ax
	; mov fs, ax
	; mov gs, ax
	mov ss, ax
%endmacro

; x86_EnterRealMode
%macro x86_EnterRealMode 0
    [bits 32]
	; 1 - jump to 16-bit protected mode segment
    jmp word 18h:.pmode16

	.pmode16:
    [bits 16]
    ; 2 - disable protected mode bit in cr0
    mov eax, cr0
    and al, ~1
    mov cr0, eax

    ; 3 - jump to real mode
    jmp word 00h:.rmode

	.rmode:
    ; 4 - setup segments
    mov ax, 0
    mov ds, ax
    mov ss, ax

    ; 5 - enable interrupts
    sti
%endmacro
