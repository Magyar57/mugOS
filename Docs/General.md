# General documentation

This is a general description of the mugOS operating system.

The OS is written for the Intel x86_64 / AMD64 architecture. There are no plans to add support for other CPUs yet,
but the OS is designed in a way that makes adding architectures relatively easy.

## Features

By design, the mugOS uses the MBP: mugOS Boot Protocol. It only supports UEFI booting, as legacy BIOS was
[deprecated by Intel](https://www.intel.com/content/www/us/en/support/articles/000057401/intel-nuc.html) themselves.
See the corresponding section below for more information.

To this day, the kernel supports the following features:
- A custom bootloader
- Interrupt and IRQ support (see the [Interrupts documentation](./Interrupts.md))
- Filesystem support: none for now
- IO support: PS/2 mouse and keyboard, Serial support
- Graphical support: VGA, GOP
- Subsystems for: Graphics, mouse & keyboard support, logging.

Note: subsystems are a way to abstract hardware and expose functionnality of the underlying drivers to the
rest of the kernel, and eventually to userspace.

To see planned features, see the [Roadmap](./Roadmap.md).

### MBP: mugOS Boot Protocol

Note: this is a plan and is not yet implemented.

The MBP will support:
- Setting a display resolution a boot time
- Using a framebuffer at runtime
- Getting a memory map of the system
