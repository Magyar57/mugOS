# Config.mk: configurations variables
# Note: the paths are relative to the root folder (mugOS)

# Output folders
export BUILD_DIR:=$(abspath build)
export TOOLCHAIN_PATH:=$(abspath toolchain)
export PATH:=$(PATH):$(TOOLCHAIN_PATH)/bin

# Architecture to compile mugOS for
# Can be overriden here, or from the command line: `make -E ARCH=arm64`
# (Do not edit Arch.mk if you wish to change the architecture!)
export ARCH?=x86_64
include BuildScripts/Arch.mk

# Download links
OVMF_URL:=https://github.com/ilobilo/ovmf-binaries.git
LIMINE_BRANCH:=v9.x-binary
# Get the latest versions: https://ftp.gnu.org/gnu/binutils/ and https://gcc.gnu.org/releases.html
BINUTILS_URL:=https://ftp.gnu.org/gnu/binutils/binutils-2.45.tar.xz
GCC_URL:=https://ftp.gwdg.de/pub/misc/gcc/releases/gcc-15.1.0/gcc-15.1.0.tar.gz

# Compilation options
export TARGET_ASM:=nasm
export TARGET_ASMFLAGS:=-f elf64 -g -F dwarf
export TARGET_CC:=clang --target=x86_64-none-elf -fdiagnostics-absolute-paths
export TARGET_CFLAGS:=-g -Wall -Wextra -std=c2x -O0 -ffreestanding -mno-red-zone -mcmodel=large -mgeneral-regs-only -fsanitize=undefined
export TARGET_LD:=ld.lld
export TARGET_LDFLAGS:=-nostdlib -static -L$(BUILD_DIR)
export TARGET_LDLIBS:=-lkernel
export TARGET_AR:=ar
export TARGET_ARFLAGS=rcs
export MAKE_FLAGS:=--no-print-directory

# Output files
export KERNEL=$(BUILD_DIR)/kernel.elf
export STDLIB_KERNEL=$(BUILD_DIR)/libkernel.a
export STDLIB_USERSPACE_STATIC=$(BUILD_DIR)/libc.a
export STDLIB_USERSPACE_DYNAMIC=$(BUILD_DIR)/libc.so
RAW_IMAGE:=$(BUILD_DIR)/raw_disk.img
PARTITION1:=$(BUILD_DIR)/partition1.img
IMAGE:=$(BUILD_DIR)/disk.img

# Conf files
LIMINE_CONF:=Bootloader/limine.conf

# Configuration
PARTITION1_OFFSET=2048
