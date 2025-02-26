
; getSegmentSelector(int selector, int ti, int rpl)
; https://wiki.osdev.org/Segment_Selector
; slector = (segment selector & 0xfff8) | (ti & 0x0004) | (rpl & 0x0003)
%define getSegmentSelector(selector, ti, cpl) \
	((selector & 0xfff8) | ((ti << 2) & 0x0004) | (cpl & 0x0003))

global usermode_function
usermode_function:
	nop
	; hlt ; execute privileged instruction to trigger General Protection Fault
.loop:
	nop
	jmp .loop

	ret
; END usermode_function

; void x86_jumpToUsermode(uint16_t ucodeSegment, uint16_t udataSegment);
global x86_jumpToUsermode
x86_jumpToUsermode:
	; rdi = ucodeSegment aka utextSegment
	; rsi = udataSegment

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
	mov fs, ax
	mov gs, ax
	; ss is set by iretq

	; Setup the stack frame for iretq

	; Push user_data_segment:rsp
	push rsi		; rdi still contains the UDATA segment
	mov rax, rsp
	push rax
	; Push rflags
	pushfq
	; Push user_code_segment:return_address
	push rdi
	lea rax, [usermode_function]
	push rax

	iretq
; END x86_jumpToUsermode
