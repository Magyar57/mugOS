include BuildScripts/Config.mk
include BuildScripts/Toolchain.mk

all: image

.PHONY: all image kernel run debug clean kclean iclean

#
# Bootable disk image
#
image: $(IMAGE)

# 1. Initial 20 MB disk image, with an empty EFI partition
$(RAW_IMAGE):
	@dd if=/dev/zero of=$@ bs=1M count=20 status=none
	@sgdisk $@ --clear --new 1:2048 --type 1:ef00 >/dev/null
	@limine bios-install $@ >/dev/null 2>&1
	@echo "Created and formatted $@"

# 2. Extract the EFI partition from the disk image, format it, and copy static files
$(PARTITION1): $(LIMINE_CONF) $(RAW_IMAGE) | $(LIMINE_UEFI_EXEC) $(LIMINE_BIOS_EXEC)
	@dd if=$(RAW_IMAGE) of=$@ bs=512 skip=$(PARTITION1_OFFSET) status=none
	@mkfs.fat $@ -F 12 -n "MUGOS" >/dev/null
	@mmd -i $@ ::boot ::EFI ::EFI/BOOT
	@mcopy -i $@ $(LIMINE_BIOS_EXEC) $(LIMINE_CONF) ::boot
	@mcopy -i $@ $(LIMINE_UEFI_EXEC) ::EFI/BOOT
	@echo "Created and formatted $@"

# 3. Write compiled files to PARTITION1, and format into a new final output file
$(IMAGE): $(KERNEL) $(PARTITION1) $(RAW_IMAGE)
	@mcopy -i $(PARTITION1) -o $(KERNEL) ::boot
	@if [ ! -f $@ ]; then cp $(RAW_IMAGE) $@ ; fi
	@dd if=$(PARTITION1) of=$@ bs=512 seek=$(PARTITION1_OFFSET) status=none
	@echo "Formatted $@"

#
# Kernel
#
kernel: $(KERNEL)

$(KERNEL): $(shell find . -path "./Kernel/*" -type f) $(STDLIB_KERNEL) | $(BUILD_DIR)
	@$(MAKE) -C Kernel $(MAKE_FLAGS)

#
# Standard library (for kernel & userspace)
#
stdlib: $(STDLIB_KERNEL) $(STDLIB_USERSPACE_STATIC) $(STDLIB_USERSPACE_DYNAMIC)

$(STDLIB_KERNEL):
	@$(MAKE) $@ -C Stdlib $(MAKE_FLAGS)

$(STDLIB_USERSPACE_STATIC):
	@$(MAKE) $@ -C Stdlib $(MAKE_FLAGS)

$(STDLIB_USERSPACE_DYNAMIC):
	@$(MAKE) $@ -C Stdlib $(MAKE_FLAGS)

#
# Run (if needed, add arguments using `make run -e QEMU_ARGS="arg1 arg2"`)
#
run:
	qemu-system-$(QEMU_ARCH) $(QEMU_ARGS) \
		-accel tcg \
		-machine q35 \
		-cpu qemu64 \
		-smp cpus=1,sockets=1,cores=1,threads=1 \
		-m 128M \
		-drive if=pflash,file=$(UEFI_FIRMWARE),format=raw,readonly=on \
		-drive if=ide,media=disk,file=$(BUILD_DIR)/disk.img,format=raw

debug:
	make run -e QEMU_ARGS="-gdb tcp::1234 -S" &
	gdb \
		-ex "file $(KERNEL)" \
		-ex "target remote localhost:1234" \
		-ex "break kmain" \
		-ex "layout src" \
		-ex "continue"

#
# Build directory
#
$(BUILD_DIR):
	@mkdir -p $@

#
# Clean (kclean: cleans kernel only, sclean: stdlib only, iclean: images only)
#
clean:
	rm -rf $(BUILD_DIR)

kclean:
	rm -rf $(BUILD_DIR)/kernel $(BUILD_DIR)/kernel.*

sclean:
	rm -rf $(BUILD_DIR)/stdlib $(STDLIB_KERNEL) $(STDLIB_USERSPACE_STATIC) $(STDLIB_USERSPACE_DYNAMIC)

iclean:
	rm $(RAW_IMAGE) $(PARTITION1) $(IMAGE)
