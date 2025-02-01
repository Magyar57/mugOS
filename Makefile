include BuildScripts/Config.mk
include BuildScripts/Toolchain.mk

all: image

.PHONY: all image bootloader kernel run clean
.PHONY: tools tools_fat

#
# Disk (hard drive) image
#
image: $(BUILD_DIR)/disk.img
IMAGE_FILES:=$(BUILD_DIR)/kernel.elf
# IMAGE_FILES+=$(BUILD_DIR)/test.txt
# IMAGE_FILES+=$(BUILD_DIR)/test_sub.txt

$(BUILD_DIR)/disk.img: $(IMAGE_FILES) | $(LIMINE_EFI_EXEC) $(LIMINE_BIOS_EXEC)
	./BuildScripts/CreateImage.sh $@ >/dev/null 2>&1

$(BUILD_DIR)/test.txt:
	printf "This is a test file :D\n" >$@

$(BUILD_DIR)/test_sub.txt:
	printf "This is a test file, in a subdirectory !\n" >$@

#
# Kernel
#
kernel: $(BUILD_DIR)/kernel.elf

$(BUILD_DIR)/kernel.elf: $(shell find Kernel/** -type f) | $(BUILD_DIR)
	@$(MAKE) -C Kernel $(MAKE_FLAGS)

#
# Bootloader
#
LIMINE_EFI_EXEC:=$(TARGET_TOOLCHAIN)/share/limine/BOOTX64.EFI
LIMINE_BIOS_EXEC:=$(TARGET_TOOLCHAIN)/share/limine/limine-bios.sys

#
# Tools
#
tools: tools_fat
tools_fat: $(BUILD_TOOLS_FAT_DIR)/tests $(BUILD_TOOLS_FAT_DIR)/cli

$(BUILD_TOOLS_FAT_DIR)/tests: $(BUILD_TOOLS_FAT_DIR)/Tests.o $(BUILD_TOOLS_FAT_DIR)/Fat.o | $(BUILD_TOOLS_FAT_DIR)
	gcc -g -Wall -std=c2x $^ -o $@

$(BUILD_TOOLS_FAT_DIR)/cli: $(BUILD_TOOLS_FAT_DIR)/FatCLI.o $(BUILD_TOOLS_FAT_DIR)/Fat.o | $(BUILD_TOOLS_FAT_DIR)
	gcc -g -Wall -std=c2x $^ -o $@

$(BUILD_TOOLS_FAT_DIR)/%.o : Tools/FAT/%.c | $(BUILD_TOOLS_FAT_DIR)
	gcc -g -Wall -std=c2x -c $< -o $@

#
# Run (if needed, add arguments using `make run -E QEMU_ARGS="arg1 arg2"`)
#
run:
	qemu-system-$(QEMU_ARCH) $(QEMU_ARGS) \
		-drive if=pflash,file=/usr/share/edk2/x64/OVMF.4m.fd,format=raw,readonly=on \
		-drive if=ide,media=disk,file=$(BUILD_DIR)/disk.img,format=raw

#
# Build directories
#

$(BUILD_DIR):
	@mkdir -p $@

$(BUILD_TOOLS_FAT_DIR): | $(BUILD_DIR)
	@mkdir -p $@

#
# Clean (kclean: clean kernel only)
#
clean:
	rm -rf $(BUILD_DIR)

kclean:
	rm -rf $(BUILD_DIR)/kernel $(BUILD_DIR)/kernel.*
