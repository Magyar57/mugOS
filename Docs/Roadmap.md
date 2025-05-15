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
- [X] Framebuffer driver
- [X] Bootloader: switch to Limine
- [X] Add kernel step by step debugging support (gdb)
- [X] Memory detection (map)
- [X] Physical memory allocator (bitmap)
- [X] Virtual memory management (paging)
- [ ] Heap (malloc/kmalloc)
- [ ] Virtual filesystem
- [ ] Kernel FAT driver
- [ ] ELF binary loading
- [X] Userland
- [ ] Init process
- [ ] System calls
- [ ] mush (the **mu**gOS **sh**ell)

## Modifications

- Refactor: abstract the Serial ports from the Serial driver
- With a timer subsystem, set a minimum delay between set LEDs for the PS/2 keyboard, to prevent bricking the device
- With USB implemented, intialize and disable USB legacy support BEFORE initializing the PS/2 controller
- With ACPI implemented, finish step 2 of the 8042 PS/2 controller initalization
- With proper executable and lib support, remove callbacks from the keyboard driver when its owner exits
- With threads, implement threaded interrupts (so that we minimize time with masked interrupts) ; see linux request_threaded_irq kernel/irq/manage.c

Notes:
- PS/2 Driver purposely does NOT support hot-plug (as the specification PS/2 was designed for)
- PS/2 Keyboard driver SysRq detection implementation sends spurious (but innocuous) alt presses (see PS2.c in handleScancode for more details)
- The framebuffer driver only supports 32 bpp framebuffers
