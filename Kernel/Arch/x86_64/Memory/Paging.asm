section .text

; void enablePaging(void* pageTable, uint16_t ktextSegment, uint16_t kdataSegment);
; 					rdi=pageTable,   rsi=ktextSegment,      rdx=kdataSegment
; NOTE: This function is present as an example on how to enable paging (and long mode), but
; it is not actually used, since Limine already does it for us.
global enablePaging
enablePaging:
	; Disable paging
	mov rbx, cr0
	and rbx, 0x7fffffff		; clear Paging bit (31st bit)
	mov cr0, rbx

	; These (PAE and Long mode) should be set by the bootloader already, but we do it nontheless
	; Enable PAE (physical address extension) in the cr4 register
	; PAE is mandatory for Long Mode
	mov rdx, cr4
	or rdx, (1 << 5)
	mov cr4, rdx
	; Enable long mode
	mov rcx, 0xc0000080		; 0xc0000080 => EFER register (model specific register)
	rdmsr 					; read from specific memory register
	or rax, 0x100			; set Long Mode bit (9th bit)
	wrmsr					; write from...

	; Set page table
	mov cr3, rdi			; rdi contains the page table pointer

	; Enable paging
	mov rbx, cr0
	mov rax, 0x80000001		; Protected mode (1st bit) and paging (31st bit)
	or rbx, rax
	mov cr0, rbx

	; Reload segment registers
	mov ax, dx				; rdx=kdataSegment
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	push rsi
	lea rax, [rel .reload_cs]
	push rax
	retfq
	.reload_cs:

	[bits 64]
	ret
; END enablePaging

; bool setPML4(physical_address_t pml4);
global setPML4
setPML4:
	; Check that the table is page aligned
	mov rax, rdi
	and rax, 0x0000000000000fff
	test rax, rax 			; <=> cmp rax, 0
	jne .err

	; Put in canonical form (PML4: 48 bits address)
	mov rax, 0x0000fffffffff000
	and rdi, rax
	; Set page table in cr3
	mov cr3, rdi

	; Return true
	mov rax, 1
	ret

	; Error: return false
	.err:
	xor rax, rax
	ret
; END setPML4

; bool setPML5(physical_address_t pml5);
global setPML5
setPML5:
	; Check that the table is page aligned
	mov rax, rdi
	and rax, 0x0000000000000fff
	test rax, rax 			; <=> cmp rax, 0
	jne .err

	; Put in canonical form (PML5: 52 bits address)
	mov rax, 0x00fffffffffff000
	and rdi, rax
	; Set page table in cr3
	mov cr3, rdi

	; Return true
	mov rax, 1
	ret

	; Error: return false
	.err:
	xor rax, rax
	ret
; END setPML5

; void flushTLB(void* addr);
global flushTLB
flushTLB:
	invlpg [rdi]
	ret
; END flushTLB
