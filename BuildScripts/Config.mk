# Config.mk: configurations variables

# This is mugOS v0.5
export MUGOS_MAJOR=0
export MUGOS_MINOR=5

# ==== Inputs =================================================================

LIMINE_CONF:=Bootloader/limine.conf
PARTITION1_OFFSET=2048

# ==== Outputs ================================================================

# Folders
export BUILD_DIR:=$(abspath build)
export TOOLCHAIN_PATH:=$(abspath toolchain)
export PATH:=$(PATH):$(TOOLCHAIN_PATH)/bin

# Files
export KERNEL=$(BUILD_DIR)/kernel.elf
export STDLIB_KERNEL=$(BUILD_DIR)/libkernel.a
export STDLIB_USERSPACE_STATIC=$(BUILD_DIR)/libc.a
export STDLIB_USERSPACE_DYNAMIC=$(BUILD_DIR)/libc.so
RAW_IMAGE:=$(BUILD_DIR)/raw_disk.img
PARTITION1:=$(BUILD_DIR)/partition1.img
IMAGE:=$(BUILD_DIR)/disk.img

# ==== Architecture ===========================================================

# Architecture to compile mugOS for (overridable from the environment)
export ARCH?=x86_64

include BuildScripts/Arch.mk

# ==== Download links =========================================================

OVMF_URL:=https://github.com/ilobilo/ovmf-binaries.git
LIMINE_URL:=https://github.com/limine-bootloader/limine.git
LIMINE_BRANCH:=v9.x-binary
BINUTILS_URL:=https://ftp.gnu.org/gnu/binutils/binutils-2.45.tar.xz
GCC_URL:=https://ftp.gwdg.de/pub/misc/gcc/releases/gcc-15.1.0/gcc-15.1.0.tar.gz

# ==== Compilation ============================================================

export ASM:=nasm
export CC:=clang --target=x86_64-none-elf -fdiagnostics-absolute-paths
export LD:=ld.lld

# Kernel options
export K_ASMFLAGS:=-f elf64 -g3 -F dwarf
export K_CFLAGS:=-g -Wall -Wextra -std=c2x -O0 -ffreestanding \
	-mno-red-zone -mcmodel=large -mgeneral-regs-only -fsanitize=undefined \
	-DMUGOS_MAJOR=$(MUGOS_MAJOR) -DMUGOS_MINOR=$(MUGOS_MINOR)
export K_LDFLAGS:=-nostdlib -static -L$(BUILD_DIR)
export K_LDLIBS:=-lkernel

# Userspace options
export U_ASMFLAGS:=-f elf64 -g3 -F dwarf
export U_CFLAGS:=-g -Wall -Wextra -std=c2x -O0 -ffreestanding
export U_LDFLAGS:=-nostdlib -L$(BUILD_DIR)
export U_LDLIBS:=-lc

# Misc
export MAKE_FLAGS:=--no-print-directory
