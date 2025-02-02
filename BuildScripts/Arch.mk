# Arch.mk: Sanitize and define architecture-related variables
# Note: the paths are relative to the root folder (mugOS)

# x86_64
ifeq ($(ARCH), $(filter $(ARCH),x86_64 amd64))
export ARCH:=x86_64
export TARGET:=x86_64-elf
QEMU_ARCH:=x86_64
LIMINE_UEFI_EXEC:=$(TOOLCHAIN_PATH)/$(TARGET)/share/limine/BOOTX64.EFI
LIMINE_BIOS_EXEC:=$(TOOLCHAIN_PATH)/$(TARGET)/share/limine/limine-bios.sys

# arm64 (aka Aarch64)
else ifeq ($(ARCH), $(filter $(ARCH),arm64 ARM64))
export ARCH:=arm64
export TARGET:=x86_64-elf
QEMU_ARCH:=aarch64
LIMINE_UEFI_EXEC:=$(TOOLCHAIN_PATH)/$(TARGET)/share/limine/BOOTAA64.EFI
LIMINE_BIOS_EXEC:=
$(error Sorry, the ARM architecture is unsupported yet)

else
$(error Unknown or unsupported architecture '$(ARCH)', please specify a valid one)
endif
