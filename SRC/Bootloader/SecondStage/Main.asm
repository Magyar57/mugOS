; ===================================================================================
;							mugOS bootloader - Second Stage
; ===================================================================================
;
; This file contains the Second Stage bootloader code of the mugOS operating system.
; It is to be written on a floppy disc image (on the first sector of it)
;
; Its role is to [TODO describe]
; then load and execute the mugOS kernel.
;
; ===================================================================================

bits 16

section _ENTRY class=CODE

extern _cstart_
global entry

entry:
	
	; Initalization
	cli
	; We are using the small memory model: the stack and data segment are the same
	; The data segment has already been setup by the first stage, so we simply copy it to the stack segment register
	mov ax, ds
	mov ss, ax
	; Reset the base and stack pointer to 0
	mov sp, 0
	mov bp, sp
	sti

	; expect boot drive in dl, send it as argument to the C main function (by pushing it to the stack)
	xor dh, dh
	push dx

	; Launch the C main function
	call _cstart_

	; If we ever return from it, simply halt the system
	cli
	hlt
