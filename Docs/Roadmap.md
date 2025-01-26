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
- [ ] Bootloader: abandon Legacy in favor of UEFI
- [X] GOP Driver
- [ ] UEFI Bootloader: add kernel ELF support
- [ ] MBP: mugOS Boot Protocol
- [ ] Memory detection
- [ ] kmalloc
- [ ] Kernel FAT driver
- [ ] Kernel filesystem
- [ ] ELF binary loading
- [ ] Userland
- [ ] Init process
- [ ] Add kernel debugging support (gdb)

## Modifications

- Add a way to download OVMF (in a makefile target ?) and update documentation (README)
- Improve GOP driver: video mode selection (max resolution ?, detect VM), bpp support (can it have variable size ?)

These are modifications to be done, that are dependent on other techs to be implemented first.

- [ ] When implementing USB, intialize and disable USB legacy support BEFORE initializing the PS/2 controller
- [ ] With ACPI implemented, finish step 2 of the 8042 PS/2 controller initalization
- [ ] With proper executable and lib support, remove callbacks from the keyboard driver when its owner exits

Notes:
- Current PS/2 implementation purposely does not support hot-plug (as the PS/2 was not designed for it originally...)
