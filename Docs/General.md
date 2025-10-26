# General documentation

This is a general description of the mugOS operating system.

The OS is written for the Intel x86_64 / AMD64 architecture. There are no plans to add support for other CPUs yet,
but the OS is designed in a way that makes adding architectures relatively easy.

## Features

The mugOS uses the Limine bootloader. It used to be shipped with it's own [Legacy bootloader](../Bootloader/Legacy),
that was upgraded to a more modern [UEFI bootloader](../Bootloader/UEFI/), which was then deprecated in favour
of the modern and well tested [Limine bootloader](https://github.com/limine-bootloader/limine).
Once past the booting part, the code is only redundant between the bootloader and the kernel. The decision
was taken to switch to the Limine bootloader, to be able to focus on the unique kernel functionnalities.

To this day, the kernel supports the following features:
- The Limine boot protocol
- CPU features detection
- Interrupts: exceptions, IRQs
- Memory management: memory map, physical and virtual memory managers, paging
- Kernel heap, implemented through a Slab allocator algorithm
- Symetric MultiProcessing (SMP) stub
- Time subsystem: sleep, delay functions, timers: PIT, PM timer
- I/O subsystem: PS/2 mouse and keyboard
- Graphics subsystem: VGA, Framebuffer
  ([GOP](https://uefi.org/specs/UEFI/2.10/12_Protocols_Console_Support.html#graphics-output-protocol) and
  [VBE](https://wiki.osdev.org/VESA_Video_Modes))
- Filesystem support: none for now
- Serial ports driver (UART)
- ACPI tables

The standard library supports:
- String operations functions
- I/O, with full printf support on integers
- Default streams
- Heap (malloc, implementation inspired by OpenBSD's `sbrk`-free implementation)
- mugOS-specific features: struct-embedded lists, ringbuffer,
  allocatable caches of objects (the slab allocator used in the kernel)

To see planned features, see the [Roadmap](./Roadmap.md).
