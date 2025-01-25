# Config.mk: configurations variables
# Note: the paths are relative to the root folder (mugOS)

# Architecture to compile mugOS for
# Can be overriden from the command line: `make -E ARCH=arm64`
export ARCH?=x86_64

# Toolchain (paths and links to download and compile)
# Get the latest versions: https://ftp.gnu.org/gnu/binutils/ and https://gcc.gnu.org/releases.html
BINUTILS_URL:=https://ftp.gnu.org/gnu/binutils/binutils-2.42.tar.xz
GCC_URL:=https://ftp.gwdg.de/pub/misc/gcc/releases/gcc-14.1.0/gcc-14.1.0.tar.gz
export TOOLCHAIN_PATH:=$(abspath toolchain)

# Compilation for host system
export CFLAGS:=-g -Wall -std=c2x
export ASMFLAGS:=
export CC:=gcc
export CXX:=g++
export LD:=gcc
export ASM:=nasm
export LINKFLAGS:=
export LIBS:=
export CLEAR_ENV:=CFLAGS= ASMFLAGS= CC= CXX= LD= ASM= LINKFLAGS= LIBS=

# Compilation for target system
export TARGET:=i686-elf
export TARGET_ASM:=nasm
export TARGET_ASMFLAGS:=-f elf64 -g -F dwarf
export TARGET_CC:=clang --target=x86_64-none-elf
export TARGET_CFLAGS:=-g -Wall -std=c2x -ffreestanding -mno-red-zone -fPIC -fPIE -O0
export TARGET_LD:=ld
export TARGET_LDFLAGS:=-nostdlib -pic -pie --oformat binary
export TARGET_LIBS:=

export MAKE_FLAGS:=--no-print-directory

# Output folders
export BUILD_DIR:=$(abspath build)
export BUILD_TOOLS_DIR:=$(BUILD_DIR)/tools
export BUILD_TOOLS_FAT_DIR:=$(BUILD_TOOLS_DIR)/fat
