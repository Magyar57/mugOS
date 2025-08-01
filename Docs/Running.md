# Running mugOS

## On real hardware

If you wish to run mugOS on real hardware, it is possible ! You will need to grab
a disk image from the [releases](https://github.com/Magyar57/mugOS/releases),
or [build it](./Building.md) yourself. Then flash a USB stick using your favourite
USB flashing software (KDE's *ISO Image Writer*, Windows' *Rufus*, etc), and boot
your computer from the USB stick !

## Emulation in QEMU

The `make run` target will emulate the OS with the default architecture,
and booting mode. This is x86_64, and UEFI. BIOS booting mode is still
supported (even if Intel deprecated it years earlier).

## Debugging (QEMU+GDB)

GDB allows you to debug both the assembly and the C code, step by step.

The `make debug` target will launch a GDB debug session in your terminal, as well as a QEMU window.
A breakpoint is set at kernel entry.

If you wish to customize the debugging, see the makefile's debug target for how to launch GDB.
