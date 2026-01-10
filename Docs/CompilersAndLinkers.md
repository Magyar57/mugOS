# Compilers and linkers

Building the OS, as of any program, takes two steps: compilation and linking.

Both gcc, clang, ld and lld are supported. The default is the LLVM toolchain, as it support
cross-compilation natively. To change which compiler or linker is used, modify the
environment variables in the `BuildScripts/Config.mk` script, as shown below.

Note 1: If you wish to use gcc or ld, you will need to compile them (see the corresponding
        section below)

Note 2: By default, the OS will be built for the x86_64 architecture. If you wish to build for
        another one (e.g. arm64), add the `make toolchain ARCH=arm64` option.

## Compilers

- x86_64 Clang (default): `CC=clang --target=x86_64-none-elf -fdiagnostics-absolute-paths`
- x86_64 GCC: `CC=x86_64-elf-gcc`

## Linkers

- x86_64 LLD (default): `LD=ld.lld`
- x86_64 LD: `LD=x86_64-elf-ld` ; add `-L$(TOOLCHAIN_PATH)/lib/gcc/x86_64-elf/[version]`
  to K_LDFLAGS and U_LDFLAGS, and add `-lgcc` to K_LDLIBS and U_LDLIBS

## Building your toolchain

If you're NOT using LLVM (clang/lld), you'll need to build what you wish to use.
This is handled automatically by the `BuildScripts/Toolchain.mk` script.

Build the tools that you need, **from the top** of the project's directory tree:

- Clang: N/A
- LLD: N/A
- LD: `make toolchain-binutils`
- GCC: `make toolchain-binutils && make toolchain-gcc`
