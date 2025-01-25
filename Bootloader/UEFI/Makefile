# UEFI bootloader makefile

TARGET_CFLAGS+=-fshort-wchar # add "-D $(ARCH)" ?

# Include path for the UEFI bootloader
export CPATH=$(abspath .):$(abspath EFI/Arch/$(ARCH))

BUILD_SUBDIR:=$(BUILD_DIR)/bootloader/uefi

SOURCES_C=$(wildcard *.c)
OBJECTS_C=$(patsubst %.c, $(BUILD_SUBDIR)/%.o, $(SOURCES_C))

all: bootloader

.PHONY: bootloader clean run

bootloader: $(BUILD_DIR)/BOOTX64.EFI

# TODO incorporate $(TARGET_LDFLAGS) and $(TARGET_LD)
$(BUILD_DIR)/BOOTX64.EFI: $(OBJECTS_C)
	clang --target=x86_64-none-windows -nostdlib -Wl,-entry:bmain,-subsystem:efi_application,/MAP:$(BUILD_DIR)/uefi-bootloader.map -fuse-ld=lld-link -o $@ $(BUILD_SUBDIR)/*.o

.PHONY: kernel.bin
kernel.bin:
	cp ../../build/kernel.bin .

# Generic object rules

$(BUILD_SUBDIR)/%.o: %.c | $(BUILD_SUBDIR)
	clang --target=x86_64-none-windows -g -Wall -ffreestanding -fshort-wchar -mno-red-zone -O0 -c $< -o $@

# Build dirs

$(BUILD_SUBDIR):
	@mkdir -p $@
