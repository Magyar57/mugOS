; ===================================================================================
;							mugOS bootloader - First Stage
; ===================================================================================
;
; This file contains the First Stage bootloader code of the mugOS operating system.
; It is to be written on a floppy disc image (on the first sector of it)
;
; 	=> It uses legacy booting.
; 	=> It is encoded in the FAT12 format.
;
; Its role is to setup the data from the disk and the BIOS, 
; then load and execute the Second Stage bootloader.
;
; ===================================================================================

; FAT12 header - BIOS Parameter Block (see https://wiki.osdev.org/FAT12#BPB_.28BIOS_Parameter_Block.29)
jmp short start
nop

bdb_oem:					db 'MSWIN4.1'			; OEM identifier, 8 bytes
bdb_bytes_per_sector:		dw 512					;
bdb_sector_per_cluster:		db 1					;
bdb_reserved_sectors:		dw 1					; Number of sectors used by the "reserved" FAT header
bdb_fat_count:				db 2					;
bdb_dir_entries_count:		dw 0e0h					;
bdb_total_sectors:			dw 2880					; 2880 * 512 = 1.44 Mo (floppy disk size)
bdb_media_descriptor_type:	db 0f0h					; F0: 3.5" floppy disk
bdb_sectors_per_fat:		dw 9					; 9 sectors per fat
bdb_sectors_per_track:		dw 18					; 18 sectors per track
bdb_heads:					dw 2					; 2 sided disk
bdb_hidden_sectors:			dd 0					; (ie the LBA of the beginning of the partition)
bdb_large_sector_count:		dd 0					;

; FAT12 header - Extended Boot Record (see https://wiki.osdev.org/FAT12#Extended_Boot_Record)
ebr_drive_number:			db 0					; 0x00 = floppy, 0x80 = hdd
ebr_windows_nt_flags:		db 0					; As we're not in WindowsNT, this is reserved
ebr_signature:				db 29h					;
ebr_volume_id:				db 13h, 69h, 42h ,57h	; Floppy's serial number, value doesn't matter
ebr_volume_label:			db 'mugOS btldr'		; Exactly 11 bytes, pad with spaces
ebr_system_id:				db 'FAT12   '			; Same on 8 bytes

; ======================= ;
; Bootloader code section
; ======================= ;

; nasm maccro for '\n' (line feed)
%define ENDL 0x0d, 0x0a

org 0x7c00 ; Tells the assembler where we EXPECT our code to be loaded in memory. The assembler uses this information to calculate label addresses
bits 16 ; Tells the assembler to emit 16-bit code (as the CPU starts in 16 bit mode)

; Entry point
start:
	jmp main

; Function lba_to_chs
; Converts an LBA address to a CHS address
; Parameters:
;	- ax: LBA address
; Returns:
;	- cx [bits 0-5]: sector number
;	- cx [bits 6-15]: cylinder
;	- dh: head
; 	=>	so CX = 		---CH--- ---CL---
;		where cylinder:	76543210 98      
; 		and sector:		           543210
lba_to_chs:
	; SPT means SectorPerTrack
	; sector 	= (LBA % SPT) + 1
	; head 		= (LBA / SPT) % heads
	; cylinder 	= (LBA / SPT) / heads
	push ax
	push dx
	
	xor dx, dx							; dx = 0
	div word [bdb_sectors_per_track]	; ax = ax/SPT <=> ax = LBA/SPT    AND    dx = ax % SPT <=> dx = LBA % SPT
	inc dx								; dx++ so now dx = (LBA % SPT) + 1 = sector
	mov cx, dx							; cx = sector
	xor dx, dx							;
	div word [bdb_heads] 				; ax = ax/heads <=> ax = (LBA/SPT) / heads = cylinder AND dx = (LBA/SPT) % heads = head
	mov dh, dl 							; dh = head
	mov ch, al							; ch = cylinder (lower 8 bits)
	shl ah, 6							;
	or cl, ah							; put upper 2 bits of cylinder in CL

	pop ax
	mov dl, al							; restore DL only
	pop ax

	ret
; END lba_to_chs

; Function disk_read
; Reads sectors from disk
; Parameters:
;	- ax: LBA address
;	- cl: number n of sectors to read (up to 128)
;	- dl: drive number
;	- es:bx: memory address where to store read data
disk_read:
	
	; save registers that we use
	push ax
	push bx
	push cx
	push dx
	push di
	
	push cx								; temporarily save cl (number of sectors to read)
	call lba_to_chs						; compute CHS
	pop ax								; ax (al) = ancien cx (cl) = n

	; Read at least 3 times, as recommended by the documentation: https://stanislavs.org/helppc/int_13-2.html
	mov ah, 02h							; INT 13,2 : read disk sectors
	mov di, 3
	.retry:
		pusha							; save all registers, we don't know what bios modifies
		stc								; set carry flag, some BIOS'es don't set it
		int 13h							; call INT 13,2 : read disk sectors ; carry flag cleared => success
		jnc .done						; jump if carry flag not set
		
		; failed
		popa
		call disk_reset

		dec di
		test di, di
		jnz .retry

	.fail:
	; after all attempts are exhausted
	jmp floppy_error

	.done:
	popa

	; restore modified registers
	pop di
	pop dx
	pop cx
	pop bx
	pop ax
	ret
; END disk_read

; Function disk_reset
; Resets disk controller
; Parameters:
;	dl: drive number
disk_reset:
	pusha
	mov ah, 0
	stc
	int 13h
	jc floppy_error
	popa
	ret
; END disk_reset

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

; Function wait_key_and_reboot
wait_key_and_reboot:
	mov ah, 0							; wait for key pressed
	int 16h
	jmp 0ffffh:0						; jump to beginning of BIOS, should reboot

	cli									; mask interrupts, so we cannot be awaken
	hlt									; stop (forever)
; END wait_key_and_reboot

; Function floppy_error
; Prints a floppy error message, then reboots
floppy_error:
	mov si, read_error_message
	call puts
	jmp wait_key_and_reboot
; END floppy_error

; Function second_stage_not_found_error
; Prints a "stage not found" error message, then reboots
second_stage_not_found_error:
	mov si, msg_second_stage_not_found
	call puts
	jmp wait_key_and_reboot
; END second_stage_not_found_error

main:
	; setup data segments registers
	mov ax, 0
	mov ds, ax
	mov es, ax
	
	; setup stack registers
	mov ss, ax
	mov sp, 0x7c00

	; some BIOSes might start us at 07c0:000 instead of 0000:07c0
	; so we make sure to be at the expected location
	push es
	push word .after ; push return adress to the stack
	retf
	.after:

	; BIOS should set DL to drive number, so we store it
	mov [ebr_drive_number], dl

	; Show boot message
	mov si, init_msg
	call puts

	; read drive parameters (sectors per track and head count)
	; these parameters are already written on disk, but we can load them using a routine call from the BIOS as well
	; instead of relying on the data on the formatted disk
	push es
	mov ah, 08h
	int 13h
	jc floppy_error
	pop es
	; store retrived data
	and cl, 0x3f ; remove top 2 bits
	xor ch, ch
	mov [bdb_sectors_per_track], cx ; store sector count
	inc dh ; before increment, dh = logical last index of heads = number_of_heads - 1 because head indexes starts at 0
	mov [bdb_heads], dh ; store head count

	; read FAT root directory from disk
	; compute LBA of root directory = reserved + fats*sectors_per_fat
	; note: this section can be hardcoded
	mov ax, [bdb_sectors_per_fat]
	mov bl, [bdb_fat_count]
	xor bh, bh
	mul bx ; ax = (fats * sectors_per_fat)
	add ax, [bdb_reserved_sectors] ; ax += reserved sectors => so ax = LBA of root directory
	push ax ; save LBA onto the stack
	; compute size of root directory = (32*number_of_entries) / bytes_per_sector
	mov ax, [bdb_dir_entries_count]
	shl ax, 5 ; shift left ax 5 times ; ax *= 2^5 ; ax *= 32
	xor dx, dx
	div word [bdb_bytes_per_sector] ; number of sectors we need to read
	; compute size of root directory - Adjust sector number
	test dx, dx ; if dx != 0, add 1 (this means we have a sector only partially filled with code/data)
	jz .root_dir_after
	inc ax
	.root_dir_after:
	; now we can finally read the root directory
	mov cl, al ; cl = number of sectors to read = size of root directory
	pop ax ; ax = LBA (we retrive it from the stack, we saved it earlier)
	mov dl, [ebr_drive_number] ; dl = drive number
	mov bx, buffer ; es:bx = buffer address
	call disk_read

	; search for 2NDSTAGE.bin file
	xor bx, bx ; bx will be our i of the for loop
	mov di, buffer ; dx will be our current_directory_entry pointer. 
	; As the name is the first entry in the FAT directories structure, it also points to the file/folder name

	.search_second_stage:
		mov si, second_stage_filename
		mov cx, 11 ; compare up to 11 characters
		push di
		; cmpsb: compares the two (string) bytes in memory at addresses ds:si and es:di and increment/decrement si & di
		; repe: repeats a string instruction while the operands are equal (zero flag = 1), or until xc==0 ; cx is decremented on each iteration
		repe cmpsb
		pop di
		je .found_second_stage ; if the strings are equal, we found the file
		; else, we move to the next directory entry, until there are none remaining
		
		add di, 32 ; we add the size of a directory entry to the current_directory_entry pointer
		inc bx ; i++

		cmp bx, [bdb_dir_entries_count] ; if i<nb_root_dir_entries (we still have entries to check)
		jl .search_second_stage ; jump to search_second_stage
		
		; otherwise, second stage not found
		jmp second_stage_not_found_error

	.found_second_stage:
	; di should have the address to the entry
	mov ax, [di + 26] ; first logical cluster field (offset 26 in the directory entry)
	mov [second_stage_cluster], ax ; save the second stage cluster to memory

	; loat FAT from disk into memory (at buffer's address)
	mov ax, [bdb_reserved_sectors]
	mov cl, [bdb_sectors_per_fat]
	mov dl, [ebr_drive_number]
	mov bx, buffer
	call disk_read

	; read second stage file and process FAT chain
	mov bx, SECOND_STAGE_LOAD_SEGMENT
	mov es, bx
	mov bx, SECOND_STAGE_LOAD_OFFSET
	.load_second_stage_loop:
		; Read next cluster
		mov ax, [second_stage_cluster]

		; first cluster = (second_stage_cluster - 2)*sectors_per_cluster + start_sector
		; start sector = reserved + fats + root_directory_sioze = 1 + 18 + 134 = 33
		; WARNING TODO fix: this is a hardcoded value that is ok for floppy but wrong for other types of disks. This will have to be fixed later
		add ax, 31

		mov cl, 1
		mov dl, [ebr_drive_number]
		call disk_read ; TODO plante

		add bx, [bdb_bytes_per_sector] ; WARNING TODO ANOTHER ERROR: this add will overflow if the 2ndStage.bin file we're reading is larger than 64 Ko

		; compute location of next cluster
		mov ax, [second_stage_cluster]
		mov cx, 3
		mul cx
		mov cx, 2
		div cx ; ax = index of entry in FAT, dx = cluster % 2

		mov si, buffer
		add si, ax
		mov ax, [ds:si] ; read entry from FAT table at index ax

		or dx, dx
		jz .even

		.odd:
		shr ax, 4
		jmp .next_cluster_after

		.even:
		and ax, 0x0fff

		.next_cluster_after:
		cmp ax, 0x0ff8 ; 0x0ff8 or above: end of FAT12 sectors chain
		jae .second_stage_loaded

		mov [second_stage_cluster], ax
		jmp .load_second_stage_loop

	.second_stage_loaded:
	
	; jump the second stage of the bootloader
	mov dl, [ebr_drive_number] ; boot device in dl
	mov ax, SECOND_STAGE_LOAD_SEGMENT ; set segment registers
	mov ds, ax
	mov es, ax
	jmp SECOND_STAGE_LOAD_SEGMENT:SECOND_STAGE_LOAD_OFFSET

	; We should not reach this bit of code
	jmp wait_key_and_reboot
	
	; End of bootloader: infinite loop
	cli	; mask interrupts, so we cannot be awaken
	hlt ; stop (forever)
; END main

; =================== ;
; Static data section
; =================== ;

init_msg: 					db 'Bootloader...', ENDL, 0
read_error_message:			db 'Disk read failed !', ENDL, 0
second_stage_filename:		db '2NDSTAGEBIN'
msg_second_stage_not_found:	db '2nd-stage file not found !', ENDL, 0
second_stage_cluster:		dw 0

; Memory addresses where to load the second stage's code
; Question is: where do we put the files in memory ?
; We are in 16 bits real mode, so we can't access memory above the 1Mo limit
; so we can look at memory map and pick a location from there, 
; without worrying about overriding something: 
; https://wiki.osdev.org/Memory_Map_(x86)#Overview
; We chose 0x00007E00 to 0x0007FFFF (480.5 KiB) - Conventional memory
; Note: "equ" directive: no memory will be allocated for the constants
SECOND_STAGE_LOAD_SEGMENT		equ 0x2000
SECOND_STAGE_LOAD_OFFSET		equ 0

; 0xaa55 signature
; We're writing to a floppy disc, where 1 sector is 512B
; so we declare constant bytes (db directive) until we reach the last bytes of the sector (times instruction)
; in nasm, $ is the memory offset of the current line, and $$ is the memory offset of the beginning of the current section
times 510-($-$$) db 0
dw 0aa55h ; hexa 0xaa55

; TODO comment, ou changer le nom de l'étiquette
; On ne peut pas écrire de code après le précédent "dw", car il s'agit de la fin du secteur du bootloader
; Ce secteur de bootloader est chargé en RAM, mais pas ce qui le suit sur le floppy (FAT puis root directory)
; En revanche on peut y placer des étiquettes, qui correspondront à de la mémoire de code non écrite
; On peut donc y charger ce que l'on veut (temporairement, un root directory du disque FAT par exemple)
buffer:
