# Compilers and linkers

Building the OS, as of any program, takes two steps: compilation and linking.

Both gcc, clang, ld and lld are supported. The default is the LLVM toolchain, as it support
cross-compilation by default. If you wish to use gcc, you will need to recompile it. And if you wish
to use ld, you will need to recompile binutils. This is handled automatically by the
`BuildScripts/Toolchain.mk` makefile: run `make toolchain` **at the top** of the projet directory tree.

Note: By default, it will build for the x86_64 architecture. If you wish to build for another architecture,
(e.g. arm64) add the `make toolchain -E ARCH=arm64` option.

## Compilers

To change compiler, replace the "TARGET_CC" value in `BuildScripts/Config.mk`.

- x86_64 GCC: `x86_64-elf-gcc`
- x86_64 Clang: `clang --target=x86_64-none-elf`

## Linkers

To change compiler, replace the "TARGET_LD" value in `BuildScripts/Config.mk`.

Note: Only calling the linker directly is supported.

- x86_64 LD: `x86_64-elf-ld`
- x86_64 LLD: `ld.lld`

## Building your toolchain

If you're NOT using LLVM (clang/lld), you'll need to build what you wish to use:

- ld: `make toolchain_binutils`
- gcc: `make toolchain_binutils toolchain_gcc`
