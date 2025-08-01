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
OVMF_URL:=https://cdn.download.clearlinux.org/image/OVMF.fd
LIMINE_BRANCH:=v9.x-binary
# Get the latest versions: https://ftp.gnu.org/gnu/binutils/ and https://gcc.gnu.org/releases.html
BINUTILS_URL:=https://ftp.gnu.org/gnu/binutils/binutils-2.45.tar.xz
GCC_URL:=https://ftp.gwdg.de/pub/misc/gcc/releases/gcc-15.1.0/gcc-15.1.0.tar.gz

# Compilation for target system
export TARGET_ASM:=nasm
export TARGET_ASMFLAGS:=-f elf64 -g -F dwarf
export TARGET_CC:=clang --target=x86_64-none-elf
export TARGET_CFLAGS:=-g -Wall -Wextra -std=c2x -O0 -ffreestanding -mno-red-zone -mcmodel=large -mgeneral-regs-only -fsanitize=undefined
export TARGET_LD:=ld.lld
export TARGET_LDFLAGS:=-nostdlib -static
export TARGET_LIBS:=
export MAKE_FLAGS:=--no-print-directory ARCH=$(ARCH)

# Output files & configurations
UEFI_FIRMWARE:=$(TOOLCHAIN_PATH)/share/edk2/x64/OVMF.fd
LIMINE_CONF:=Bootloader/limine.conf
IMAGE:=$(BUILD_DIR)/disk.img
IMAGE_FILES:=$(BUILD_DIR)/kernel.elf
PARTITION1_OFFSET=2048

# Temporary (intermediate) output files
TEMP_IMAGE:=/tmp/disk.img
TEMP_PARTITION1:=/tmp/partition1.img
