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
- The Limine protocol
- Interrupt and IRQ support (see the [Interrupts documentation](./Interrupts.md))
- Filesystem support: none for now
- IO support: PS/2 mouse and keyboard, Serial support
- Graphical support: VGA, Framebuffer
  ([GOP](https://uefi.org/specs/UEFI/2.10/12_Protocols_Console_Support.html#graphics-output-protocol) and 
  [VBE](https://wiki.osdev.org/VESA_Video_Modes))
- Subsystems for: Graphics, mouse & keyboard support, logging.

Note: subsystems are a way to abstract hardware and expose functionnality of the underlying drivers to the
rest of the kernel, and eventually to userspace.

To see planned features, see the [Roadmap](./Roadmap.md).
