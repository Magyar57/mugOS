section .note.GNU-stack noalloc noexec nowrite progbits

section .text

global usermode_function
global x86_jumpToUsermode

; getSegmentSelector(int selector, int ti, int rpl)
; https://wiki.osdev.org/Segment_Selector
; selector = (segment selector & 0xfff8) | (ti & 0x0004) | (rpl & 0x0003)
%define getSegmentSelector(selector, ti, cpl) \
	((selector & 0xfff8) | ((ti << 2) & 0x0004) | (cpl & 0x0003))

usermode_function:
	nop
	; hlt ; execute privileged instruction to trigger General Protection Fault
	.loop:
		nop
		jmp .loop

	ret
;

; void x86_jumpToUsermode(uint16_t UTEXT, uint16_t UDATA);
x86_jumpToUsermode:
	; rdi = UTEXT (usermode code/text segment)
	; rsi = UDATA (usermode data segment)

	; disable interrupts because for now, we don't have a TSS to
	; be able to go back to ring 0
	cli

	; Setup the selectors for usermode: set TI to 0 and RPL to 3
	; equivalent of getSegmentSelector(rdi, 0, 3)
	and rdi, 0xfff8
	or rdi, 0x0003
	; Same for UDATA
	and rsi, 0xfff8
	or rsi, 0x0003

	; Setup segment registers
	mov rax, rsi
	mov ds, ax
	mov es, ax
	; ss is set by iretq

	; Store the kernel's gs_base, we're going to userspace
	swapgs

	; Setup the stack frame for iretq

	; Push UDATA_segment:rsp
	push rsi		; rsi still contains the UDATA segment
	mov rax, rsp
	push rax
	; Push rflags
	pushfq
	; Push UTEXT_segment:return_address
	push rdi
	lea rax, [rel usermode_function]
	push rax

	iretq
;
