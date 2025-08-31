# mugOS roadmap

## Milestones

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
- [X] Text-mode VGA driver
- [X] Logging subsystem
- [X] Serial output driver
- [X] Bootloader: abandon Legacy in favor of UEFI
- [X] Framebuffer driver
- [X] Bootloader: switch to Limine
- [X] Add kernel step by step debugging support (gdb)
- [X] Memory detection (map)
- [X] Physical memory allocator (bitmap)
- [X] Virtual memory management (paging: mapping/unmapping)
- [X] Kernel heap (kmalloc, slab allocator)
- [X] ACPI tables parsing
- [X] APIC (LAPIC and IOAPIC) drivers
- [X] Timers: PIT driver
- [X] Time subsystem
- [ ] Virtual filesystem
- [ ] Kernel FAT driver
- [ ] ELF binary loading
- [X] Userland
- [X] Userspace heap (malloc, custom hashmap+bitmap implementation)
- [ ] Init process
- [ ] System calls
- [ ] mush (the **mu**gOS **sh**ell)

## Possible evolutions

- [X] *(Refactor)* Remove HHDM, in favour of dynamic memory mapping
- [ ] *(Refactor)* Abstract the Serial ports from the Serial driver
- [X] *(Fix)* Replace direct memory-mapped I/O (MMIO) access with safe, ordered and fenced calls
- [ ] *(Requires a timer subsystem)* Set a minimum delay between set LEDs for the PS/2 keyboard, to prevent bricking the device
- [ ] *(Requires USB implemented)* Initialize and disable USB legacy support BEFORE initializing the PS/2 controller
- [ ] *(Requires ACPI implemented)* Finish step 2 of the 8042 PS/2 controller initalization
- [ ] *(Requires proper executable and lib support)* Remove callbacks from the keyboard driver when its owner exits
- [ ] *(Requires threads)* Implement threaded interrupts (so that we minimize time with masked interrupts) ; see linux request_threaded_irq kernel/irq/manage.c

Notes:
- PS/2 Driver purposely does NOT support hot-plug (as the specification PS/2 was designed for)
- PS/2 Keyboard driver SysRq detection implementation sends spurious (but innocuous) alt presses (see PS2.c in handleScancode for more details)
- The framebuffer driver only supports 32 bpp framebuffers
