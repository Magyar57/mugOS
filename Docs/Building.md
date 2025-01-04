# Building mugOS

## Native build

The native build first compiles binutils and gcc for cross-compilation.
Then it compiles the code into a bootloader image, kernel image, and finally a disk image.
The depedencies listed below, before wget, are necessary for cross-compilation.
What comes after wget is needed for the mugOS compilation.

Build dependencies for each supported operating system:
- Debian and derivatives: `sudo apt install build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo wget nasm mtools dosfstools libguestfs-tools qemu-system-x86`
- Fedora and derivatives: `sudo dnf install gcc gcc-c++ make bison flex gmp-devel libmpc-devel mpfr-devel texinfo wget nasm mtools dosfstools guestfs-tools qemu-system-x86`
- Arch Linux: `pacman -Syu base-devel gcc bison flex gmp libmpc mpfr texinfo wget nasm mtools dosfstools qemu`
- NixOS: use the provided environement: `nix-shell Environment/Shell.nix`
- macOS: `brew install bison flex gmp libmpc mpfr texinfo wget nasm mtools qemu`
- Windows: Not supported. Please use WSL2.

To build the operating system image, run `make` in the root folder.
If you wish to use a specific compiler or linker, see [Compilers and Linkers](./CompilersAndLinkers.md).

Note: If you don't wish to recompile binutils and gcc, and install the dependencies,
you can use the docker build or a Nix-Shell.

## Docker build

The [Dockerfile](../Environment/Dockerfile) will build an image containing all the dependencies as well,
and can be used to build the image without downloading the dependencies on your system. To do so, run the following commands:

- Change current directory to mugOS folder `cd path/to/mugOS`
- Build the compiler-hosting image: `ln -sf Environment/Dockerfile && docker build -t mug-os:2.0 . ; rm Dockerfile`
- Compile the OS: `docker run --rm -v .:/srv/mugOS mug-os:2.0 make && sudo chown -R $(whoami):$(whoami) build`

## Running or debugging mugOS

### Run

Once the OS is compiled, run it (emulate it) in QEMU, by executing: `qemu-system-i386 -drive", "file=${workspaceFolder}/build/floppy.img,format=raw,if=floppy`

### Debugging...

#### ... With Bochs

To debug the OS, you can use bochs: `bochs-debugger -f Environment/Bochs.cfg`.

#### ... With gdb

You can attach to QEMU with gdb, by making QEMU wait for a gdb connection first.
This will be supported more throughly later.
