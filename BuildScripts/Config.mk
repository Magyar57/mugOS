# Config.mk: configurations variables
# Note: the paths are relative to the root folder (mugOS)

# Architecture to compile mugOS for
# Can be overriden here, or from the command line: `make -E ARCH=arm64`
export ARCH?=x86_64

# Sanitize and define architecture variables
ifeq ($(ARCH), $(filter $(ARCH),x86_64 amd64))
export ARCH:=x86_64
export TARGET:=x86_64-elf
QEMU_ARCH:=x86_64
else ifeq ($(ARCH), $(filter $(ARCH),arm ARM arm64 ARM64))
$(error Sorry, the ARM architecture is unsupported yet)
else
$(error Unknown or unsupported architecture '$(ARCH)', please specify a valid one)
endif

# Toolchain
# Get the latest versions: https://ftp.gnu.org/gnu/binutils/ and https://gcc.gnu.org/releases.html
BINUTILS_URL:=https://ftp.gnu.org/gnu/binutils/binutils-2.42.tar.xz
GCC_URL:=https://ftp.gwdg.de/pub/misc/gcc/releases/gcc-14.1.0/gcc-14.1.0.tar.gz
export TOOLCHAIN_PATH:=$(abspath toolchain)
export TARGET_TOOLCHAIN:=$(TOOLCHAIN_PATH)/$(TARGET)
export PATH:=$(PATH):$(TARGET_TOOLCHAIN)/bin

# Compilation for target system
export TARGET_ASM:=nasm
export TARGET_ASMFLAGS:=-f elf64 -g -F dwarf
export TARGET_CC:=clang --target=x86_64-none-elf
export TARGET_CFLAGS:=-g -Wall -std=c23 -O0 -ffreestanding -mno-red-zone -mcmodel=kernel -mgeneral-regs-only
export TARGET_LD:=ld.lld
export TARGET_LDFLAGS:=-nostdlib -static
export TARGET_LIBS:=

export MAKE_FLAGS:=--no-print-directory
export CLEAR_ENV:=ASM= ASMFLAGS= CC= CFLAGS= CXX= LD= LDFLAGS= LINKFLAGS= LIBS=

# Output folders
export BUILD_DIR:=$(abspath build)
export BUILD_TOOLS_DIR:=$(BUILD_DIR)/tools
export BUILD_TOOLS_FAT_DIR:=$(BUILD_TOOLS_DIR)/fat
