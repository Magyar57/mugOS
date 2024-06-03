# mugOS

This is a custom operating system, written for learning and experimentation purposes during my free time.

To see more in details how the system works, see [the included documentation](./Docs/).

## Build

To build the operating system image, run `make` in the root folder.

It will first compile binutils and gcc for crosscompilation, then compile the source code for the system, then create a disk image for emulation. All of this necessitates the following dependencies (depending on your system):

- Windows: use WSL
- Debian and derivativees: `sudo apt install build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo wget nasm mtools dosfstools libguestfs-tools qemu-system-x86`
- Fedora and derivatives: `sudo dnf install gcc gcc-c++ make bison flex gmp-devel libmpc-devel mpfr-devel texinfo wget nasm mtools dosfstools guestfs-tools qemu-system-x86`
- macOS: `brew install wget bison flex gmp libmpc mpfr texinfo mtools nasm qemu`

There is a [Docker image](./Dockerfile) which contains the dependencies as well, and can be used to build the image directly.

## Run

To run the app, execute `qemu-system-i386 -fda build/floppy.img`

Note: you need to have Qemu installed. You can alternatively use docker (in CLI mode) to run it, but this option might not work properly in the future. Executing with a local qemu install is recommended.

## Docker

To compile using docker:

- Extract the provided compilers in the [Compilers folder](./Compilers)
- Build the compiler-hosting image: `docker build -t mug-os .`
- Compile the OS: `docker compose up`

To compile and run using docker (CLI mode only):
- Go to the [Compose.yml file](./Compose.yml)
- Comment the "compile" command, and uncomment the "compile and run"
- Run `docker compose up`

Note: to exit Qemu from nographic mode, hit `Ctrl-A`, then `X`.
