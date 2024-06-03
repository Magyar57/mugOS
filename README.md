# mugOS

This is a custom operating system, written for learning and experimentation purposes during my free time.

To see more in details how the system works, see [the included documentation](./Docs/).

## Build

### Native build

The native build necessitates the following dependencies:
- Debian and derivatives: `sudo apt install build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo wget nasm mtools dosfstools libguestfs-tools qemu-system-x86`
- Fedora and derivatives: `sudo dnf install gcc gcc-c++ make bison flex gmp-devel libmpc-devel mpfr-devel texinfo wget nasm mtools dosfstools guestfs-tools qemu-system-x86`
- macOS: `brew install wget bison flex gmp libmpc mpfr texinfo mtools nasm qemu`
- Windows: please use WSL 2

To build the operating system image, run `make` in the root folder.

Note: The first compilation will take a long time, since it will compile binutils and gcc for crosscompilation. If you don't wish to install all the listed dependencies, or compile gcc, you can use the docker build.

### Docker build

The [Dockerfile](./Dockerfile) will build an image containing all the dependencies as well, and can be used to build the image without downloading the dependencies on your system. To do so, run the following commands:

- Build the compiler-hosting image: `docker build -t mug-os:2.0 .`
- Compile the OS: `docker run --rm -v .:/srv/mugOS mug-os:2.0 make && sudo chown -R $(whoami):$(whoami) build`

## Run

Once the OS is compiled, run it (emulate it) by executing: `qemu-system-i386 -fda build/floppy.img`

## Debugging

To debug the OS, you can use bochs: `bochs-debugger -f Debuggers/Bochs.cfg`

It is possible to attach gdb to qemu as well, but this is not well supported yet by the OS.
