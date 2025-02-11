# mugOS Roadmap and TODOs

## Roadmap

- [X] Hello world bootloader
- [X] Two-stage bootloader
- [X] Switch to protected mode
- [X] Bootloader's FAT driver
- [X] Hello world kernel
- [X] Kernel IDT/GDT
- [X] Handle interrupts (ISR)
- [X] Handle hardware interrupts (IRQ)
- [X] Implement PS/2 drivers (controller and devices)
- [X] Implement a keyboard driver subsystem
- [X] Port my C Tetris to mugOS (run in ring 0)
- [X] Double fault handler
- [X] Text-mode VGA driver
- [X] Logging subsystem
- [X] Serial output driver
- [X] Bootloader: abandon Legacy in favor of UEFI
- [X] Framebuffer Driver
- [X] Bootloader: switch to Limine
- [X] Add kernel step by step debugging support (gdb)
- [ ] Memory detection
- [ ] kmalloc
- [ ] Kernel FAT driver
- [ ] Kernel filesystem
- [ ] ELF binary loading
- [ ] Userland
- [ ] Init process

## Modifications

- Bugfix: Writing to the framebuffer triggers a page fault sometimes (through Logging_log)
- Bugfix: The Serial driver internal buffer messes up the order sometimes

These are modifications to be done, that are dependent on other techs to be implemented first.

- [ ] When implementing USB, intialize and disable USB legacy support BEFORE initializing the PS/2 controller
- [ ] With ACPI implemented, finish step 2 of the 8042 PS/2 controller initalization
- [ ] With proper executable and lib support, remove callbacks from the keyboard driver when its owner exits

Notes:
- PS/2 Driver purposely does NOT support hot-plug (as the specification PS/2 was designed for)
- PS/2 Keyboard driver SysRq detection implementation sends spurious (but ) alt presses (see PS2.c in handleScancode for more details)
- The framebuffer driver only supports 32 bpp framebuffers
