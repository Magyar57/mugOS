# Building mugOS

## Native build

The native build necessitates the following dependencies:
- Debian and derivatives: `sudo apt install build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo wget nasm mtools dosfstools libguestfs-tools qemu-system-x86`
- Fedora and derivatives: `sudo dnf install gcc gcc-c++ make bison flex gmp-devel libmpc-devel mpfr-devel texinfo wget nasm mtools dosfstools guestfs-tools qemu-system-x86`
- NixOS: use the provided environement: `nix-shell Shell.nix`
- macOS: `brew install wget bison flex gmp libmpc mpfr texinfo mtools nasm qemu`
- Windows: please use WSL 2

To build the operating system image, run `make` in the root folder.

Note: The first compilation will take a long time, since it will compile binutils and gcc for crosscompilation. If you don't wish to install all the listed dependencies, or compile gcc, you can use the docker build.

## Docker build

The [Dockerfile](./Dockerfile) will build an image containing all the dependencies as well, and can be used to build the image without downloading the dependencies on your system. To do so, run the following commands:

- Build the compiler-hosting image: `docker build -t mug-os:2.0 .`
- Compile the OS: `docker run --rm -v .:/srv/mugOS mug-os:2.0 make && sudo chown -R $(whoami):$(whoami) build`

## Running or debugging mugOS

### Run

Once the OS is compiled, run it (emulate it) in QEMU, by executing: `qemu-system-i386 -fda build/floppy.img`

### Debugging...

#### ... With Bochs

To debug the OS, you can use bochs: `bochs-debugger -f Debuggers/Bochs.cfg`.

### ... With gdb

You can attach to QEMU with gdb, by making QEMU wait for a gdb connection first.
This will be supported more throughly later.
