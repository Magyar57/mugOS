# Building mugOS

## Native build

If you wish to use a specific compiler or linker, see [Compilers and Linkers](./CompilersAndLinkers.md).

There are different dependencies depending on what you want to do: *Running*, *Building* with clang, or building with gcc (cross compiles gcc: *Toolchain*).

Build dependencies for each tested operating system:

- Arch Linux (`pacman -S PACKAGES`):
  - Running: `qemu-full edk2-ovmf`
  - Building: `make nasm gcc clang lld git gptfdisk mtools dosfstools`
  - Toolchain: `wget gcc g++ texinfo` TODO plus gmp etc ?
- Fedora (`dnf install PACKAGES`):
  - Running: `qemu edk2-ovmf`
  - Building: `make nasm gcc clang lld git mtools dosfstools gdisk`
  - Toolchain: `wget base-devel gmp-devel libmpc-devel mpfr-devel`
- NixOS: use the provided environment: `nix-shell Environment/Shell.nix`
- macOS (`brew install FORMULAE`): 
  - Running: `qemu`
  - Building: `nasm lld git mtools gptfdisk`
  - Toolchain: `wget bison flex gmp libmpc mpfr texinfo`
- Windows: Not supported. Please use WSL2.

To build the operating system image, run `make` in the root folder.
By default, it will build for the x86_64 architecture. If you wish to build for another architecture,
(e.g. arm64) add the `make -E ARCH=arm64` option.

If you wish to build with gcc, see [Compilers and Linkers](./CompilersAndLinkers.md).

## Docker build

The [Dockerfile](../Environment/Dockerfile) will build an image containing all the dependencies as well,
and can be used to build the image without downloading the dependencies on your system. To do so, run the following commands:

- Change current directory to mugOS folder `cd path/to/mugOS`
- Build the compiler-hosting image: `ln -sf Environment/Dockerfile && docker build -t mug-os:2.0 . ; rm Dockerfile`
- Compile the OS: `docker run --rm -v .:/srv/mugOS mug-os:2.0 make && sudo chown -R $(whoami):$(whoami) build`
