; ===================================================================================
;							mugOS bootloader - Second Stage
; ===================================================================================
;
; This file contains the Second Stage bootloader code of the mugOS operating system.
; It is to be written on a floppy disc image (on the first sector of it)
;
; Its role is to switch to protected mode (32 bits)
; then load and execute the mugOS kernel.
;
; ===================================================================================

bits 16

section .entry		; gcc directive

extern __bss_start	; defined in the linker map
extern __end		; defined in the linker map
extern start		; defined in main.c
global entry

%include "x86_SwitchMode.asm"

; Since we load the second stage at some address, and immediately jump at this address,
; if we don't want our main to be here we should jump to it right at the beginning
jmp entry

; Function disable_NMI: Disables the Non-Maskable interrupts
; >>> interrupts must be disabled (call cli before disable_NMI) !! <<<
disable_NMI:
	[bits 16]
	push ax
	in al, 0x70
	or al, 0x80		; al = al | 0b1000000 (set first bit to 1)
	out 0x70, al
	; The CMOS RTC expects a read from or write to the data port 0x71 after any 
	; write to index port 0x70 or it may go into an undefined state
	in al, 0x71
	pop ax
	ret
; END disable_NMI

; Function enable_NMI: Enables the Non-Maskable interrupts
; >>> interrupts must be disabled (call cli before disable_NMI) !! <<<
enable_NMI:
	[bits 32]
	push eax
	in al, 0x70
	and al, 0x7f		; al = al & 0b01111111 (set first bit to 0)
	out 070h, al
	; The CMOS RTC expects a read from or write to the data port 0x71 after any 
	; write to index port 0x70 or it may go into an undefined state
	in al, 0x71
	pop eax
	ret
; END enable_NMI

; Function enable_A20: enables the A20 gate (disable memory loop after bit 20)
enable_A20:
	[bits 16]

	; Disable the keyboard
	call A20_wait_input
	mov al, KeyboardControllerDisableKeyboard
	out KeyboardControllerCommandPort, al
	
	; Read the value of the Controller Output Port
	call A20_wait_input
	mov al, KeyboardControllerReadCtrlOutputPort ; ask for the value
	out KeyboardControllerCommandPort, al
	call A20_wait_output
	in al, KeyboardControllerDataPort ; read the value
	push eax ; save it
	
	; Write to the Controller Output Port, with the A20 gate disabled
	call A20_wait_input
	mov al, KeyboardControllerWriteCtrlOutputPort
	out KeyboardControllerCommandPort, al

	call A20_wait_input
	pop eax ; retrive the previously saved value
	or al, 0b00000010 ; set bit number 1 (index starts at 0), which is the a20 gate bit, to 1
	out KeyboardControllerDataPort, al

	; Re-enable the keyboard
	call A20_wait_input
	mov al, KeyboardControllerEnableKeyboard
	out KeyboardControllerCommandPort, al

	call A20_wait_input
	ret
; END enable_A20

A20_wait_input:
	[bits 16]
	; Wait until status bit 2 (input buffer) is set to 0
	; By reading from the keyboard's command port, we read the status byte
	in al, KeyboardControllerCommandPort
	test al, 2
	jnz A20_wait_input
	ret
; END A20_wait_input

A20_wait_output:
	[bits 16]
	; Wait until status bit 1 (output buffer) is set to 1
	; By reading from the keyboard's command port, we read the status byte
	in al, KeyboardControllerCommandPort
	test al, 1
	jz A20_wait_output
	ret
; END A20_wait_output

; Loads the GDT (https://www.felixcloutier.com/x86/lgdt:lidt)
load_GDT:
	[bits 16]
	lgdt [g_GDT_descriptor]
	ret
; END load_GDT

entry:
	[bits 16]
	cli

    ; save boot drive using a global variable
    mov [g_bootDrive], dl

	; Setup the stack
	; We are using the small memory model: the stack and data segment are the same
	; The data segment has already been setup by the first stage, so we simply copy it to the stack segment register
	mov ax, ds
	mov ss, ax
	mov sp, 0xfff0
	mov bp, sp

	; Switch to protected mode (see https://wiki.osdev.org/Protected_Mode)
	cli						; disable interrupts
	call disable_NMI		; disble NMI (non-maskable interrupts)
	call enable_A20			; Enable the A20 Line.
	call load_GDT			; load the Global Descriptor Table
	x86_EnterProtectedMode	; enter protected mode !
	[bits 32]

	; Clear bss (uninitialized data)
	mov edi, __bss_start
	mov ecx, __end
	sub ecx, edi
	mov al, 0
	cld
	rep stosb ; stosb: store string byte

	; Re-nable interrupts
	call enable_NMI
	; sti ; => we need more preparation before enabling interrupts in pmode

	; The signature of our C entry point is 'cstart_(uint16_t bootDrive)'
	; => We push the bootDrive argument on the stack
	xor edx, edx
	mov dl, [g_bootDrive]
	push edx
	; And call the C main
	call start

	; If we ever return from it, simply halt the system
	cli
	hlt
; END entry

; Interrupts codes & controllers values
KeyboardControllerDataPort				equ 0x60
KeyboardControllerCommandPort			equ 0x64 ; reading this port returns the status register, writing writes a command
KeyboardControllerDisableKeyboard		equ 0xad
KeyboardControllerEnableKeyboard		equ 0xae
KeyboardControllerReadCtrlOutputPort	equ 0xd0
KeyboardControllerWriteCtrlOutputPort	equ 0xd1

; Global variables
g_bootDrive: db 0

; Global Descriptor Table (describes memory segments)
g_GDT:
	; Descriptor: NULL
	dq 0
	; Descriptor: 32-bit code segment, flat memory model
	dw 0xffff				; limit (bits  0-15) = 0xffffffff for full 32-bit range
	dw 0					; base  (bits  0-15) = 0x00000000
	db 0					; base  (bits 16-23) = 0x00000000
	db 0b10011010			; flags: access, present, ring 0, code segment, executable, direction 0, readable
	db 0b11001111			; flags: granularity 4KB pages, 32-bits pmode + limit (bits 16-19) = 0xffffffff for full 32-bit range
	db 0					; base  (bits 24-32) = 0x00000000
	; Descriptor: 32-bit data segment, flat memory model
	dw 0xffff				; limit (bits  0-15) = 0xffffffff for full 32-bit range
	dw 0					; base  (bits  0-15) = 0x00000000
	db 0					; base  (bits 16-23) = 0x00000000
	db 0b10010010			; flags: access, present, ring 0, data segment, executable, direction 0, writable
	db 0b11001111			; flags: granularity 4KB pages, 32-bits pmode + limit (bits 16-19) = 0xffffffff for full 32-bit range
	db 0					; base  (bits 24-32) = 0x00000000
	; Descriptor: 16-bit code segment, flat memory model
	dw 0xffff				; limit (bits  0-15) = 0xffffffff
	dw 0					; base  (bits  0-15) = 0x00000000
	db 0					; base  (bits 16-23) = 0x00000000
	db 0b10011010			; flags: access, present, ring 0, code segment, executable, direction 0, readable
	db 0b00001111			; flags: granularity 1B pages, 16-bits pmode + limit (bits 16-19) = 0xffffffff
	db 0					; base  (bits 24-32) = 0x00000000
	; Descriptor: 16-bit data segment, flat memory model
	dw 0xffff				; limit (bits  0-15) = 0xffffffff
	dw 0					; base  (bits  0-15) = 0x00000000
	db 0					; base  (bits 16-23) = 0x00000000
	db 0b10010010			; flags: access, present, ring 0, data segment, executable, direction 0, writable
	db 0b00001111			; flags: granularity 1B pages, 16-bits pmode + limit (bits 16-19) = 0xffffffff
	db 0					; base  (bits 24-32) = 0x00000000

; Descriptor for the GDT
g_GDT_descriptor:
	dw g_GDT_descriptor - g_GDT - 1 ; Size of the GDT
	dd g_GDT ; Location (address) of the GDT
