# Building mugOS

## Native build

If you wish to use a specific compiler or linker, see [Compilers and Linkers](./CompilersAndLinkers.md).

Build dependencies for each tested operating system:

- Arch Linux: `pacman -Syu base-devel gcc clang ldd bison flex gmp libmpc mpfr texinfo wget nasm mtools dosfstools qemu edk2-ovmf`
- Fedora and derivatives: `sudo dnf install gcc gcc-c++ clang make bison flex gmp-devel libmpc-devel mpfr-devel texinfo wget nasm mtools dosfstools guestfs-tools qemu-system-x86`
- NixOS: use the provided environment: `nix-shell Environment/Shell.nix`
- macOS: `brew install bison flex gmp libmpc mpfr texinfo wget nasm mtools qemu`
- Windows: Not supported. Please use WSL2.

To build the operating system image, run `make` in the root folder.
By default, it will build for the x86_64 architecture. If you wish to build for another architecture,
(e.g. arm64) add the `make toolchain -E ARCH=arm64` option.

## Docker build

The [Dockerfile](../Environment/Dockerfile) will build an image containing all the dependencies as well,
and can be used to build the image without downloading the dependencies on your system. To do so, run the following commands:

- Change current directory to mugOS folder `cd path/to/mugOS`
- Build the compiler-hosting image: `ln -sf Environment/Dockerfile && docker build -t mug-os:2.0 . ; rm Dockerfile`
- Compile the OS: `docker run --rm -v .:/srv/mugOS mug-os:2.0 make && sudo chown -R $(whoami):$(whoami) build`

## Running or debugging mugOS

### Running

The `make run` command will run the OS with the default architecture/booting mode.

Alternatively, if you wish to choose the architecure and booting mode, you can choose from the followings:

- x86_64 UEFI: `qemu-system-x86_64 -drive if=pflash,file=/usr/share/edk2/x64/OVMF.4m.fd,format=raw,readonly=on -drive if=ide,media=disk,file=build/disk.img,format=raw`
- x86_64 Legacy BIOS: `qemu-system-x86_64 -drive -drive if=ide,media=disk,file=build/disk.img,format=raw`

### Debugging

#### ... With gdb

- Run the system that you wish to debug (see the Run section), and add the `-s -S` QEMU flags to make it wait for a gdb connection
- The launch the debugger: `gdb`. Now connect to the remote QEMU server and set it up:
  - target remote localhost:1234
  - symbol-file build/kernel.elf
  - break kmain
  - layout src
  - continue

#### ... With Bochs

To debug the OS, you can use bochs: `bochs-debugger -f Environment/Bochs.cfg`.
This does NOT support UEFI booting, as bochs doesn't support OVMF.
