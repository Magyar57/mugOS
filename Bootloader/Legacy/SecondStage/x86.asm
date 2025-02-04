bits 16

%include "x86_SwitchMode.asm"

; linearToSegmentOffset: Convert linear address to segment:offset address
; Args:
;	1 - linear address
;	2 - (out) target segment (e.g. es)
;	3 - target 32-bit register to use (e.g. eax)
;	4 - target lower 16-bit half of #3 (e.g. ax)
%macro linearToSegmentOffset 4
	mov %3, %1      ; linear address to eax
	shr %3, 4
	mov %2, %4
	mov %3, %1      ; linear address to eax
	and %3, 0xf
%endmacro

; void x86_outb(uint16_t port, uint8_t value);
global x86_outb
x86_outb:
	[bits 32]
	mov dx, [esp + 4]
	mov al, [esp + 8]
	out dx, al
	ret
; END x86_outb

; uint8_t x86_inb(uint16_t port);
global x86_inb
x86_inb:
	[bits 32]
	mov dx, [esp + 4]
	xor eax, eax
	in al, dx
	ret
; END x86_inb

; void x86_Disk_Reset(uint8_t drive);
global x86_Disk_Reset
x86_Disk_Reset:
	[bits 32]

	; make new call frame
	push ebp
	mov ebp, esp
	x86_EnterRealMode
	[bits 16]

	; call bios interrupt 13, ah=00h => disk reset call
	mov ah, 00h
	mov dl, [bp + 8]
	stc ; stc means Set Carry flag - it sets the cf flag to 1. The BIOS set the carry flag to 0 for success
	int 13h

	; return:
	; 0 (false) if cf is 1
	; 1 (true) if cf is 0
	mov eax, 1
	sbb eax, 0 ; ax -= 0 + cf (so ax -= cf)

	push eax
	x86_EnterProtectedMode
	pop eax

	; restore old call frame
	leave
	ret
; END x86_Disk_Reset

; void x86_Disk_Read(uint8_t drive, uint16_t cylinder, uint16_t sector, uint16_t head, uint8_t count, uint8_t far* dataOut);
global x86_Disk_Read
x86_Disk_Read:
	[bits 32]
	push ebp
	mov ebp, esp

	x86_EnterRealMode
	[bits 16]

	; save modified registers
	push ebx
	push es

	; setup interrupt arguments
	mov dl, [bp+8]		; dl - drive

	mov ch, [bp+12]		; ch - cylinder (lower 8 bits)
	mov cl, [bp+13]		; cl - cylinder to bits 6-7
	shl cl, 6			; shift left to keep bits 6-7

	mov al, [bp+16]		; we use al temporarily
	and al, 3Fh			; mask sector number with 00111111 (keep lower 6 bits)
	or cl, al			; move result to cl, and keep upper 2 bits previoulsy set. Now cl - sector to bits 0-5

	mov dh, [bp+20]		; dh - head

	mov al, [bp+24]		; al - number of sectors to read

	linearToSegmentOffset [bp+28], es, ebx, bx
	
	; call int 13h
	mov ah, 02h
	stc
	int 13h

	; return (same than previously)
	mov ax, 1
	sbb ax, 0

	; restore modified registers
	pop es
	pop ebx

	push eax
	x86_EnterProtectedMode
	pop eax

	leave
	ret
; END x86_Disk_Read

; void x86_Disk_GetDriveParameters(uint8_t drive, uint8_t* driveTypeOut, uint16_t* cylindersOut, uint16_t* sectorsOut, uint16_t* headsOut);
global x86_Disk_GetDriveParameters
x86_Disk_GetDriveParameters:
	[bits 32]

	; Enter: make new call frame
	push ebp
	mov ebp, esp

	x86_EnterRealMode
	[bits 16]

	; save registers
	push es
	push bx
	push esi
	push di

	; call bios int 13h (get drive parameters)
	mov dl, [bp+8]
	mov ah, 08h
	mov di, 0			; es:di - 0000:0000
	mov es, di			; es:di - 0000:0000
	stc
	int 13h

	; retrive informations given by the BIOS

	; return value
	mov eax, 1
	sbb eax, 0

	; drive type from bl

	linearToSegmentOffset [bp+12], es, esi, si
	mov es:[si], bl
	
	; cylinders
	mov bl, ch			; cylinders lower bits in ch
	mov bh, cl			; cylinders upper bits in cl (6-7)
	shr bh, 6			; move the bits at the right place
	inc bx

	linearToSegmentOffset [bp+16], es, esi, si
    mov es:[si], bx

	; sectors
	xor ch, ch			; sectors - lower 5 bits in cl
	and cl, 0x3f

	linearToSegmentOffset [bp+20], es, esi, si
	mov es:[si], cx

	; heads
	mov cl, dh			; heads - dh
	inc cx

	linearToSegmentOffset [bp+24], es, esi, si
	mov es:[si], cx

	; restore registers
	pop di
	pop esi
	pop bx
	pop es

	push eax
	x86_EnterProtectedMode
	[bits 32]
	pop eax

	leave
	ret
; END x86_Disk_GetDriveParameters
