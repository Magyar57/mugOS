# Compilers and linkers

Building the OS, as of any program, takes two steps: compilation and linking.

Both gcc, clang, ld and lld are supported. The default is the LLVM toolchain, as it support
cross-compilation by default. To change which compiler and linker is used, modify the
environment variables in the `BuildScripts/Config.mk` script, as shown below.

Note 1: If you wish to use gcc or ld, you will need to recompile the right
        packages (see the corresponding section below)

Note 2: By default, it will build for the x86_64 architecture. If you wish to build for
        another architecture, (e.g. arm64) add the `make toolchain -E ARCH=arm64` option.

## Compilers

- x86_64 Clang (default): `TARGET_CC=clang --target=x86_64-none-elf -fdiagnostics-absolute-paths`
- x86_64 GCC: `TARGET_CC=x86_64-elf-gcc`

## Linkers

Note: Only direct invocation of the linker is supported.

- x86_64 LLD (default): `TARGET_LD=ld.lld` and `TARGET_LIBS=`
- x86_64 LD: `TARGET_LD=x86_64-elf-ld` and `TARGET_LIBS=-lgcc -L$(TOOLCHAIN_PATH)/lib/gcc/x86_64-elf/[version]`
  (replace version with the value in the GCC_URL variable)

## Building your toolchain

If you're NOT using LLVM (clang/lld), you'll need to build what you wish to use.
This is handled automatically by the `BuildScripts/Toolchain.mk` script.

Build the tools that you need, from **the top** of the project directory tree:

- Clang: N/A
- LLD: N/A
- LD: `make toolchain-binutils`
- GCC: `make toolchain-binutils && make toolchain-gcc`
