# Building mugOS

## Architecture

By default, it will build for the x86_64 architecture.
If you wish to build for another architecture, (e.g. arm64) compile
with the `make -e ARCH=arm64` option.

## Native build

There are different dependencies depending on what you want to do:

- *Running* are running dependencies (ikr)
- *Building* are building dependencies for clang. It is the default, recommended way
- *Toolchain* are building dependencies for cross-compiling binutils and gcc, if you wish to build with gcc.
  See [Compilers and Linkers](./CompilersAndLinkers.md) for more informations.

Note that you will always need `make`, as it is the base entry point for all commands.

Build dependencies, for each tested operating system:

- Arch Linux (`pacman -S PACKAGES`):
  - Running: `qemu-full edk2-ovmf`
  - Building: `make wget git binutils gcc clang nasm lld gptfdisk mtools dosfstools`
  - Toolchain: `diffutils libmpc mpfr gmp texinfo`
- Fedora (`dnf install PACKAGES`):
  - Running: `qemu edk2-ovmf`
  - Building: `make wget git binutils gcc clang nasm lld gdisk mtools dosfstools`
  - Toolchain: `libmpc-devel mpfr-devel gmp-devel`
- macOS (`brew install FORMULAE`):
  - Running: `qemu`
  - Building: `make wget git binutils gcc nasm lld mtools gptfdisk`
  - Toolchain: `bison flex libmpc mpfr gmp texinfo`
- Windows: Unsupported.

To build the operating system image, run `make` in the root folder.

## Docker build

The [Dockerfile](../Dockerfile) will build an image containing all the dependencies,
and can be used to build mugOS without downloading the dependencies on your system.
To do so, run the following commands:

- Change current directory to mugOS folder: `cd path/to/mugOS`
- Build the compiler-hosting image: `docker build --tag mug-os:2.2 .`
- Compile the OS: `docker run --rm --user $(id -u) --volume .:/srv/mugOS mug-os:2.2`
