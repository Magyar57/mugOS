# UEFI bootloader makefile

# Compilation for target system (UEFI bootloader: needs PE & Microsoft ABI)
# This has been moved here from Config.mk, since this bootlaoder was deprecated in favor of Limine
export UEFI_CC:=clang --target=x86_64-none-windows
export UEFI_CFLAGS:=-g -Wall -std=c2x -O0 -ffreestanding -mno-red-zone -fshort-wchar
export UEFI_LD:=clang --target=x86_64-none-windows
export UEFI_LDFLAGS:=-nostdlib -fuse-ld=lld-link -Wl,-entry:bmain,-subsystem:efi_application
export UEFI_LIBS:=

# Include path for the UEFI bootloader
export CPATH=$(abspath .):$(abspath EFI/Arch/$(ARCH))

BUILD_SUBDIR:=$(BUILD_DIR)/bootloader/uefi

SOURCES_C=$(wildcard *.c)
OBJECTS_C=$(patsubst %.c, $(BUILD_SUBDIR)/%.o, $(SOURCES_C))

all: bootloader

.PHONY: bootloader clean run

bootloader: $(BUILD_DIR)/BOOTX64.EFI

$(BUILD_DIR)/BOOTX64.EFI: $(OBJECTS_C)
	$(UEFI_LD) $(UEFI_LDFLAGS) -Wl,/MAP:$(BUILD_DIR)/uefi-bootloader.map -o $@ $(BUILD_SUBDIR)/*.o

# Generic object rules

$(BUILD_SUBDIR)/%.o: %.c | $(BUILD_SUBDIR)
	clang --target=x86_64-none-windows $(UEFI_CFLAGS) -c $< -o $@

# Build dirs

$(BUILD_SUBDIR):
	@mkdir -p $@
