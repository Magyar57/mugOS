; =====================================================================================
;								Kernel main entry point
; =====================================================================================
; This code is loaded by the bootloader, and jumped to after the inialization.
; TODO describe what it does
; =====================================================================================

%define ENDL 0x0d, 0x0a

org 0x0000 ; Tells the assembler where we EXPECT our code to be loaded in memory. The assembler uses this information to calculate label addresses
bits 16 ; Tells the assembler to emit 16-bit code (as the CPU starts in 16 bit mode)

; Entry point
start:
	jmp main

; Function puts
; Prints a string to the screen
; Params:
; - ds:si points to the string to print. The string has to be terminated with the null char
puts:
	push si
	push ax
	push bx
	.loop:
		lodsb 			; loads next char in al
		or al, al 		; verify if next char is null ?
		jz .done 		; => break if it is
		
		; call BIOS function
		mov ah, 0x0e	; set page number to 0
		mov bh, 0		; write char in TTY mode
		int 0x10 		; video BIOS interrupt

		jmp .loop
	.done:
	pop bx
	pop ax
	pop si
	ret
; END puts

main:
	; Print kernel loading message
	mov si, init_msg
	call puts

	.halt:
	cli
	hlt
; END main

; =================== ;
; Static data section
; =================== ;

init_msg: db 'Loading mugOS kernel...', ENDL, 'Supposons que je sois dans votre kernel', ENDL, 0
