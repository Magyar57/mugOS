include BuildScripts/Config.mk
include BuildScripts/Toolchain.mk

all: image

.PHONY: all image kernel run clean kclean

#
# Disk image
#
image: $(IMAGE)

$(IMAGE): $(IMAGE_FILES) $(TEMP_IMAGE) $(TEMP_PARTITION1)
	@mcopy -i $(TEMP_PARTITION1) -o $(IMAGE_FILES) ::boot
	@if [ ! -f $@ ]; then cp $(TEMP_IMAGE) $@ ; fi
	@dd if=$(TEMP_PARTITION1) of=$@ bs=512 seek=$(PARTITION1_OFFSET) status=none
	@echo "Formatted $@"

$(TEMP_IMAGE):
	@dd if=/dev/zero of=$@ bs=1M count=20 status=none
	@sgdisk $@ --clear --new 1:2048 --type 1:ef00 >/dev/null
	@limine bios-install $@ >/dev/null 2>&1
	@echo "Created and formatted $@"

$(TEMP_PARTITION1): $(LIMINE_CONF) | $(TEMP_IMAGE) $(LIMINE_UEFI_EXEC) $(LIMINE_BIOS_EXEC)
	@dd if=$(TEMP_IMAGE) of=$@ bs=512 skip=$(PARTITION1_OFFSET) status=none
	@mkfs.fat $@ -F 12 -n "MUGOS" >/dev/null
	@mmd -i $@ ::boot ::EFI ::EFI/BOOT
	@mcopy -i $@ $(LIMINE_BIOS_EXEC) Bootloader/limine.conf ::boot
	@mcopy -i $@ $(LIMINE_UEFI_EXEC) ::EFI/BOOT
	@echo "Created and formatted $@"

#
# Kernel
#
kernel: $(BUILD_DIR)/kernel.elf

$(BUILD_DIR)/kernel.elf $(BUILD_DIR)/kernel.map: $(shell find Kernel/** -type f) | $(BUILD_DIR)
	@$(MAKE) -C Kernel $(MAKE_FLAGS)

#
# Tools
#
.PHONY: tools
tools:
	@$(MAKE) -C Tools/FAT $(MAKE_FLAGS)

#
# Run (if needed, add arguments using `make run -E QEMU_ARGS="arg1 arg2"`)
#
run:
	qemu-system-$(QEMU_ARCH) $(QEMU_ARGS) \
		-drive if=pflash,file=$(UEFI_FIRMWARE),format=raw,readonly=on \
		-drive if=ide,media=disk,file=$(BUILD_DIR)/disk.img,format=raw

#
# Build directory
#
$(BUILD_DIR):
	@mkdir -p $@

#
# Clean (kclean: cleans kernel only)
#
clean:
	rm -rf $(BUILD_DIR) $(TEMP_IMAGE) $(TEMP_PARTITION1)

kclean:
	rm -rf $(BUILD_DIR)/kernel $(BUILD_DIR)/kernel.*
