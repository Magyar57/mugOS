# Running mugOS

If you wish to run mugOS on real hardware, it is possible ! You will need to grab a disk image
(see the [releases](https://github.com/Magyar57/mugOS/releases), or [build it](./Building.md) yourself).
Then flash a USB stick using your favourite USB flashing software (KDE's *ISO Image Writer*, Windows' *Rufus*...),
and boot your computer on the USB stick !

Otherwise, you can emulate (and debug) mugOS on QEMU or VirtualBox.
For VirtualBox, create a VM as usual, and add the mugOS disk image to the VM. For QEMU and debugging, see below.

## Emulating in QEMU

The `make run` command will emulate the OS with the default architecture/booting mode.
Alternatively, if you wish to choose the architecure and booting mode, you can choose from the followings:

- x86_64 UEFI: `qemu-system-x86_64 -drive if=pflash,file=/usr/share/edk2/x64/OVMF.4m.fd,format=raw,readonly=on -drive if=ide,media=disk,file=build/disk.img,format=raw`
- x86_64 Legacy BIOS: `qemu-system-x86_64 -drive -drive if=ide,media=disk,file=build/disk.img,format=raw`

## Debugging (QEMU+GDB)

GDB allows you to debug both the assembly and the C code, step by step.

- Run the system that you wish to debug (see the Emulating section), and add the `-s -S` QEMU flags to make it wait for a gdb connection
- Then launch the debugger: `gdb`. Now connect to the remote QEMU server and set it up:
  - target remote localhost:1234
  - symbol-file build/kernel.elf
  - break kmain
  - layout src
  - continue
