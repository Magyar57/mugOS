# Compilers and linkers

Building the OS, as of any program, takes two steps: compilation and link.

Different compilers and linkers are supported. To use your favourite, replace the flags
in `BuildScripts/Config.mk` with the values given in the following paragraphs.

Note 1: In any case, you will need to recompile gcc (and maybe binutils) in order to link agaisnt lib-gcc.
Recompilation is handled automatically by the `BuildScripts/Toolchain.mk` makefile.

Note 2: The lld linker is supported, but as of today, it relies on gcc in order to call either ld or lld
with the right options. For this reason, choosing it as a linker is discouraged.

## Compilers

gcc:
```makefile
export TARGET_CC:=i686-elf-gcc
export TARGET_CFLAGS:=-g -Wall -std=c2x -ffreestanding
```

clang:
```makefile
export TARGET_CC:=clang --target=i386-none-elf
export TARGET_CFLAGS:=-g -Wall -std=c2x -ffreestanding
```

## Linkers

Note: you can call the linkers directly (see below), but you will need to change the output map paths options, wherever they are specified.

ld (gcc frontend):
```makefile
export TARGET_LD:=i686-elf-gcc
export TARGET_LDFLAGS:=-Wl,--oformat,binary -nostdlib
export TARGET_LIBS:=-lgcc
```

lld (clang frontend) (discouraged !):
```makefile
export TARGET_LD:=clang -target i386-none-elf
export TARGET_LDFLAGS:=-fuse-ld=lld -nostdlib -fno-pie -static -Wl,--oformat,binary,--build-id=none
export TARGET_LIBS:=-L$(TOOLCHAIN_PATH)/i686-elf/lib/gcc/i686-elf/14.1.0/ -lgcc
```

ld:
```makefile
export TARGET_LD=i686-elf-ld
export TARGET_LDFLAGS=--oformat binary
export TARGET_LIBS=-L$(TOOLCHAIN_PATH)/i686-elf/lib/gcc/i686-elf/14.1.0/ -lgcc
```

lld:
```makefile
export TARGET_LD:=ld.lld
export TARGET_LDFLAGS:=--oformat binary -nostdlib
export TARGET_LIBS:=-L$(TOOLCHAIN_PATH)/i686-elf/lib/gcc/i686-elf/14.1.0/ -lgcc
```
