bits 16

section _TEXT class=CODE

;
; U4D
;
; Operation:      Unsigned 4 byte division
; Inputs:         DX;AX   Dividend
;                 CX;BX   Divisor
; Outputs:        DX;AX   Quotient
;                 CX;BX   Remainder
; Volatile:       none
;
global __U4D
__U4D:
    shl edx, 16         ; dx to upper half of edx
    mov dx, ax          ; edx - dividend
    mov eax, edx        ; eax - dividend
    xor edx, edx

    shl ecx, 16         ; cx to upper half of ecx
    mov cx, bx          ; ecx - divisor

    div ecx             ; eax - quot, edx - remainder
    mov ebx, edx
    mov ecx, edx
    shr ecx, 16

    mov edx, eax
    shr edx, 16

    ret

;
; U4M
; Operation:      integer four byte multiply
; Inputs:         DX;AX   integer M1
;                 CX;BX   integer M2
; Outputs:        DX;AX   product
; Volatile:       CX, BX destroyed
;
global __U4M
__U4M:
    shl edx, 16         ; dx to upper half of edx
    mov dx, ax          ; m1 in edx
    mov eax, edx        ; m1 in eax

    shl ecx, 16         ; cx to upper half of ecx
    mov cx, bx          ; m2 in ecx

    mul ecx             ; result in edx:eax (we only need eax)
    mov edx, eax        ; move upper half to dx
    shr edx, 16

    ret

; void _cdecl x86_div64_32(uint64_t dividend, uint32_t divisor, uint64_t* quotientOut, uint32_t* remainderOut);
global _x86_div64_32
_x86_div64_32:
	; make new call frame
	push bp
	mov bp, sp
	; save registers that we'll be using
	push bx

	; divide upper 32 bits
	mov eax, [bp + 8] ; eax <- upper 32 bits of dividend
	mov ecx, [bp + 12] ; exc <- divisor
	xor edx, edx ; edx = 0
	div ecx	; eax - quot, edx - remainder

	; store upper 32 bits of quotient
	mov bx, [bp + 16]
	mov [bx + 4], eax

	; divide lower 32 bits
	mov eax, [bp + 4] ; eax <- lower 32 bits of dividend
	; edx elready contains the old remainder, nothing to assign
	div ecx

	; store results
	mov [bx], eax
	mov bx, [bp + 18]
	mov [bx], edx

	pop bx ; restore used register
	; restore call frame
	mov sp, bp
	pop bp
	ret

;
; int 10h ah=0eh
; args: character, page
;
global _x86_Video_WriteCharTeletype
_x86_Video_WriteCharTeletype:
	; make new call frame
	push bp			; save old call frame
	mov bp, sp		; init new call frame

	; save bx (we're using it later in the function)
	push bx

	; [bp + 0] - old call frame
	; [bp + 2] - return address (small memory model => 2 bytes)
	; [bp + 4] - first function argument (a char) ; bytes are converted to words, you can't push a byte on the stack. We're in 16 bits mode, a word is 2 bytes
	; [bp + 6] - second arg (page)
	
	; 0eh interruption : print a character to the screen
	mov ah, 0eh
	mov al, [bp + 4]
	mov bh, [bp + 6]
	int 10h

	; restore used registers
	pop bx

	; restore old call frame
	mov sp, bp
	pop bp

	ret

; void _cdecl x86_Disk_Reset(uint8_t drive);
global _x86_Disk_Reset
_x86_Disk_Reset:
	push bp
	mov bp, sp

	; call bios interrupt 13, ah=00h => disk reset call
	mov ah, 00h
	mov dl, [bp + 4]
	stc ; stc means Set Carry flag - it sets the cf flag to 1. The BIOS set the carry flag to 0 for success
	int 13h

	; return:
	; 0 (false) if cf is 1
	; 1 (true) if cf is 0
	mov ax, 1
	sbb ax, 0 ; ax -= 0 + cf (so ax -= cf)

	mov sp, bp
	pop bp
	ret

; void _cdecl x86_Disk_Read(uint8_t drive, uint16_t cylinder, uint16_t sector, uint16_t head, uint8_t count, uint8_t far* dataOut);
global _x86_Disk_Read
_x86_Disk_Read:
	push bp
	mov bp, sp

	; save modified registers
	push bx
	push es

	; setup interrupt arguments
	mov dl, [bp + 4]	; dl - drive

	mov ch, [bp + 6]	; ch - cylinder (lower 8 bits)
	mov cl, [bp + 7]	; cl - cylinder to bits 6-7
	shl cl, 6			; shift left to keep bits 6-7

	mov dh, [bp + 10]	; dh - head

	mov al, [bp + 8]	; we use al temporarily
	and al, 3Fh			; mask sector number with 00111111 (keep lower 6 bits)
	or cl, al			; move result to cl, and keep upper 2 bits previoulsy set. Now cl - sector to bits 0-5

	mov al, [bp + 12]	; al - number of sectors to read

	mov bx, [bp + 16]	; es:bx - far pointer to dataOut
	mov es, bx
	mov bx, [bp + 14]

	mov ah, 02h
	stc
	int 13h

	; return (same than previously)
	mov ax, 1
	sbb ax, 0

	; restore modified registersr
	pop es
	pop bx

	mov sp, bp
	pop bp
	ret

; void _cdecl x86_Disk_GetDriveParameters(uint8_t drive, uint8_t* driveTypeOut, uint16_t* cylindersOut, uint16_t* sectorsOut, uint16_t* headsOut);
global _x86_Disk_GetDriveParameters
_x86_Disk_GetDriveParameters:
	push bp
	mov bp, sp

	; save registers
	push es
	push bx
	push si
	push di

	mov dl, [bp + 4]
	mov ah, 08h
	mov di, 0			; es:di - 0000:0000
	mov es, di			; es:di - 0000:0000
	stc
	int 13h

	; retrive informations given by the BIOS

	; return value
	mov ax, 1
	sbb ax, 0

	; drive type from bl
	mov si, [bp + 6]	; si = driveTypeOut
	mov [si], bl		; *si = bl (drive type)

	mov bl, ch			; lower bits in ch
	mov bh, cl			; upper bits in cl (6-7)
	shr bh, 6			; move the bits at the right place
	mov si, [bp + 8]
	mov [si], bx

	xor ch, ch			; sectors - lower 5 bits in cl
	and cl, 3Fh
	mov si, [bp + 10]
	mov [si], cx

	mov cl, dh			; heads - dh
	mov si, [bp + 12]
	mov [si], cx

	; restore registers
	pop di
	pop si
	pop bx
	pop es

	mov sp, bp
	pop bp
	ret
