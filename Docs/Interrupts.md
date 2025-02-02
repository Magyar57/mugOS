# Interrupts

## x86_64

In mugOS and on the x86_64 plateform, interrupts are implemented as follows:

- IDT [0-255] => All point to `ISR_Asm_prehandler` => `ISR_C_prehandler`
- `ISR_C_prehandler`: checks in ISR list (this list is a module maintained variable)
	- 0x00 => `ISR_divisionByZeroError`
	- 0x08 => `ISR_doubleFault`
	- 32-47 => `IRQ_prehandler`
	- Unhandled ISR => kernel panic
- `IRQ_prehandler`: checks in IRQ list
	- 0x00 ( 0) => `IRQ_timer`
	- 0x01 ( 1) => `IRQ_keyboard`
	- 0x0c (12) => `IRQ_mouse`
	- Unhandled IRQ => kernel panic
